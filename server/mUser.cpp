#include "xml/m_xmlparser.h"
#include "file/m_file.h"
#include "convert/mConvert.h"
#include "mUser.h"
#include "mCommon.h"
#include "mCash.h"
#include "mServer.h"


unsigned int TUser::GuestID = 1;

TCasinoControl *TUser::p_CasinoControl = NULL;

TUser::TJackpot TUser::fJackpot[TUser::fJackpotSize];
unsigned int TUser::fJackpotCount = 0;

TStr TUser::TJackpot::Info()
{
	TStr s;
	for( unsigned int i = 0; i < GameCount; i++ )
		s += TStr(Game[i].ID)+":"+TStr(Game[i].Bet)+TStr(";");
	return s;
}

void TUser::TJackpot::Info(const TStr& In)
{
	TStr s(In);
	GameCount = 0;
	while( !s.isEmpty() )
	{
		if( GameCount >= fGameSize )
			break;
		TStr z(s.GetBefore(";"));
		int ID = z.CopyBefore(":").ToIntDef(0);
		if( ID > 0 ) {
			Game[GameCount].ID = ID;
			Game[GameCount].Bet = z.CopyAfter(":").ToDouble();
			GameCount++;
		}
	}
}

int TUser::JackpotGet(const unsigned int ID)
{
	for( unsigned int i = 0; i < fJackpotCount; i++ )
	{
		if( fJackpot[i].ID == ID )
			return (int)(fJackpot[i].Cash);
	}
	return 0;
}

void TUser::JackpotBet(const unsigned int GameID, const double& Cash)
{
	for( unsigned int i = 0; i < fJackpotCount; i++ )
	{
		for( unsigned int j = 0; j < fJackpot[i].GameCount; j++ )
		{
			if( fJackpot[i].Game[j].ID == GameID ) {
				double JCash = Cash * fJackpot[i].Game[j].Bet;
				fJackpot[i].Cash += JCash; 
				break;
			}
		}
	}
}

void TUser::Load(m_classes::TMySqlConnect* db)
{
	m_classes::TMySqlQuery query(db);
	query.openSql("SELECT ID,fCash,fInfo FROM money_jackpot");
	fJackpotCount = query.recordCount();
	if( fJackpotCount > fJackpotSize )
		fJackpotCount = fJackpotSize;
	for( unsigned int i = 0; i < fJackpotCount; i++ )
	{
		fJackpot[i].ID = query.fieldByNameAsInt("ID");
		fJackpot[i].Cash = query.fieldByNameAsInt("fCash");
		fJackpot[i].Info(query.fieldByName("fInfo"));
		query.next();
	}
}

void TUser::Save(m_classes::TMySqlConnect* db)
{
	m_classes::TMySqlQuery query(db);
	for( unsigned int i = 0; i < fJackpotCount; i++ )
	{
		query.execSql(
			"REPLACE money_jackpot SET "
			" fCash='"+TStr(fJackpot[i].Cash)+"',"
			" fInfo='"+fJackpot[i].Info()+"',"
			" flast=now(),"
			" id="+TStr(fJackpot[i].ID));
	}
}

void TUser::GameClear()
{
	if( fSession != NULL )
		fSession->fGame = NULL;
}

void TUser::OnStatusEnter()
{
	if( fID == 0 )
		return;
	if( fCashReal == 0 )
		fCashRealBonus = fCashFreeBets = fCashFree = 0;
	TServerSSI rs;
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	rs.ssi()->SSIValue("ID",fID);
	query.execSql(rs.ssi()->SSIRes("User_Auth_Update"));
	rs.ssi()->SSIValue("CASH",Cash());
	rs.ssi()->SSIValue("MODEID",Mode());
	if( fSession != NULL ) {
		if( fSession->fIP_Connect.isEmpty() )
			rs.ssi()->SSIValue("USERIP",fSession->fIP_HTTP);
		else
			rs.ssi()->SSIValue("USERIP",fSession->fIP_Connect);
	}
	query.execSql(rs.ssi()->SSIRes("Log_User_Enter"));
	fLogID = query.last_id();
}

void TUser::OnStatusLeave()
{
	CashRollback();
	Save();
	if( fLogID == 0 )
		return;
	TServerSSI rs;
	rs.ssi()->SSIValue("CASH",Cash());
	rs.ssi()->SSIValue("ID",fLogID);
	rs.ssi()->SSIValue("MODEID",Mode());
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.execSql(rs.ssi()->SSIRes("Log_User_Leave"));
	fLogID = 0;
}

void TUser::Save()
{
	if( fID == 0 )
		return;
	TServerSSI rs;
	rs.ssi()->SSIValue("ID",fID);
	rs.ssi()->SSIValue("CASHFUN",fCashFun);
	rs.ssi()->SSIValue("CASHREAL",fCashReal);
	rs.ssi()->SSIValue("CASHREALBONUS",(int)fCashRealBonus);
	rs.ssi()->SSIValue("CASHBONUS",TStr(fCashBonus));
	rs.ssi()->SSIValue("CASHFREE",TStr(fCashFree));
	rs.ssi()->SSIValue("CASHFREEK",TStr(fCashFreeBets));
	rs.ssi()->SSIValue("CASHREALMIN",fCashRealMin);
	rs.ssi()->SSIValue("CASHREALMINDN",fCashRealMinDn);
	rs.ssi()->SSIValue("CASHREALMAX",fCashRealMax);
	rs.ssi()->SSIValue("CASHREALMAXUP",fCashRealMaxUp);
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.execSql(rs.ssi()->SSIRes("User_Edit_Cash"));
}

void TUser::ModeToREAL()
{
	if( fStatus != tREAL ) {
		Save();
		OnStatusLeave();
		fStatus = tREAL;
		OnStatusEnter();
	}
	TServerSSI rs;
	ToSSI(rs.ssi());
	fMessage += rs.ssi()->SSIRes("server_usertype");
}

void TUser::ToSSI(m_ssi::TSSIControl *ssi)
{
	ssi->SSIValue("LOGIN",fLogin);
	ssi->SSIValue("NICK",Nick());
	ssi->SSIValue("CASH",Cash());
	ssi->SSIValue("ICON",Icon());

	switch( Status() )
	{
		case tGIFT: ssi->SSIValue("TYPE","gift"); break;
		case tFUN:  ssi->SSIValue("TYPE","fun"); break;
		case tREAL: ssi->SSIValue("TYPE","real"); break;
		default: ssi->SSIValue("TYPE","");
	}
	ssi->SSIValue("BONUS",CashBonus());

	ssi->SSIValue("COUNTRY",fCountryID);
	ssi->SSIValue("LANG",fLangID);
	ssi->SSIValue("PHONE",fPhone);
	ssi->SSIValue("SEX",fSex);
	ssi->SSIValue("ICON",fIcon);
	ssi->SSIValue("EMAIL",fEMail);
	ssi->SSIValue("DATE",fDate);
	ssi->SSIValue("GAME",fGame);
	ssi->SSIValue("BONUS",fBonus);
}

unsigned int TUser::Cash(TUserStatus status)
{
	if( status == tFUN )
		return fCashFun;
	return fCashReal;
}

unsigned int TUser::Cash()
{
	double r = 0;
	if( Status() == tFUN )
		r = fCashFun;
	else
		r = fCashReal;
	if( fCashBet != NULL )
		r -= fCashBet->Cash();
	if( r < 0 )
		throw TExcept("TUser::Cash");
	return (unsigned int)r /*+ fCashRes*/;
}

/*
unsigned int TUser::CashJackpot(const unsigned int GameID)
{
	return (unsigned int)p_CasinoControl->Cash(
		Mode(),
		GameID,
		TCasinoControl::JACKPOT);
}
*/

/*
bool TUser::CashOut(const unsigned int Cash, const TStr& sSystem, const TStr& Info)
{
	if( Cash > 100000 )
		return false;
	unsigned int System = 0;
	if( sSystem == "webmoney" )
		System = cPayWebMoney;
	if( sSystem == "creditcard" )
		System = cPayCreditCard;
	if( System > 0 ) {
		if( CashOut() > Cash ) {
			CashAdd(-(int)Cash,System);
			return true;
		}
	}
	return false;
}
*/

/*
int CashOrder(
		const unsigned int SystemID,
		const TStr& UserID,
		const int Cash,
		const TStr& Info)
{
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.execSql("INSERT INTO log_pay_order ("
		"fdate,"
		"ftime,"
		"fid_user,"
		"fid_system,"
		"fcash,"
		"finfo"
		") VALUES ("
		"now(),"
		"now(),"+
		TStr(UserID)+","+
		TStr(SystemID)+","+
		TStr(Cash)+","+
		"'"+Info+"'"
		")");
	return query.last_id();
}
*/

/*
void TUser::BonusAdd(
	const unsigned int Cash,
	const unsigned int Bonus,
	const unsigned int System)
{
	fCashFreeK = (fCashFree+Bonus)/(fCashFree*fCashFreeK+(Cash+Bonus)*5);
	CashAdd(Bonus,System);
}
*/

unsigned int TUser::CashAdd(const int Cash, const TUserStatus Status)
{
	if( Status == tFUN )
		return fCashFun += Cash;
	return fCashReal += Cash;
}

unsigned int TUser::CashAdd(
	const EPaySystem SystemID,
	const int Cash,
	const int CashBonus)
{
	TStr s;
	if( SystemID == sysWm )
		s = "webmoney";
	if( SystemID == sysCard )
		s = "credit";
	if( SystemID == sysEport )
		s = "eport";
	if( SystemID == sysWire )
		s = "wire";
	if( Cash > 0 )
		fMessage += "<money "
			"type=\"in\" "
			"system=\""+s+"\" "
			"cash=\""+TStr(Cash)+"\" "
			"bonus=\""+CashBonus+"\" "
			"info=\"\" id=\"1\" />";
	return fCashReal += (Cash+CashBonus);
}

void TUser::CashError(
	const EPaySystem SystemID,
	const int Cash)
{
	TStr s;
	if( SystemID == sysWm )
		s = "webmoney";
	if( SystemID == sysCard )
		s = "credit";
	if( SystemID == sysEport )
		s = "eport";
	if( SystemID == sysWire )
		s = "wire";
	fMessage += "<moneyerror "
		"type=\"in\" "
		"system=\""+s+"\" "
		"cash=\""+TStr(Cash)+"\" "
		"info=\"\" id=\"1\" />";
}

void TUser::OnCashAdd()
{
	if( Mode() == 2 )
		if( fStatus == tFUN && Cash() < 5000 ) {
			CashAdd(100000,tFUN);
			UserCashLog(
				false,
				sysFUN,
				fID,
				0,
				100000,
				fCashFun,
				0);
			fMessage += "<money cash=\"100000\" />";
		}
}

TUser::~TUser()
{
//	OnStatusLeave();
}

void TUser::CloseBlock() {
	OnStatusLeave();
	if( fSession != NULL ) {
		fSession->fClose = TSession::clBLOCK;
//		fSession->fUser = NULL;
	}
	fClose = true;
//	fSession = NULL;
}

void TUser::Close() {
	OnStatusLeave();
	fSession = NULL;
}

void TUser::Auth(TSession* Session)
{
	if( Session != NULL )
		fSession = Session;
	OnStatusEnter();
}

void TUser::Auth(m_classes::TMySqlQuery& query, TSession* Session)
{
	fID = TStr(query.fieldByName("ID")).ToIntDef(0);
	if( query.fieldByName("fid_mode")[0] == '1' )
		fStatus = tREAL;
	else
		fStatus = tFUN;
	fLogin = query.fieldByName("fLogin");
	fPassword = query.fieldByName("fPassword");
	fNick = query.fieldByName("fNick");
	fEMail = query.fieldByName("fEmail");

	fCashRealMin = TStr(query.fieldByName("fCash_Real_Min")).ToInt();
	fCashRealMinDn = TStr(query.fieldByName("fCash_Real_Min_Dn")).ToInt();
	fCashRealMax = TStr(query.fieldByName("fCash_Real_Max")).ToInt();
	fCashRealMaxUp = TStr(query.fieldByName("fCash_Real_Max_Up")).ToInt();

	fCashFun = TStr(query.fieldByName("fCash_Fun")).ToInt();
	fCashReal = TStr(query.fieldByName("fCash_Real")).ToInt();
	fCashRealBonus = TStr(query.fieldByName("fCash_RealBonus")).ToDouble();
	if( fCashRealBonus < 0 )
		fCashRealBonus = 0;
	fCashFree = TStr(query.fieldByName("fCash_Free")).ToDouble();
	fCashFreeBets = TStr(query.fieldByName("fCash_FreeK")).ToDouble();
	fCashBonus = TStr(query.fieldByName("fCash_Bonus")).ToDouble();

	fLangID = TStr(query.fieldByName("fid_lang")).ToIntDef(0);
	fCountryID = TStr(query.fieldByName("fid_Country")).ToIntDef(0);

	TXML xml(query.fieldByName("fInfo"));
	TXMLNode *p = xml.FindNode("user");
	if( p != NULL ) {
		fPhone = p->VarValue("Phone");
		fIcon = p->VarValue("Icon");
		fSex = p->VarValue("Sex");
		fGame = p->VarValue("Game");
		fDate = p->VarValue("Date");
		fBonus = p->VarValue("Bonus");
	}
	Auth(Session);
}

TStr TUser::Edit(
	const bool  reg,
	const TStr& Login,
	const TStr& Password,
	const TStr& Nick,
	const TStr& EMail,
	const TStr& Country,
	const TStr& Phone,
	const TStr& Lang,
	const TStr& Sex,
	const TStr& Icon,
	const TStr& Game,
	const TStr& Date,
	TStr Status,
	const TStr& Bonus)
{
	TServerSSI rs;
	if( reg && ID() > 0 )
		return "";
	if( !reg && ID() == 0 )
		return " ";
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	rs.ssi()->SSIValue("LOGIN",Login);
	rs.ssi()->SSIValue("PASSWORD",Password);
	if( reg ) {
		query.openSql(rs.ssi()->SSIRes("User_Auth"));
		if( !query.eof() )
			return " ";
	} else {
		rs.ssi()->SSIValue("ID",fID);
		query.openSql(rs.ssi()->SSIRes("User_Edit_CheckLogin"));
		if( !query.eof() )
			return " ";
	}

	if( fStatus == tGIFT ) {
		Status == "real";
		fCashFreeBets = 0;
		fCashRealBonus = fCashFree = fCashReal;
		fCashReal = 0;
	} else
		if( reg ) {
			fCashRealBonus = fCashReal = 0;
			fCashFree = fCashFreeBets = 0;
		}
	bool fEnter = reg;
	if( Status == "real" ) {
		if( fStatus != tREAL ) {
			OnStatusLeave();
			fStatus = tREAL;
			fEnter = true;
		}
	} else {
		if( fStatus != tFUN ) {
			OnStatusLeave();
			fStatus = tFUN;
			fEnter = true;
		}
	}

	TXML xml;
	TXMLNode * p = xml.AddNode("user","");
	p->SetVar("Sex",fSex = Sex);
	p->SetVar("Phone",fPhone = Phone);
	p->SetVar("Icon",fIcon = Icon);
	p->SetVar("Game",fGame = Game);
	p->SetVar("Date",fDate = Date);
	p->SetVar("Bonus",fBonus = Bonus);
	fLogin = Login;
	fPassword = Password;
	fEMail = __unescape(EMail);
	fEMail = fEMail.Trim();
	int i = 0;
	while( i < fEMail.Length() )
	{
		char c = fEMail[i];
		if( c == '-' || c == '_' || c == '.' || c == '@' || c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z' || c >= '0' && c <= '9' )
			i++;
		else
			fEMail.Delete(i,1);
	}
	fEMail = __escape(fEMail);
	rs.ssi()->SSIValue("NICK",fNick = Nick);
	rs.ssi()->SSIValue("EMAIL",fEMail);
	rs.ssi()->SSIValue("LANG",fLangID = Lang.ToIntDef(1));
	fCountryID = Country.ToIntDef(0);
	if( fCountryID == 0 )
		fCountryID = 155;
	rs.ssi()->SSIValue("COUNTRY",fCountryID);
	rs.ssi()->SSIValue("INFO",StrToSQL(xml.ToStr()));
	rs.ssi()->SSIValue("MODE",Mode());
	if( reg ) {
		int i =
			getRnd(rs.ssi()->SSIResDef("USER_REG_RND","15").ToIntDef(15))+
			rs.ssi()->SSIResDef("USER_REG_MIN","3").ToIntDef(3);
		while( true )
		{
			query.execSql(rs.ssi()->SSIRes("User_Reg"));
			fID = query.last_id();
			if( i-- <= 0 )
				break;
			query.execSql("DELETE FROM tbl_users WHERE ID="+TStr(fID));
		}

		// Генерация уникального номера
		TStr Code(fID);
		Code = TStr(getRnd(10)) + TStr(getRnd(10)) + Code + TStr(getRnd(10));
		while( Code.Length() < 12 )
			if( getRnd(3) == 2 )
				Code = TStr(getRnd(10)) + Code;
			else
				Code = Code + TStr(getRnd(10));
		rs.ssi()->SSIValue("CODE",Code,false);
		rs.ssi()->SSIValue("ID",fID);
		query.execSql(rs.ssi()->SSIRes("User_RegCode"));
		CashAdd(100000,tFUN);
		UserCashLog(
			false,
			sysFUN,
			fID,
			0,
			100000,
			100000,
			0);
		rs.ssi()->SSIValue("CASHFUN",fCashFun);
		rs.ssi()->SSIValue("CASHREAL",fCashReal);
		rs.ssi()->SSIValue("CASHREALBONUS",(int)fCashRealBonus);
		rs.ssi()->SSIValue("CASHBONUS",TStr(fCashBonus));
		rs.ssi()->SSIValue("CASHFREE",TStr(fCashFree));
		rs.ssi()->SSIValue("CASHFREEK",TStr(fCashFreeBets));
		rs.ssi()->SSIValue("CASHREALMIN",fCashRealMin);
		rs.ssi()->SSIValue("CASHREALMINDN",fCashRealMinDn);
		rs.ssi()->SSIValue("CASHREALMAX",fCashRealMax);
		rs.ssi()->SSIValue("CASHREALMAXUP",fCashRealMaxUp);
		query.execSql(rs.ssi()->SSIRes("User_Edit_Cash"));
	} else
		rs.ssi()->SSIValue("ID",fID);
	query.execSql(rs.ssi()->SSIRes("User_Edit_Profile"));

	if( fEnter )
		OnStatusEnter();

	return "";
}

// BET

bool TUser::CashBetAdd(TTable* Table)
{
	if( Table != NULL ) {
		if( Table->Cash() == 0 )
			return true;
		if( Table->Cash() > 0 )
			if( ((int)this->Cash() - (int)(Table->Cash()+0.001)) < 0 )
				return false;
		if( fCashBet == NULL ) {
			if( Table->Cash() > 0 )
				fCashBet = new TTable(Table);
			else
				return false;
		} else
			fCashBet->AddTable(Table,true);
	}
//	OnCashAdd();
	return true;
}

void TUser::CashBetRollback()
{
	if( fCashBet != NULL ) {
		delete fCashBet;
		fCashBet = NULL;
	}
}

static int iLock = 0;

bool TUser::CashBetCommit(
			const unsigned int GameID,
			TTable* Table)
{
	if( Table != NULL && !CashBetAdd(Table) )
		return false;
	double BonusP = 0;
	if( fCashBet != NULL ) {
		// Потеря центов
		int fff = (int)(fCashBet->Cash() + 0.001);
		if( fff < 0 )
			fff = 0;
		if( Status() == tFUN )
			fCashFun -= fff;
		else {
			JackpotBet(GameID,fff);
			// СПИСЫВАНИЕ НЕОТЫГРАННОГО БОНУСА
			if( fCashRealBonus > fCashReal )
				fCashRealBonus = fCashReal;
			if( fCashFree > fCashReal ) {
//				fCashFreeBets = (fCashFreeBets * fCashReal) / fCashFree; ???
				fCashFree = fCashReal;
			}
			BonusP = fCashRealBonus / fCashReal * fff;
			if( BonusP > fff )
				BonusP = fff;
			fCashRealBonus -= BonusP;
			if( fCashRealBonus < 0 )
				fCashRealBonus = 0;
			if(
				GameID != 1 &&  // roulette
				GameID != 50 && // 0 zero roulette
				GameID != 40 && // bacarata
				GameID != 42 && // casino war
				GameID != 33    // paigow poker
			) {
				double k = 1.0;
				if( GameID == 20 )
					k = 2.0;
				if( fCashFree > 0 ) {
					fCashFree -= fCashFree * fff / fCashFreeBets / k;
					fCashFreeBets -= fff / k;
				}
				if( fCashFree < 0 || fCashFreeBets < 0 )
					fCashFree = fCashFreeBets = 0;
				fCashBonus += fff*0.005;
				fMessage += "<user bonus=\""+TStr((int)fCashBonus)+"\" />";
			}
			if( fCashReal < (unsigned int)fff )
				fCashReal = 0;
			else
				fCashReal -= fff;
			// Индивидуальный призовой фонд
			if( fCashRealMin > 0 && fCashRealMin > fCashReal )
				fCashRealMinDn = fCashRealMin = 0;
			if( fCashRealMax > 0 && fCashRealMax < (fCashReal+fff) )
				fCashRealMaxUp = fCashRealMax = 0;
		}
		fCashBetCommit += fff;
	}
	TServerDB  rb;
	fCashBetID = p_CasinoControl->TRBet(
		rb.db(),
		fID,
		Mode(),
		GameID,
		fCashBet,
		fCashBetID,
		BonusP);
	CashBetRollback();
	iLock = 0;
	return true;
}


// WIN

bool TUser::CashRealMin(TTable* Table, const bool f)
{
	// Индивидуальный призовой фонд
	bool r = true;
	if( iLock++ < 400 ) {
		if( !f && fCashRealMin > 0 ) {
			if( Table != NULL && Table->Cash() > 0 ) {
				r = getRnd(100) > (int)fCashRealMinDn;
			}
		}
	}
	return r;
}

bool TUser::CashRealMax(TTable* Table, const bool f)
{
	// Индивидуальный призовой фонд
	bool r = true;
	if( iLock++ < 400 ) {
		if( !f && fCashRealMax > 0 ) {
			if( (Table == NULL || Table->Cash() == 0) ) {
				r = getRnd(100) > (int)fCashRealMaxUp;
			}
		}
	}
	return r;
}


bool TUser::CashWinUpdate(
			const unsigned int GameID,
			TTable* Table,
			const bool f)
{
	bool r = false;
	if( fCashWinID > -1 ) {
		if( CashRealMin(Table,f) && CashRealMax(Table,f) )
			r = p_CasinoControl->TRUpdate(fCashWinID,Table,f);
	} else
		r = CashWinAdd(GameID,Table,f);
	return r;
}


bool TUser::CashWinAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f)
{
	bool r = false;
	if( fCashWinID > -1 ) {
		if( CashRealMin(Table,f) && CashRealMax(Table,f) )
			r = p_CasinoControl->TRAdd(fCashWinID,Table,false);
	} else {
		if( CashRealMin(Table,f) && CashRealMax(Table,f) ) {
			fCashWinID = p_CasinoControl->TRCreate(
				fID,
				Mode(),
				GameID,
				TCasinoControl::GAME,
				Table,
				f);
			r = fCashWinID >= -1;
		}
	}
	return r;
}

/*            
void TUser::CashJackpotCommit(
		const TStr& Log,
		unsigned int GameID,
		const int Jackpot)
{
	if( Status() == tFUN )
		fCashFun += Jackpot;
	else
		fCashReal += Jackpot;
	TServerDB  db;
	if( !Log.isEmpty() && fCashBetCommit > 0 ) {
		m_classes::TMySqlQuery query(db.db());
		query.execSql(TStr("INSERT INTO log_history_")+Mode()+" "+
//			"(fid_session,fdate,ftime,fid_game,fid_user,fcash_bet,fcash_win,fcash,finfo,fid_cash_bet,fid_cash_win "
			"(fid_session,fdate,ftime,fid_game,fid_user,fcash_bet,fcash_win,fcash,fid_cash_bet,fid_cash_win "
			") VALUES ("+
			fLogID+","+
			"now(),"+
			"now(),"+
			GameID+","+
			fID+","+
			fCashBetCommit+","+
			Jackpot+","+
			Cash()+","+
//			"'"+StrToSQL(Log)+"',"+
			fCashBetID+","+
			0+
			")");
		query.execSql(TStr("INSERT DELAYED INTO log_history_")+Mode()+"_info "+
			"(id,finfo) VALUES ("+TStr(query.last_id())+",'"+StrToSQL(Log)+"')");
	}
	fCashBetID = -100;
	fCashWinID = -100;
}
*/

void TUser::CashWinCommit(const TStr& Log)
{
	if( this->fCashWinID < 0 )
		return;
	fCashWinCommit = p_CasinoControl->TRCash(this->fCashWinID);
	if( fCashWinCommit > 0 ) {
		if( Status() == tFUN )
			fCashFun += fCashWinCommit;
		else
			fCashReal += fCashWinCommit;
	}
	TServerDB  db;
	unsigned int GameID = p_CasinoControl->TRGame(this->fCashWinID)->ID();
	fCashWinID = p_CasinoControl->TRCommit(db.db(),this->fCashWinID);
	if( !Log.isEmpty() && fCashBetCommit > 0 ) {
		m_classes::TMySqlQuery query(db.db());
		query.execSql(TStr("INSERT INTO log_history_")+Mode()+" "+
//			"(fid_session,fdate,ftime,fid_game,fid_user,fcash_bet,fcash_win,fcash,finfo,fid_cash_bet,fid_cash_win "
			"(fid_session,fdate,ftime,fid_game,fid_user,fcash_bet,fcash_win,fcash,fid_cash_bet,fid_cash_win "
			") VALUES ("+
			fLogID+","+
			"now(),"+
			"now(),"+
			GameID+","+
			fID+","+
			fCashBetCommit+","+
			fCashWinCommit+","+
			Cash()+","+
//			"'"+StrToSQL(Log)+"',"+
			fCashBetID+","+
			fCashWinID+
			")");
		query.execSql(TStr("INSERT DELAYED INTO log_history_")+Mode()+"_info "+
			"(id,finfo) VALUES ("+TStr(query.last_id())+",'"+StrToSQL(Log)+"')");
	}
	fCashBetID = -100;
	fCashWinID = -100;
}

void TUser::CashWinRollback()
{
	p_CasinoControl->TRRollback(fCashWinID);
	fCashWinID = -100;
}


// - RES


bool TUser::CashResAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f)
{
	if( fCashRes == NULL )
		fCashRes = new TTable(Table);
	else
		fCashRes->AddTable(Table,true);
	// !!!
	if( !CashRealMin(Table,f) || !CashRealMax(Table,f) ) {
		print("\nfalse");
		return false;
	}
	// !!!
	if( fCashWinID < -1 || !CashWin()->CheckMin(fCashRes) )
		return CashWinUpdate(GameID,fCashRes,f);
	return true;
}


void TUser::CashResRollback()
{
	if( fCashRes != NULL ) {
		delete fCashRes;
		fCashRes = NULL;
	}
}


void TUser::CashResCommit(const unsigned int GameID)
{
//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//	CashWinUpdate(GameID,fCashRes);
	if( fCashWinID > -1 )
		p_CasinoControl->TRUpdate(fCashWinID,fCashRes);
	else {
		if( fCashWinID > -1 ) {
			p_CasinoControl->TRAdd(fCashWinID,fCashRes,false);
		} else {
			fCashWinID = p_CasinoControl->TRCreate(
				fID,
				Mode(),
				GameID,
				TCasinoControl::GAME,
				fCashRes,
				false);
		}
	}
//	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if( fCashRes != NULL ) {
		delete fCashRes;
		fCashRes = NULL;
	} 
}


// - TMP


bool TUser::CashTmpAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f)
{
	if( fCashTmpID > -1 )
		return p_CasinoControl->TRAdd(fCashTmpID,Table,f);
	fCashTmpID = p_CasinoControl->TRCreate(
		fID,
		Mode(),
		GameID,
		TCasinoControl::GAME,
		Table,
		f);
	return fCashTmpID >= -1;
}


void TUser::CashTmpCommit()
{
	if( fCashWinID < 0 )
		fCashWinID = fCashTmpID;
	else
		p_CasinoControl->TRJoin(fCashWinID,fCashTmpID);
	fCashTmpID = -100;
}


void TUser::CashTmpRollback()
{
	p_CasinoControl->TRRollback(fCashTmpID);
	fCashTmpID = -100;
}


// - COMMIT

void TUser::CashCommit(
			const unsigned int GameID,
			const TStr& Log)
{
	CashBetCommit(GameID,NULL);
	CashTmpCommit();
	CashResCommit(GameID);
	CashWinCommit(Log);
}

void TUser::CashCommit()
{
	fCashBetCommit = fCashWinCommit = 0;
}

void TUser::CashRollback()
{
	CashBetRollback();
	CashTmpRollback();
	CashResRollback();
	CashWinRollback();
	fCashBetCommit = fCashWinCommit = 0;
}

void UserCashLog(
	const bool         StatusReal,
	const EPaySystem   SystemID,
	const unsigned int UserID,
	const unsigned int BalanceFree,
	const int          Cash,
	const unsigned int Balance,
	const unsigned int OrderID)
{
	TStr ModeID;
	if( StatusReal )
		ModeID = "1";
	else
		ModeID = "2";
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.execSql(
		"INSERT INTO log_pay_"+ModeID+" ( "
		"  fdate,"
		"  ftime,"
		"  fid_user,"
		"  fid_system,"
		"  fBalanceFree,"
		"  fcash,"
		"  fBalance,"
		"  fid_order,"
		"  flast "
		") VALUES ("
		"  now(),"
		"  now(),"
		+TStr(UserID)+","
		+TStr(SystemID)+","
		+TStr(BalanceFree)+","
		+TStr(Cash)+","
		+TStr(Balance)+","
		+TStr(OrderID)+","
		"now()"
		")");
}
