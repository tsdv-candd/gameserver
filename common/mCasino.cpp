#include "utils/m_datetime.h"
#include "mCasino.h"



static void QueryToSSI(m_ssi::TSSIControl *app, m_classes::TQuery *query)
{
	for( int i = 0; i < query->fieldCount(); i++ )
		app->SSIValue(query->fieldName(i),query->fields(i),false);
}



//
// CashToStr
//
TStr CashToStr(TStr s)
{
	s = s.CopyBefore(".").CopyBefore(",").CopyBefore(";");
	int c = s.ToInt();
	int c1 = abs(c)/100;
	int c2 = abs(c)%100;
	s = TStr(c1)+".";
	if( c2 == 0 )
		s += "00";
	else
	if( c2 < 10 )
		s += "0"+TStr(c2);
	else
		s += TStr(c2);
	if( c < 0 )
		s = TStr("-") + s;
	return s;
}



//
// BalanceToStr(
//
TStr BalanceToStr(int i)
{
	TStr s(abs(i));
	s = CashToStr(s);
	if( i > 0 )
		s = "+" + s;
	else
	if( i < 0 )
		s = "-" + s;
	return s;
}

TStr BalanceToStr(const TStr& s)
{
	return BalanceToStr(s.ToInt());
}



//
// OnPeriod
//
long OnPeriod(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date)
{
	long cDate = Now();
	if( !Date.isEmpty() )
		cDate = StrToDate(Date);
	int cDay, cMonth, cYear;
	DateTo(cDate, cYear, cMonth, cDay);

	ssi->SSIBlock("&isDAY",Period == cdateDAY);
	ssi->SSIBlock("&isMONTH",Period == cdateMONTH);
	ssi->SSIBlock("&isYEAR",Period == cdateYEAR);

	ssi->SSIValue("MONTH",cMonth);
	ssi->SSIValue("YEAR",cYear);

	query->execSql(ssi->SSIRes("SQL_CasinoList_TmpCreate"));
	switch( Period )
	{
		case cdateDAY:
			for( int i = cDay; i >= 1; i-- )
			{
				ssi->SSIValue("DATE",i);
				ssi->SSIValue("TITLE",DateTimeToStr(CreateDate(cYear,cMonth,i),"%d.%m.%Y"));
				query->execSql(ssi->SSIRes("SQL_CasinoList_TmpInsert"));
			}
			break;
		case cdateMONTH: {
			int m = cMonth;
			if( cYear != DateTimeToYear(Now()) )
				m = 12;
			for( int i = m; i >= 1; i-- )
			{
				ssi->SSIValue("DATE",i);
				ssi->SSIValue("TITLE",DateTimeToStr(CreateDate(cYear,i,1),"%m.%Y"));
				query->execSql(ssi->SSIRes("SQL_CasinoList_TmpInsert"));
			}
			break;
		}
		case cdateYEAR:
			for( int i = DateTimeToYear(Now()); i >= 2003; i-- )
			{
				ssi->SSIValue("DATE",i);
				ssi->SSIValue("TITLE",DateTimeToStr(CreateDate(i,1,1),"%Y"));
				query->execSql(ssi->SSIRes("SQL_CasinoList_TmpInsert"));
			}
			break;
	}
	return cDate;
}



//
// OnPeriodItem
//
void OnPeriodItem(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	long                    cDate)
{
	int cDay, cMonth, cYear;
	DateTo(cDate, cYear, cMonth, cDay);
	if( Period == cdateDAY ) {
		int day = TStr(query->fieldByName("fDATE")).ToInt();
		ssi->SSIValue("DateBegin",TStr(cYear)+"-"+TStr(cMonth)+"-"+TStr(day));
		ssi->SSIValue("DateEnd",TStr(cYear)+"-"+TStr(cMonth)+"-"+TStr(day));
	} else
	if( Period == cdateMONTH ) {
		ssi->SSIValue("DateBegin",TStr(cYear)+"-"+query->fieldByName("fDATE")+TStr("-1"));
		ssi->SSIValue("DateEnd",TStr(cYear)+"-"+query->fieldByName("fDATE")+TStr("-31"));
	} else
	if( Period == cdateYEAR ) {
		ssi->SSIValue("DateBegin",query->fieldByName("fDATE")+TStr("-1-1"));
		ssi->SSIValue("DateEnd",query->fieldByName("fDATE")+TStr("-12-31"));
	}
}



//
// 
//
TStr CasinoPeriodList(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date)
{
	long cDate = OnPeriod(ssi,query,Period,Date);

	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Signup"));

	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Deposit"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Bonus"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_AdmIN"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_AdmOUT"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Cashout"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Refunt"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Balance"));

	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Url"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_UrlLoad"));

	ssi->SSIValue("ModeID",1);
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Game"));
	ssi->SSIValue("ModeID",2);
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_Game"));

	query->openSql(ssi->SSIRes("SQL_CasinoList"));

	TStr s;
	bool f = true;
	while( !query->eof() )
	{
		QueryToSSI(ssi,query);
		OnPeriodItem(ssi,query,Period,cDate);
		ssi->SSIBlock("&isOdd",f);
		ssi->SSIBlock("&isEven",!f);
		f = !f;
		ssi->SSIValue("DepositSum",CashToStr(query->fieldByName("DepositSum")));
		ssi->SSIValue("BonusSum",CashToStr(query->fieldByName("BonusSum")));
		ssi->SSIValue("CashoutSum",CashToStr(query->fieldByName("CashoutSum")));
		ssi->SSIValue("Balance",BalanceToStr(query->fieldByName("Balance")));
		ssi->SSIValue("AdmINSum",CashToStr(query->fieldByName("AdmINSum")));
		ssi->SSIValue("AdmOUTSum",CashToStr(query->fieldByName("AdmOUTSum")));
		ssi->SSIValue("RefuntSum",CashToStr(query->fieldByName("RefuntSum")));

		ssi->SSIValue("loadtime",TStr(query->fieldByName("loadtime")).ToInt(),false);

		s += ssi->SSIRes("AdmCasinoList_Item");
		query->next();
	}

	query->execSql(ssi->SSIRes("SQL_CasinoList_TmpDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_SignupDrop"));

	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_DepositDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_BonusDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_AdmINDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_AdmOUTDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_CashoutDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_RefuntDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_BalanceDrop"));

	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_UrlDrop"));
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_UrlLoadDrop"));
	ssi->SSIValue("ModeID",1);
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_GameDrop"));
	ssi->SSIValue("ModeID",2);
	query->execSql(ssi->SSIRes("SQL_CasinoList_Tmp_GameDrop"));

	return s;
}



//
// 
//
TStr GamePeriodList(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date)
{
	long cDate = OnPeriod(ssi,query,Period,Date);

	query->execSql(ssi->SSIRes("SQL_DateList_Tmp_Game"));
	query->openSql(ssi->SSIRes("SQL_DateList"));

	TStr s;
	bool f = true;
	while( !query->eof() )
	{
		QueryToSSI(ssi,query);

		OnPeriodItem(ssi,query,Period,cDate);

		ssi->SSIBlock("&isOdd",f);
		ssi->SSIBlock("&isEven",!f);
		f = !f;
		ssi->SSIValue("Bets",CashToStr(query->fieldByName("Bets")));
		ssi->SSIValue("Payout",CashToStr(query->fieldByName("Payout")));
		ssi->SSIValue("Profit",BalanceToStr(query->fieldByName("Profit")));
		ssi->SSIBlock("&isNegative",query->fieldByName("Profit")[0] == '-');

		s += ssi->SSIRes("AdmDateList_Item");
		query->next();
	}

	query->execSql(ssi->SSIRes("SQL_DateList_TmpDrop"));
	query->execSql(ssi->SSIRes("SQL_DateList_Tmp_GameDrop"));

	return s;
}
