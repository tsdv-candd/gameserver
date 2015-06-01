#include "mMain.h"
#include "file/m_file.h"
#include "sql/mMySQL.h"
#include "utils/m_datetime.h"
#include "xml/m_xmlparser.h"
#include "convert/mConvert.h"
#include "mail/m_mail.h"
#include "common/mPay.h"
#include "common/mCashUtils.h"
#include "common/mCasino.h"
#include "mSSIService.h"

#include <stdio.h>

/*
int StrToCash(const TStr& Cash)
{
	int l = Cash.CopyBefore(".").ToIntDef(0);
	int r = Cash.CopyAfter(".").ToIntDef(0);
	if( l < 0 || Cash[0] == '-' )
		r = -r;
	return l*100 + r;
}
*/

TStr DateToCombo(m_ssi::TSSIControl *ssi, const TStr& Name, const TStr& fDate)
{
	TStr s;
	s = "<select name="+Name+"Day>";
	unsigned int d = fDate.CopyAfter("-").CopyAfter("-").ToIntDef(0);
	for( unsigned int i = 1; i <= 31; i++ )
	{
		s += "<option value="+TStr(i);
		if( d == i )
			s += " selected";
		s += ">"+TStr(i);
	}
	s += "</select>&nbsp;<select name="+Name+"Month>";
	d = fDate.CopyAfter("-").CopyBefore("-").ToIntDef(0);
	for( unsigned int i = 1; i <= 12; i++ )
	{
		s += "<option value="+TStr(i);
		if( d == i )
			s += " selected";
		s += ">"+ssi->SSIRes("Month_"+TStr(i));
	}
	s += "</select>&nbsp;<select name="+Name+"Year>";
	d = fDate.CopyBefore("-").ToIntDef(0);
	for( unsigned int i = 2003; i <= 2004; i++ )
	{
		s += "<option value="+TStr(i);
		if( d == i )
			s += " selected";
		s += ">"+TStr(i);
	}
	s += "</select>";
	return s;
}

TMain::TMain(m_classes::TMySqlConnect *database, TSession *p)
: TApp(database,p),
	fRecords(15),
	fPage(1)
{
}

TStr GameInfoToStr(const TStr& GameID, const TStr& s)
{
	TStr ss;
	TStr LastID = "not";
	if( GameID == "1" || GameID == "50" ) {
		TXML xml;
		xml.FromStr(s.c_str());
		TStr _s, _n, _p, _t, _x, _d, _c, num;
		for( unsigned int i = 0; i < xml.NodesCount(); i++ )
		{
			TXMLNode* p = xml.GetNode(i);
			if( p->GetName() == "game" ) {
				num = p->VarValue("number");
			} else
			if( p->GetName() == "bet" ) {
				TStr sum = CashToStr(p->VarValue("cash"));
				char c = p->VarValue("id")[0];
				TStr cc = p->VarValue("id").CopyAfter(0);
				switch( c )
				{
					case 's':
						if( !_s.isEmpty() )
							_s += "  ";
						else
							_s = "\n";
						_s += sum+"=";
						_s += cc;
						break;
					case 'p':
						if( !_p.isEmpty() )
							_p += "  ";
						else
							_p = "\n";
						_p += sum+"=";
						switch( cc.ToInt() )
						{
							case 1: _p += "0+1"; break;
							case 2: _p += "0+2"; break;
							case 3: _p += "0+3"; break;
							case 4: _p += "1+4"; break;
							case 5: _p += "2+5"; break;
							case 6: _p += "3+6"; break;
							case 7: _p += "4+7"; break;
							case 8: _p += "5+8"; break;
							case 9: _p += "6+9"; break;
							case 10: _p += "7+10"; break;
							case 11: _p += "8+11"; break;
							case 12: _p += "9+12"; break;
							case 13: _p += "10+13"; break;
							case 14: _p += "11+14"; break;
							case 15: _p += "12+15"; break;
							case 16: _p += "13+16"; break;
							case 17: _p += "14+17"; break;
							case 18: _p += "15+18"; break;
							case 19: _p += "16+19"; break;
							case 20: _p += "17+20"; break;
							case 21: _p += "18+21"; break;
							case 22: _p += "19+22"; break;
							case 23: _p += "20+23"; break;
							case 24: _p += "21+24"; break;
							case 25: _p += "22+25"; break;
							case 26: _p += "23+26"; break;
							case 27: _p += "24+27"; break;
							case 28: _p += "25+28"; break;
							case 29: _p += "26+29"; break;
							case 30: _p += "27+30"; break;
							case 31: _p += "28+31"; break;
							case 32: _p += "29+32"; break;
							case 33: _p += "30+33"; break;
							case 34: _p += "31+34"; break;
							case 35: _p += "32+35"; break;
							case 36: _p += "33+36"; break;
							case 37: _p += "1+2"; break;
							case 38: _p += "2+3"; break;
							case 39: _p += "4+5"; break;
							case 40: _p += "5+6"; break;
							case 41: _p += "7+8"; break;
							case 42: _p += "8+9"; break;
							case 43: _p += "10+11"; break;
							case 44: _p += "11+12"; break;
							case 45: _p += "13+14"; break;
							case 46: _p += "14+15"; break;
							case 47: _p += "16+17"; break;
							case 48: _p += "17+18"; break;
							case 49: _p += "19+20"; break;
							case 50: _p += "20+21"; break;
							case 51: _p += "22+23"; break;
							case 52: _p += "23+24"; break;
							case 53: _p += "25+26"; break;
							case 54: _p += "26+27"; break;
							case 55: _p += "28+29"; break;
							case 56: _p += "29+30"; break;
							case 57: _p += "31+32"; break;
							case 58: _p += "32+33"; break;
							case 59: _p += "34+35"; break;
							case 60: _p += "35+36"; break;
						}
						break;
					case 't':
						_t += "\n";
						_t += sum + "=";
						switch( cc.ToInt() )
						{
							case 1: _t += "0+1+2"; break;
							case 2: _t += "0+2+3"; break;
							case 3: _t += "1+2+3"; break;
							case 4: _t += "4+5+6"; break;
							case 5: _t += "7+8+9"; break;
							case 6: _t += "10+11+12"; break;
							case 7: _t += "13+14+15"; break;
							case 8: _t += "16+17+18"; break;
							case 9: _t += "19+20+21"; break;
							case 10: _t += "22+23+24"; break;
							case 11: _t += "25+26+27"; break;
							case 12: _t += "28+29+30"; break;
							case 13: _t += "31+32+33"; break;
							case 14: _t += "34+35+36"; break;
						}
						break;
					case 'n':
						_n += "\n";
						_n += sum+"=";
						switch( cc.ToInt() )
						{
							case 0:  _n += "0+1+2+3"; break;
							case 1:  _n += "1+2+4+5"; break;
							case 2:  _n += "2+3+5+6"; break;
							case 3:  _n += "4+5+7+8"; break;
							case 4:  _n += "5+6+8+9"; break;
							case 5:  _n += "7+8+10+11"; break;
							case 6:  _n += "8+9+11+12"; break;
							case 7:  _n += "10+11+13+14"; break;
							case 8:  _n += "11+12+14+15"; break;
							case 9:  _n += "13+14+16+17"; break;
							case 10:  _n += "14+15+17+18"; break;
							case 11: _n += "16+17+19+20"; break;
							case 12: _n += "17+18+20+21"; break;
							case 13: _n += "19+20+22+23"; break;
							case 14: _n += "20+21+23+24"; break;
							case 15: _n += "22+23+25+26"; break;
							case 16: _n += "23+24+26+27"; break;
							case 17: _n += "25+26+28+29"; break;
							case 18: _n += "26+27+29+30"; break;
							case 19: _n += "28+29+31+32"; break;
							case 20: _n += "29+30+32+33"; break;
							case 21: _n += "31+32+34+35"; break;
							case 22: _n += "32+33+35+36"; break;
						}
						break;
					case 'x':
						_x += "\n";
						_x += sum+"=";
						switch( cc.ToInt() )
						{
							case 1: _x += "1+2+3+4+5+6"; break;
							case 2: _x += "4+5+6+7+8+9"; break;
							case 3: _x += "7+8+9+10+11+12"; break;
							case 4: _x += "10+11+12+13+14+15"; break;
							case 5: _x += "13+14+15+16+17+18"; break;
							case 6: _x += "16+17+18+19+20+21"; break;
							case 7: _x += "19+20+21+22+23+24"; break;
							case 8: _x += "22+23+24+25+26+27"; break;
							case 9: _x += "25+26+27+28+29+30"; break;
							case 10: _x += "28+29+30+31+32+33"; break;
							case 11: _x += "31+32+33+34+35+36"; break;
						}
						break;
					case 'd':
						if( !_d.isEmpty() )
							_d += "  ";
						else
							_d = "\n";
						_d += sum+"=";
						switch( cc.ToInt() )
						{
							case 1: _d += "01..12"; break;
							case 2: _d += "13..24"; break;
							case 3: _d += "25..36"; break;
						}
						break;
					case 'c':
						_c += "\n";
						_c += sum+"=";
						switch( cc.ToInt() )
						{
							case 1: _c += "1+4+7+10+13+16+19+22+25+28+31+34"; break;
							case 2: _c += "2+5+8+11+14+17+20+23+26+29+32+35"; break;
							case 3: _c += "3+6+9+12+15+18+21+24+27+30+33+36"; break;
						}
						break;
					case 'e': ss += "\n"+sum+"=even"; break;
					case 'o': ss += "\n"+sum+"=odd"; break;
					case 'k': ss += "\n"+sum+"=black"; break;
					case 'r': ss += "\n"+sum+"=red"; break;
					case 'b': ss += "\n"+sum+"=01..18"; break;
					case 'm': ss += "\n"+sum+"=19..36"; break;
				}
			}
		}
		ss += _x;
		ss += _c;
		ss += _d;
		ss += _n;
		ss += _t;
		ss += _p;
		ss += _s;
		ss = num + "\n" + ss;
	} else
	if(
		GameID == "20" ||

		GameID == "40" ||
		GameID == "41" ||
		GameID == "42" ||

		GameID == "30" ||
		GameID == "31" ||
		GameID == "32" ||
		GameID == "33"
	) {
		TXML xml;
		xml.FromStr(s.c_str());
		for( unsigned int i = 0; i < xml.NodesCount(); i++ )
		{
			TXMLNode* p = xml.GetNode(i);
			TStr ID(p->VarValue("id"));
			if( LastID != ID ) {
				if( !ss.isEmpty() )
					ss += "\n\n";
				ss += "box: " + ID;
				if( ID.ToIntDef(0) == 0 )
					ss += "dealer";
				LastID = ID;
			}
			TStr Card(p->VarValue("type"));
			if( Card.Length() == 1 )
				ss += "\n 0" + Card;
			else
				ss += "\n " + Card;
			ss += "    ";
			switch( Card.ToInt() )
			{
				case 1: ss += "2 �"; break;
				case 2: ss += "3 �"; break;
				case 3: ss += "4 �"; break;
				case 4: ss += "5 �"; break;
				case 5: ss += "6 �"; break;
				case 6: ss += "7 �"; break;
				case 7: ss += "8 �"; break;
				case 8: ss += "9 �"; break;
				case 9: ss += "10 �"; break;
				case 10: ss += "J �"; break;
				case 11: ss += "Q �"; break;
				case 12: ss += "K �"; break;
				case 13: ss += "A �"; break;

				case 14: ss += "2 �"; break;
				case 15: ss += "3 �"; break;
				case 16: ss += "4 �"; break;
				case 17: ss += "5 �"; break;
				case 18: ss += "6 �"; break;
				case 19: ss += "7 �"; break;
				case 20: ss += "8 �"; break;
				case 21: ss += "9 �"; break;
				case 22: ss += "10 �"; break;
				case 23: ss += "J �"; break;
				case 24: ss += "Q �"; break;
				case 25: ss += "K �"; break;
				case 26: ss += "A �"; break;

				case 27: ss += "2 �"; break;
				case 28: ss += "3 �"; break;
				case 29: ss += "4 �"; break;
				case 30: ss += "5 �"; break;
				case 31: ss += "6 �"; break;
				case 32: ss += "7 �"; break;
				case 33: ss += "8 �"; break;
				case 34: ss += "9 �"; break;
				case 35: ss += "10 �"; break;
				case 36: ss += "J �"; break;
				case 37: ss += "Q �"; break;
				case 38: ss += "K �"; break;
				case 39: ss += "A �"; break;

				case 40: ss += "2 �"; break;
				case 41: ss += "3 �"; break;
				case 42: ss += "4 �"; break;
				case 43: ss += "5 �"; break;
				case 44: ss += "6 �"; break;
				case 45: ss += "7 �"; break;
				case 46: ss += "8 �"; break;
				case 47: ss += "9 �"; break;
				case 48: ss += "10 �"; break;
				case 49: ss += "J �"; break;
				case 50: ss += "Q �"; break;
				case 51: ss += "K �"; break;
				case 52: ss += "A �"; break;

				case 53: ss += "Jr"; break;

			}
		}
	} else
		ss = StrToHTML(s);
	return ss;
}

void QueryCommon(
	m_classes::TQuery *query,
	const TStr& SQLPeriod,
	const TStr& SQLGroup)
{
}

TStr TMain::UserCashToXML(const TStr& Name)
{
	TStr Value(p_ValueList->Values(Name));
	if( !Value.isEmpty() ) {
		return TStr(
			"<user"
			" id=\"")+TStr(fEditID)+"\" "+
			Name+"=\""+Value+"\" "
			"/>";
	}
	return "";
}

TStr TMain::UserCashToXML(const TStr& Name1, const TStr& Name2)
{
	TStr Value1(p_ValueList->Values(Name1));
	TStr Value2(p_ValueList->Values(Name2));
	if( !Value1.isEmpty() && !Value2.isEmpty() ) {
		return TStr(
			"<user"
			" id=\"")+TStr(fEditID)+"\" "+
			Name1+"=\""+Value1+"\" "+
			Name2+"=\""+Value2+"\" "
			"/>";
	}
	return "";
}

// -- App

void TMain::Init()
{
	TApp::Init();
	SSILoad("adm.ini");
	p_database->setHostName(SSIRes("database_host"));
	p_database->setDatabase(SSIRes("database_name"));
	p_database->setLogin(SSIRes("database_login"));
	p_database->setPasswd(SSIRes("database_password"));
	p_database->connect();
}

bool TMain::Connect()
{
	if( fSock.isOpen() )
		return true;
	return fSock.Connect(
			SSIResDef("casino_server_host","127.0.0.1").c_str(),
			SSIResDef("casino_server_port","1024").ToIntDef(1024));
}

TStr TMain::Read()
{
	return fSock.Read(20000);
}

void TMain::Send(const TStr& s)
{
	fSock.Send(s);
}

//

TStr TMain::ScriptServer()
{
	TStr sql(p_ValueList->Values("sql"));
	SSIValue("SQL",sql);
	if( !sql.isEmpty() ) {
		if( !p_ValueList->Values("command").isEmpty() )
			sql = "<adm command="+sql+" />";
		if( Connect() ) {
			Send(sql);
			SSIValue("STREAM",Read());
		}
	}
	return SSIRes("PageServer");
}

TStr TMain::ScriptView()
{
	TStr s(SSIResDef(FormName,""));
	if( s.isEmpty() )
		s = SSIRes("Home");
	return s;
}

// --

TStr TMain::ScriptPage(const TStr& Page) {
	return SSIRes(Page);
}

void TMain::OnPages(const TStr& sCount)
{
	SSIValue("Count",sCount);
	unsigned int Count = StrToIntDef(sCount,0);
	int PageCount = 1;
	SSIBlock("&isPage",fRecords > 0);
	if( fRecords > 0 ) {
		PageCount = (int)(Count / fRecords);
		if( PageCount * fRecords != Count )
			PageCount = PageCount + 1;
		TStr s;
		int iStart = 0;
		int iEnd = PageCount;
		if( PageCount > 60 ) {
			iStart = fPage-1-30;
			if( iStart < 2 )
				iStart = 0;
			else
				s += SSIRes("AdmBlock_PageList_First");
			iEnd = iStart + 60;
			if( iEnd >= PageCount-1 ) {
				iEnd = PageCount;
				iStart = iEnd - 60;
			}
		}
		for( ; iStart < iEnd; iStart++ )
		{
			SSIValue("Page",iStart+1);
			if( (fPage-1) == (unsigned int)iStart )
				s += SSIRes("AdmBlock_PageList_Current");
			else
				s += SSIRes("AdmBlock_PageList_Common");
		}
		if( iEnd < PageCount ) {
			SSIValue("Page",PageCount);
			s += SSIRes("AdmBlock_PageList_Last");
		}
		SSIValue("Stream_PageList",s);
	}
	SSIValue("Stream_Pages",SSIRes("AdmBlock_Pages"));
	if( fRecords > 0 )
		SSIValue("LIMIT",TStr("LIMIT ")+(fPage-1)*fRecords+","+fRecords);
}

void TMain::OnPageBegin() {
	fEditID = fDelID = 0;
	fPage = 1;
	fRecords = 15;
	fModeID = 0;
	fGameID = 0;
	fSort = fSearch = fUsersID = fSessionsID = fUsersStatus = "";
	fDateBegin.Clear();
	fDateEnd.Clear();
	GUID = p_ValueList->Values("GUID");
	m_classes::TMySqlQuery query(p_database);
	query.execSql(SSIRes("SQL_GUID_Del"));
	if( !GUID.isEmpty() ) {
		SSIValue("GUID",GUID);
		bool f = p_ValueList->Values("bMenu").isEmpty();
		query.openSql(SSIRes("SQL_GUID_Select"));
		if( !query.eof() ) {
			if( f ) {
				fSort = query.fieldByName("fSort");
				fPage = TStr(query.fieldByName("fPage")).ToIntDef(1);
				fSearch = query.fieldByName("fSearch");
				fUsersStatus = query.fieldByName("fUsers");
				fDateBegin = query.fieldByName("fDateBegin");
				fDateEnd = query.fieldByName("fDateEnd");
				fUsersID = query.fieldByName("fUsersID");
				fSessionsID = query.fieldByName("fSessionsID");
				fGameID = TStr(query.fieldByName("fGameID")).ToIntDef(0);
			}
			fModeID = TStr(query.fieldByName("fModeID")).ToIntDef(0);
			fRecords = TStr(query.fieldByName("fRecords")).ToIntDef(15);
		}
	}
	if( !p_ValueList->Values("Page").isEmpty() ) {
		fPage = p_ValueList->Values("Page").ToIntDef(1);
		if( fPage == 0 )
			fPage++;
	}
	bool f = false;
	if( !p_ValueList->Values("bSearch").isEmpty() ) {
		fPage = 1;
		f = true;
	}
	if( !p_ValueList->Values("Sort").isEmpty() )
		fSort = p_ValueList->Values("Sort");

	if( !p_ValueList->Values("Edit").isEmpty() )
		fEditID = p_ValueList->Values("Edit").ToIntDef(0);

	if( !p_ValueList->Values("Del").isEmpty() )
		fDelID = p_ValueList->Values("Del").ToIntDef(0);

	if( f || !p_ValueList->Values("Search").isEmpty() )
		fSearch = p_ValueList->Values("Search");

	if( f || !p_ValueList->Values("UsersID").isEmpty() )
		fUsersID = p_ValueList->Values("UsersID");

	if( f || !p_ValueList->Values("SessionsID").isEmpty() )
		fSessionsID = p_ValueList->Values("SessionsID");

	if( !p_ValueList->Values("GameID").isEmpty() )
		fGameID = p_ValueList->Values("GameID").ToInt();

	if( !p_ValueList->Values("ModeID").isEmpty() )
		fModeID = p_ValueList->Values("ModeID").ToInt();
	if( fModeID == 0 )
		fModeID = 2;

	if( f || !p_ValueList->Values("UsersStatus").isEmpty() )
		fUsersStatus = p_ValueList->Values("UsersStatus");

	if( !p_ValueList->Values("Records").isEmpty() )
		fRecords = p_ValueList->Values("Records").ToIntDef(15);

	if( f || !p_ValueList->Values("DateBegin").isEmpty() )
		fDateBegin = p_ValueList->Values("DateBegin");
	if( f || !p_ValueList->Values("DateEnd").isEmpty() )
		fDateEnd = p_ValueList->Values("DateEnd");

	if(
		!p_ValueList->Values("BeginDay").isEmpty() ||
		!p_ValueList->Values("BeginMonth").isEmpty() ||
		!p_ValueList->Values("BeginYear").isEmpty()
	) {
		if( p_ValueList->Values("BeginDay").isEmpty() )
			p_ValueList->SetValue("BeginDay",1);
		if( p_ValueList->Values("BeginMonth").isEmpty() ) {
			if( p_ValueList->Values("BeginYear").isEmpty() )
				p_ValueList->SetValue("BeginMonth",DateTimeToMonth(Now()));
			else
				p_ValueList->SetValue("BeginMonth",1);
		}
		if( p_ValueList->Values("BeginYear").isEmpty() )
			p_ValueList->SetValue("BeginYear",DateTimeToYear(Now()));
		fDateBegin =
			p_ValueList->Values("BeginYear") + "-" +
			p_ValueList->Values("BeginMonth") + "-" +
			p_ValueList->Values("BeginDay");
	}

	if(
		!p_ValueList->Values("EndDay").isEmpty() ||
		!p_ValueList->Values("EndMonth").isEmpty() ||
		!p_ValueList->Values("EndYear").isEmpty()
	) {
		if( p_ValueList->Values("EndDay").isEmpty() )
			p_ValueList->SetValue("EndDay",31);
		if( p_ValueList->Values("EndMonth").isEmpty() ) {
			if( p_ValueList->Values("EndYear").isEmpty() )
				p_ValueList->SetValue("EndMonth",DateTimeToMonth(Now()));
			else
				p_ValueList->SetValue("EndMonth",12);
		}
		if( p_ValueList->Values("EndYear").isEmpty() )
			p_ValueList->SetValue("EndYear",DateTimeToYear(Now()));
		fDateEnd =
			p_ValueList->Values("EndYear") + "-" +
			p_ValueList->Values("EndMonth") + "-" +
			p_ValueList->Values("EndDay");
	}

	if( !f )
		OnPageDefault();

	SSIValue("GUID",GUID);
	SSIValue("FEDITID",fEditID);
	query.openSql(SSIRes("SQL_Games"));
	SSIValue("Stream_Game",QueryToCombo(&query,TStr(fGameID)));
	SSIValue("Stream_Mode",ListToSSI(this,"AdmBlock_FltMode_",fModeID),false);
	SSIValue("FMODEID",fModeID);
	SSIBlock("&isReal",fModeID == 1,false);
	SSIBlock("&isFun",fModeID != 1,false);
	TStr s1,s2;
	unsigned int d1 = fDateBegin.CopyAfter("-").CopyAfter("-").ToIntDef(0);
	unsigned int d2 = fDateEnd.CopyAfter("-").CopyAfter("-").ToIntDef(0);
	for( unsigned int i = 1; i <= 31; i++ )
	{
		SSIValue("Title",i);
		SSIValue("ID",i);
		SSIBlock("&isSelected",d1 == i);
		s1 += SSIRes("AdmBlock_Option");
		SSIBlock("&isSelected",d2 == i);
		s2 += SSIRes("AdmBlock_Option");
	}
	SSIValue("Stream_BeginDay",s1);
	SSIValue("Stream_EndDay",s2);
	s1 = s2 = "";
	d1 = fDateBegin.CopyAfter("-").CopyBefore("-").ToIntDef(0);
	d2 = fDateEnd.CopyAfter("-").CopyBefore("-").ToIntDef(0);
	for( unsigned int i = 1; i <= 12; i++ )
	{
		SSIValue("Title",SSIRes("Month_"+TStr((int)i)));
		SSIValue("ID",i);
		SSIBlock("&isSelected",d1 == i);
		s1 += SSIRes("AdmBlock_Option");
		SSIBlock("&isSelected",d2 == i);
		s2 += SSIRes("AdmBlock_Option");
	}
	SSIValue("Stream_BeginMonth",s1);
	SSIValue("Stream_EndMonth",s2);
	s1 = s2 = "";
	d1 = fDateBegin.CopyBefore("-").ToIntDef(0);
	d2 = fDateEnd.CopyBefore("-").ToIntDef(0);
	for( unsigned int i = 2002; i <= 2004; i++ )
	{
		SSIValue("Title",i);
		SSIValue("ID",i);
		SSIBlock("&isSelected",d1 == i);
		s1 += SSIRes("AdmBlock_Option");
		SSIBlock("&isSelected",d2 == i);
		s2 += SSIRes("AdmBlock_Option");
	}
	SSIValue("Stream_BeginYear",s1);
	SSIValue("Stream_EndYear",s2);
	SSIValue("Search",fSearch);
	SSIValue("UsersID",fUsersID);
	SSIValue("SessionsID",fSessionsID);
	SSIBlock("&isFiltr"+TStr(fRecords),true,false);
	SSIBlock("&isFiltr"+fUsersStatus,true,false);
	query.execSql(SSIRes("SQL_GUID_Ins"));
	GUID=query.last_id();
	SSIValue("GUID",GUID);
	p_ValueList->SetValue("guid",query.last_id());
	SSIValue("cmURL",SSIRes("cmLink"));
}

void TMain::OnPageEnd() {
	SSIValue("Records",fRecords);
	SSIValue("Sort",fSort);
	SSIValue("Page",fPage);
	SSIValue("Users",fUsersStatus);
	SSIValue("Search",fSearch);
	SSIValue("DateBegin",fDateBegin);
	SSIValue("DateEnd",fDateEnd);
	SSIValue("UsersID",fUsersID);
	SSIValue("SessionsID",fSessionsID);
	SSIValue("GameID",fGameID);
	SSIValue("ModeID",fModeID);
	SSIValue("guid",GUID);
	m_classes::TMySqlQuery query(p_database);
	query.execSql(SSIRes("SQL_GUID_Upd"));
}

TStr TMain::PageValue(const TStr& Name)
{
	if( Name == "fUsersID" )
		return fUsersID;
	if( Name == "fSearch" )
		return fSearch;
	if( Name == "fGameID" && fGameID > 0 )
		return fGameID;
	if( Name == "fSessionsID" )
		return fSessionsID;
	if( Name == "fDateBegin" )
		return fDateBegin;
	if( Name == "fDateEnd" )
		return fDateEnd;
	return "";
}

TStr TMain::OnPageSort(const TStr& FormMenu)
{
	TStr s(OnPageHead(FormMenu));
	if( !s.isEmpty() )
		s = "ORDER BY " + s;
	return s;
}

TStr TMain::OnPageWhere(const TStr& FormMenu)
{
	TPageWhere r = OnPageWhereMulti(FormMenu);
	return r.Search0;
}

TMain::TPageWhere TMain::OnPageWhereMulti(const TStr& FormMenu)
{
	TPageWhere r;
	TStr Page_Search;
	for( unsigned int i = 1; true; i++ )
	{
		TStr Name(SSIResDef(FormMenu+i+"_name",""));
		if( Name.isEmpty() )
			break;
		TStr Value(PageValue(Name));
		if( Value.isEmpty() )
			Value = p_ValueList->Values(Name);
		if( !Value.isEmpty() ) {
			TStr ValueSQL(Value);
			if( !SSIResDef(FormMenu+i+"_escape","").isEmpty() )
				ValueSQL = __escape(Value);
			if( !SSIResDef(FormMenu+i+"_cash","").isEmpty() )
				ValueSQL = Value.CopyBefore(".").ToInt()*100+Value.CopyAfter(".").ToInt();
			if( !SSIResDef(FormMenu+i+"_page","").isEmpty() )
				Page_Search += "&" + Name + "=" + Value;
			SSIValue(Name,ValueSQL);
			TStr vvv(SSIResDef(FormMenu+i+"_Value",""));
			TStr sss(SSIRes(FormMenu+i+"_sql"));
			if( vvv.isEmpty() )
				r.Search0 += sss;
			else {
				for( int i = 0; i < r.Count; i++ )
					if( r.Value[i] == vvv ) {
						r.Search[i] += sss;
						sss.Clear();
						break;
					}
				if( !sss.isEmpty() ) {
					r.Value[r.Count] = vvv;
					r.Search[r.Count] = sss;
					r.Count++;
				}
			}
			SSIValue(Name,Value);
		}
		if( !SSIResDef(FormMenu+i+"_block","").isEmpty() ) {
			SSIBlock(SSIRes(FormMenu+i+"_block"),!Value.isEmpty(),false);
		}
	}
	SSIValue("Page_Search",Page_Search);
	return r;
}

TStr TMain::OnPageHead(const TStr& FormMenu)
{
	TStr Sort;
	TStr Name;
	for( unsigned int i = 1; true; i++ )
	{
		Name = SSIResDef(FormMenu+i+"_name","");
		if( Name.isEmpty() )
			break;
		if( fSort.isEmpty() )
			fSort = Name;
		bool fSelected = (fSort == Name);
		if( fSort.isEmpty() && !fSelected ) {
			fSort = Name;
			fSelected = true;
		}
		if( fSelected )
			Sort = SSIRes(FormMenu+i+"_sort");
		SSIBlock("&notSelected",!fSelected);
		SSIBlock("&isSelected",fSelected);
		SSIValue("Type",Name);
		SSIValue("Stream_Head"+Name,SSIRes("Head"+Name));
	}
	if( Sort.isEmpty() ) {
		fSort = SSIResDef(FormMenu+"1_name",""); 
		return OnPageHead(FormMenu);
	}
	return Sort;
}

// --

void TMain::OnPageDefault()
{
	if( FormName == "AdmPeriodList" ) {
		if( fDateBegin.isEmpty() )
			fDateBegin = DateToStr(Now(),"%Y-%m-%d");
		if( fDateEnd.isEmpty() )
			fDateEnd = DateToStr(Now(),"%Y-%m-%d");
	}
}

//

TStr TMain::OnStreamUserGame(
	const TStr& UserID,
	const TStr& ModeID,
	const TStr& Date)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	SSIValue("ModeID",ModeID);
	if( Date.isEmpty() )
		query.openSql(SSIRes("SQL_Common_User_Game"));
	else {
		SSIValue("Date",Date);
		query.openSql(SSIRes("SQL_Common_User_Game_Date"));
	}
	TStr s;
	if( !query.eof() ) {
		int    sRoundsBonus = 0;
		int    sBetsBonus = 0;
		int    sProfitBonus = 0;
		int    sRounds = 0;
		int    sSessions = 0;
		int    sBets = 0;
		int    sProfit = 0;
		while( !query.eof() ) {
			QueryToSSI(this,&query);
			int Profit = TStr(query.fieldByName("Profit")).ToInt();
			int Bets = TStr(query.fieldByName("Bets")).ToInt();
			if( TStr(query.fieldByName("Sessions")).ToInt() > sSessions )
				sSessions = TStr(query.fieldByName("Sessions")).ToInt();
			sProfit += Profit;
			sBets += Bets;
			sRounds += TStr(query.fieldByName("Rounds")).ToInt();
			switch( TStr(query.fieldByName("id")).ToInt() )
			{
				case 1: case 33: case 40:
					break;
				default:
					sBetsBonus += Bets;
					sRoundsBonus += TStr(query.fieldByName("Rounds")).ToInt();
					sProfitBonus += Profit;
					break;
			}
			SSIValue("Bets",CashToStr(Bets));
			SSIValue("Profit",CashToStr(Profit));
			SSIBlock("&isNegative",Profit < 0);
			s += SSIRes("ADM_Common_User_Game");
			query.next();
		}
		SSIValue("Sessions",sSessions);
		SSIValue("Rounds",sRounds);
		SSIValue("RoundsBonus",sRoundsBonus);
		SSIValue("Bets",CashToStr(sBets));
		SSIValue("BetsBonus",CashToStr(sBetsBonus));
		SSIValue("Profit",CashToStr(sProfit));
		SSIValue("ProfitBonus",CashToStr(sProfitBonus));
		if( sBets > 0 )
			SSIValue("Payout",CashToStr((sBets-sProfit)*10000.0/sBets));
		if( sBetsBonus > 0 )
			SSIValue("PayoutBonus",CashToStr((sBetsBonus-sProfitBonus)*10000.0/sBetsBonus));
		SSIBlock("&isNegative",sProfit < 0);
		s += SSIRes("ADM_Common_User_GameTotal");
		s = SSIRes("ADM_Common_User_GameTitle") + s;
	}
	return s;
}

//

void TMain::CheckProxy(const TStr& IP)
{
	m_classes::TMySqlQuery q(p_database);
	q.openSql("SELECT finfo FROM ip WHERE fip='"+IP+"'");
	SSIBlock("&isProxy",false,false);
	SSIBlock("&isSocks",false,false);
	SSIBlock("&isNormal",false,false);
	SSIBlock("&isCheck",true,false);
	if( q.eof() ) {
		q.execSql("REPLACE INTO ip (fip,finfo) VALUES ('"+IP+"','check') ");
	} else {
		TStr Info(q.fieldByName("finfo"));
		if( Info.Length() < 4 ) {
			SSIBlock("&isCheck",false,false);
			SSIBlock("&isProxy",Info.Pos("P") > -1,false);
			SSIBlock("&isSocks",Info.Pos("S") > -1,false);
			SSIBlock("&isNormal",Info == "0",false);
		}
	}
}

TStr TMain::OnStreamUserIP(
	const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	query.openSql(SSIRes("SQL_Common_User_IP"));
	if( query.eof() ) {
		query.execSql(SSIRes("SQL_Common_User_IP_Sessions_1"));
		query.execSql(SSIRes("SQL_Common_User_IP_Sessions_2"));
		query.openSql(SSIRes("SQL_Common_User_IP_Sessions"));
	}
	TStr s;
	while( !query.eof() ) {
		CheckProxy(query.fieldByName("fip"));
		SSIValue("IP",query.fieldByName("fip"));
		SSIBlock("&isUnknow",strlen(query.fieldByName("fip"))<7);
		SSIBlock("&notUnknow",strlen(query.fieldByName("fip"))>=7);
		SSIValue("IPCount",query.fieldByName("fipcount"));
		s += SSIRes("ADM_Common_User_IP");
		query.next();
	}
	return s;
}

TStr TMain::OnStreamUserPaySystem(
	const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	SSIValue("WHERE"," and log_pay.fcash > 0");
	query.openSql(SSIRes("SQL_Common_User_PaySystem"));
	TStr s(SSIRes("ADM_Common_User_PaySystemTitle"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("fSum",BalanceToStr(query.fieldByName("fSum")));
		s += SSIRes("ADM_Common_User_PaySystem");
		query.next();
	}
	SSIValue("WHERE"," and log_pay.fcash < 0");
	query.openSql(SSIRes("SQL_Common_User_PaySystem"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("fSum",BalanceToStr(query.fieldByName("fSum")));
		s += SSIRes("ADM_Common_User_PaySystem");
		query.next();
	}
	return s;
}

TStr TMain::OnStreamUserPayStatus(
	const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	query.openSql(SSIRes("SQL_Common_User_PayStatus"));
	TStr s(SSIRes("ADM_Common_User_PayStatusTitle"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("fSum",BalanceToStr(query.fieldByName("fSum")));
		s += SSIRes("ADM_Common_User_PayStatus");
		query.next();
	}
	return s;
}

//

TStr TMain::OnStreamUserPayAdm(
	const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	query.openSql(SSIRes("SQL_Common_User_PayAdm"));
	TStr s(SSIRes("ADM_Common_User_PayAdm_Title"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("Cash",CashToStr(query.fieldByName("Cash")));
		SSIValue("Balance",CashToStr(query.fieldByName("Balance")));
		s += SSIRes("ADM_Common_User_PayAdm_Item");
		query.next();
	}
	return s;
}

//

TStr TMain::OnStreamUserPay(
	const TStr& UserID,
	const TStr& ModeID,
	const TStr& Limit)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	SSIValue("ModeID",ModeID);
	if( Limit.isEmpty() )
		SSIValue("Limit",Limit);
	else
		SSIValue("Limit","LIMIT " + Limit);
	query.openSql(SSIRes("SQL_Common_User_Pay"));
	TStr s(SSIRes("ADM_Common_User_PayTitle"));
	int cash = 0,count = 0;
	while( !query.eof() ) {
		cash += query.fieldByNameAsInt("Cash");
		count++;
		QueryToSSI(this,&query);
		SSIValue("Cash",BalanceToStr(query.fieldByName("Cash")));
		SSIValue("UserCashBefore",CashToStr(query.fieldByName("UserCashBefore")));
		SSIValue("UserCashAfter",CashToStr(query.fieldByName("UserCashAfter")));
		SSIBlock("&isOrder",query.fieldByNameAsInt("fid_order")>0);
		s += SSIRes("ADM_Common_User_Pay");
		query.next();
	}
	SSIValue("Cash",BalanceToStr(cash));
	SSIValue("Count",count);
	s += SSIRes("ADM_Common_User_PayTotal");
	return s;
}

//

TStr TMain::OnStreamUserGamePay(
	const TStr& UserID,
	const TStr& DateStart,
	const TStr& TimeStart,
	const TStr& DateEnd,
	const TStr& TimeEnd)
{
	if( DateStart == DateEnd && TimeStart == TimeEnd )
		return "";
	SSIValue("DateStart",DateStart);
	SSIValue("TimeStart",TimeStart);
	SSIValue("DateEnd",DateEnd);
	SSIValue("TimeEnd",TimeEnd);
	SSIValue("UserID",UserID);
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_Common_User_GamePay_2"));
	TStr s;
	int Wins = 0, Bets = 0;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		Bets += query.fieldByNameAsInt("bets");
		Wins += query.fieldByNameAsInt("wins");
		SSIValue("bets",CashToStr(query.fieldByName("bets")));
		SSIValue("betsmin",CashToStr(query.fieldByName("betsmin")));
		SSIValue("betsavg",CashToStr(query.fieldByName("betsavg")));
		SSIValue("betsmax",CashToStr(query.fieldByName("betsmax")));
		SSIValue("wins",CashToStr(query.fieldByName("wins")));
		SSIValue("winsmin",CashToStr(query.fieldByName("winsmin")));
		SSIValue("winsavg",CashToStr(query.fieldByName("winsavg")));
		SSIValue("winsmax",CashToStr(query.fieldByName("winsmax")));
		SSIValue("profits",BalanceToStr(query.fieldByName("profits")));
		s += SSIRes("ADM_Common_User_GamePay_2_item");
		query.next();
	}
	if( Bets == 0 )
		return "";
	SSIValue("bets",CashToStr(Bets));
	SSIValue("wins",CashToStr(Wins));
	SSIValue("profits",BalanceToStr(Bets-Wins));
	SSIValue("payout",CashToStr((Wins*10000.0)/Bets));
	s = SSIRes("ADM_Common_User_GamePay_2_total")+s;
	SSIValue("STREAM",s);
	return SSIRes("ADM_Common_User_GamePay_2");
}

TStr TMain::OnStreamUserGamePay(const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database),q(p_database);
	SSIValue("UserID",UserID);
	SSIValue("ModeID",1);
	query.openSql(SSIRes("SQL_Common_User_GamePay_1"));
	TStr s;
	if( !query.eof() ) {
		int cash = 0,count = 0;
		TStr DateLast("2010-01-01");
		TStr TimeLast("00:00:00");
		do {
			s += OnStreamUserGamePay(
				UserID,
				query.fieldByName("fDate"),
				query.fieldByName("fTime"),
				DateLast,
				TimeLast);
			cash += query.fieldByNameAsInt("Cash");
			count++;
			QueryToSSI(this,&query);
			SSIValue("Cash",BalanceToStr(query.fieldByName("Cash")));
			SSIValue("UserCashBefore",CashToStr(query.fieldByName("UserCashBefore")));
			SSIValue("UserCashAfter",CashToStr(query.fieldByName("UserCashAfter")));
			s += SSIRes("ADM_Common_User_GamePay_1");
			DateLast = query.fieldByName("fDate");
			TimeLast = query.fieldByName("fTime");
			query.next();
		} while( !query.eof() );
		s += OnStreamUserGamePay(
			UserID,
			"2000-01-01",
			"00:00:00",
			DateLast,
			TimeLast);
		SSIValue("Cash",BalanceToStr(cash));
		SSIValue("Count",count);
		s += SSIRes("ADM_Common_User_PayTotal");
	}
	return s;
}

//

TStr TMain::OnStreamUserOrderPay(
	const TStr& UserID)
{
	m_classes::TMySqlQuery query(p_database);
	m_classes::TMySqlQuery q(p_database);
	SSIValue("UserID",UserID);
	SSIValue("Where","");
	query.openSql(SSIRes("SQL_Common_User_Order"));
	TStr s(SSIRes("ADM_Common_User_OrderPayTitle"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("Cash",BalanceToStr(query.fieldByName("Cash")));
		s += SSIRes("ADM_Common_User_OrderPay");
		SSIValue("OrderID",query.fieldByName("ID"));
		q.openSql(SSIRes("SQL_Common_Order_Pay"));
		TStr ss;
		while( !q.eof() ) {
			QueryToSSI(this,&q);
			SSIValue("Cash",BalanceToStr(q.fieldByName("Cash")));
			SSIValue("Balance",CashToStr(q.fieldByName("Balance")));
			s += SSIRes("ADM_Common_User_OrderPayMore");
			q.next();
		}
		query.next();
	}
	return s;
}

//

TStr TMain::OnStreamUserOrderMethod(
	const TStr& UserID,
	const TStr& OrderID)
{
	m_classes::TMySqlQuery query(p_database);
	SSIValue("UserID",UserID);
	SSIValue("Where","");
	query.openSql(SSIRes("SQL_Common_User_Order"));
	TStr s(SSIRes("ADM_Common_User_OrderMethodTitle"));
	while( !query.eof() ) {
		QueryToSSI(this,&query);
		int Cash = TStr(query.fieldByName("Cash")).ToInt();
		SSIValue("Cash",BalanceToStr(Cash));
		int SystemID = TStr(query.fieldByName("SystemID")).ToInt();
		TStr Info;
		TXML InfoXML(query.fieldByName("Info"));
		switch( SystemID )
		{
			case sysWire:
				if( Cash > 0 ) {
					TStr HOLDER(InfoXML.VarValue("HOLDER/Value"));
					if( !HOLDER.isEmpty() )
						Info += "HOLDER="+__unescape(HOLDER) + "<br>";
					TStr BANK(InfoXML.VarValue("BANK/Value"));
					if( !BANK.isEmpty() )
						Info += "BANK="+__unescape(BANK);
				} else {
					Info =
						"HOLDER="+__unescape(InfoXML.VarValue("HOLDER/Value")) + "<br>" +
						"BANK="+__unescape(InfoXML.VarValue("BANK/Value")) + "<br>" +
						"PHONES="+__unescape(InfoXML.VarValue("PHONECODE/Value")) + "|" + __unescape(InfoXML.VarValue("PHONES/Value")) + "<br>" +
						"NUM="+__unescape(InfoXML.VarValue("NUM/Value")) + "<br>" +
						"ADDRESS="+__unescape(InfoXML.VarValue("ADDRESS/Value")) + "<br>" +
						"SWIFT="+__unescape(InfoXML.VarValue("SWIFT/Value"));
				}
				break;
			case sysEport:
				Info =
					InfoXML.VarValue("num1/Value") + "&nbsp;-&nbsp;" +
					InfoXML.VarValue("num2/Value");
				break;
			case sysYandex:
				Info = InfoXML.VarValue("num/Value");
				break;
			case sysWm:
				if( Cash > 0 ) {
					Info = InfoXML.VarValue("WMID/Value");
					if( InfoXML.VarValue("WMID/Currency") == "1" )
						Info += TStr("<br>")+CashToStr(InfoXML.VarValue("WMID/CurrencyCash"))+"&nbsp;RUB";
					else
					if( InfoXML.VarValue("WMID/Currency") == "2" )
						Info += TStr("<br>")+CashToStr(InfoXML.VarValue("WMID/CurrencyCash"))+"&nbsp;GRI";
					else
					if( InfoXML.VarValue("WMID/Currency") == "3" )
						Info += TStr("<br>")+CashToStr(InfoXML.VarValue("WMID/CurrencyCash"))+"&nbsp;EU";
				} else
					Info = InfoXML.VarValue("WMZ/Value")+InfoXML.VarValue("WMR/Value");
				break;
			case sysCard: {
				if( Cash > 0 ) {
					TStr STATE(InfoXML.VarValue("STATE/Value"));
					m_classes::TMySqlQuery q(p_database);
					if( !STATE.isEmpty() ) {
						q.openSql("SELECT fTitle FROM const_state WHERE ID='"+STATE+"'");
						if( !q.eof() )
							STATE = q.fields(0);
					}
					TStr COUNTRY(InfoXML.VarValue("COUNTRY/Value"));
					if( !COUNTRY.isEmpty() ) {
						q.openSql("SELECT fTitle FROM const_country WHERE fCode='"+COUNTRY+"'");
						if( !q.eof() )
							COUNTRY = q.fields(0);
					}
					TStr CCNUM(InfoXML.VarValue("CCNUM/Value"));
					for( int i = 6; i < CCNUM.Length()-4; i++ )
						CCNUM[i] = '*';
					CheckProxy(InfoXML.VarValue("IP/Value"));
					SSIValue("IP",InfoXML.VarValue("IP/Value"));
					Info =
						"<table cellspacing=0 cellpadding=0 border=0>"
						"<tr><td align=right>CLUB</td><td width=10></td><td>" + InfoXML.VarValue("CLUB/Value") +"</td></tr>"
						"<tr><td align=right>IP</td><td width=10></td><td>" + SSIRes("ADM_Common_IP") +"</td></tr>"
						"<tr><td align=right>CCNUM</td><td width=10></td><td>" + CCNUM +"</td></tr>"
						"<tr><td align=right>CCEXP</td><td width=10></td><td>" + InfoXML.VarValue("CCEXP/Value") +"</td></tr>"
						"<tr><td align=right>CCTYPE</td><td width=10></td><td>" + InfoXML.VarValue("CCTYPE/Value") +"</td></tr>"
						"<tr><td align=right>CVV2</td><td width=10></td><td>" + InfoXML.VarValue("CVV2/Value") +"</td></tr>"
						"<tr><td align=right>BANK</td><td width=10></td><td>" + InfoXML.VarValue("BANK/Value")+"</td></tr>"
						"<tr><td align=right>CCNAME</td><td width=10></td><td>" + InfoXML.VarValue("CCNAME/Value")+"</td></tr>"
						"<tr><td align=right>PHONE</td><td width=10></td><td>(" + InfoXML.VarValue("PHONECODE/Value") + ") " + InfoXML.VarValue("PHONE/Value") + "</td></tr>"
						"<tr><td align=right>COUNTRY</td><td width=10></td><td>" + COUNTRY + "</td></tr>"
						"<tr><td align=right>STATE</td><td width=10></td><td>" + STATE + "</td></tr>"
						"<tr><td align=right>ZIP</td><td width=10></td><td>" + InfoXML.VarValue("ZIP/Value") + "</td></tr>"
						"<tr><td align=right>CITY</td><td width=10></td><td>" + InfoXML.VarValue("CITY/Value") + "</td></tr>"
						"<tr><td align=right>ADDRESS</td><td width=10></td><td>" + InfoXML.VarValue("ADDRESS/Value") + "</td></tr>"
						"</table>";
				}
				break;
			}
			default:
				Info = StrToHTML(query.fieldByName("Info"));
		}
		SSIValue("Info",Info);
		SSIBlock("&isSelected",OrderID == query.fieldByName("ID"));
		s += SSIRes("ADM_Common_User_OrderMethod");
		query.next();
	}
	return s;
}

// ScriptAdmPeriodList

bool f;

TStr TMain::ScriptAdmPeriod(const TStr& res)
{
/*
	m_classes::TMySqlQuery query(p_database);
	SSIValue("fCount","",false);
	SSIValue("fSum","");
	SSIValue("fAvg","");
	SSIValue("fMax","");
	SSIValue("fMin","");
	query.openSql(SSIRes("SQL_PeriodList_"+res));
//	query.openSql(SSIRes("SQL_PeriodList_Select"));
	if( !query.eof() ) {
//		SSIValue("fCount",query.fieldByName("fCount"),false);
		QueryToSSI(this,&query);
		bool f = res == "bet" || res == "win" || res == "Profit";
		SSIValue("fSum",query.fieldByName("fSum"));
		if( f )
			SSIValue("fSum",CashToStr(query.fieldByName("fSum")));
		if( query.fieldIndex("fAvg") > -1 ) {
			SSIValue("fAvg",query.fieldByName("fAvg"));
			if( f )
				SSIValue("fAvg",CashToStr(query.fieldByName("fAvg")));
		}
//		SSIValue("fMax",query.fieldByName("fMax"));
//		SSIValue("fMin",query.fieldByName("fMin"));
	}
	SSIValue("Title",SSIRes("AdmPeriodList_Title_"+res));
	SSIBlock("&isEven",f);
	SSIBlock("&isOdd",!f);
	f = !f;
//	query.execSql(SSIRes("SQL_PeriodList_Clear"));
*/
	return SSIRes("AdmPeriodList_Item");
}

TStr TMain::ScriptAdmPeriodList()
{
	f = true;
	SSIBlock("&isCommon",true);
	SSIValue("DateBegin",fDateBegin);
	SSIBlock("&isDateBegin",!fDateBegin.isEmpty());
	SSIValue("DateEnd",fDateEnd);
	SSIBlock("&isDateEnd",!fDateEnd.isEmpty());
	TStr s;
	s += ScriptAdmPeriod("Load");
	s += ScriptAdmPeriod("Connect");
	s += ScriptAdmPeriod("Reg");
	s += ScriptAdmPeriod("Auth");
	s += ScriptAdmPeriod("Client");
	s += ScriptAdmPeriod("Players");
	s += ScriptAdmPeriod("Bet");
	s += ScriptAdmPeriod("Win");
	s += ScriptAdmPeriod("Profit");
	s += ScriptAdmPeriod("Rounds");
	SSIValue("Stream_PeriodList",s);
	
	return ScriptPage("AdmPeriodList");
}


// ScriptAdmMethodList

TStr TMain::ScriptAdmMethodList()
{
	if( fEditID > 0 )
		return ScriptAdmMethodEdit();

	SSIValue("WHERE",OnPageWhere("AdmMethodList_Search"));
	SSIValue("SORT",OnPageSort("AdmMethodList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	query.execSql(SSIRes("SQL_MethodList_IN"));
	query.execSql(SSIRes("SQL_MethodList_IN_OK"));
	query.execSql(SSIRes("SQL_MethodList_IN_SUM_OK"));
	query.execSql(SSIRes("SQL_MethodList_OUT_OK"));
	query.execSql(SSIRes("SQL_MethodList_LOSSES"));
	query.openSql(SSIRes("SQL_MethodList"));
	TStr s1,s2;
	bool f1 = true, f2 = true;
	int in_ok = 0;
	int out_ok = 0;
	int losses = 0;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIValue("IN_OK_Sum",CashToStr(query.fieldByName("IN_OK_Sum")));
		SSIValue("OUT_OK_Sum",CashToStr(query.fieldByName("OUT_OK_Sum")));
		SSIValue("LOSSES_Sum",CashToStr(query.fieldByName("LOSSES_Sum")));
		if( query.fieldByName("forder")[0] == '1' ) {
			SSIBlock("&isOdd",f1);
			SSIBlock("&isEven",!f1);
			f1 = !f1;
			in_ok += TStr(query.fieldByName("IN_OK_Sum")).ToInt();
			out_ok += TStr(query.fieldByName("OUT_OK_Sum")).ToInt();
			losses += TStr(query.fieldByName("LOSSES_Sum")).ToInt();
			s1 += SSIRes("AdmMethodList_Item");
		} else {
			SSIBlock("&isOdd",f2);
			SSIBlock("&isEven",!f2);
			f2 = !f2;
			s2 += SSIRes("AdmMethodList_Item");
		}
		query.next();
	}
	SSIValue("IN_OK_Sum",CashToStr(in_ok));
	SSIValue("OUT_OK_Sum",CashToStr(out_ok));
	SSIValue("LOSSES_Sum",CashToStr(losses));
	if( abs(in_ok + losses) > 0 ) {
		SSIValue("IN_ALL",CashToStr(in_ok + losses));
		SSIValue("OUT_P",-out_ok*100 / (in_ok + losses) );
	} else {
		SSIValue("IN_ALL",CashToStr(0));
		SSIValue("OUT_P","0" );
	}
	if( in_ok > 0 )
		SSIValue("LOSSES_P",-losses*100 / in_ok );
	else
		SSIValue("LOSSES_P",0 );
	SSIBlock("&isOdd",true);
	SSIBlock("&isEven",false);
	s1 += SSIRes("AdmMethodList_Total");
	SSIValue("Stream_MethodList",s1 + SSIRes("AdmMethodList_Div") + s2);
	
	return ScriptPage("AdmMethodList");
}

// ScriptAdmMethodEdit

TStr TMain::ScriptAdmMethodEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return ScriptAdmSetupList();

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		query.execSql(SSIRes("SQL_MethodUpdate"));
	}

	query.openSql(SSIRes("SQL_MethodView"));
	if( !query.eof() )
		QueryToSSI(this,&query);

	return ScriptPage("AdmMethodEdit");
}


//
// ScriptAdmCasinoList
//
TStr TMain::ScriptAdmCasinoList()
{
/*
	TFile f;
	TStr s;
	try {
		char* nm = "adm_casino_list.html";
#ifndef CASINO_TRIO
		nm = "/www/va-bank.com/htdocs/adm/adm_casino_list.html";
#endif
		f.OpenLoad(nm);
		char c = fgetc(f.stream);
		while( c != EOF ) {
			s += c;
			c = fgetc(f.stream);
		}
	} catch(...) {
	}
*/
/*
	if( s.isEmpty() ) {
		m_classes::TMySqlQuery query(p_database);
		SSIBlock("&isCommon",true);
		TStr Date(p_ValueList->Values("Date"));
		s += CasinoPeriodList(this,&query,cdateDAY,Date) + SSIRes("AdmCasinoList_HR");
		s += CasinoPeriodList(this,&query,cdateMONTH,Date) + SSIRes("AdmCasinoList_HR");
		s += CasinoPeriodList(this,&query,cdateYEAR,Date);
	}
	SSIValue("Stream_CasinoList",s);
*/
	TStr DateMonth(p_ValueList->Values("DateMonth"));
	if( DateMonth.Length() < 2 )
		DateMonth = DateTimeToStr(Now(),"%Y-%m-00");

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_CasinoList"));
	TStr s_month1,s,s_month2;
	bool f = true;
	SSIBlock("&isCommon",true);
	while( !query.eof() )
	{
		TStr Date(query.fieldByName("fDate"));
		Date = Date.GetBefore("-")+"-"+Date.CopyAfter("-").CopyBefore("-")+"-00";
		if( Date == DateMonth || Date == query.fieldByName("fDate") ) {
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			SSIBlock("&isMonth",TStr(query.fieldByName("fDate")).Pos("-00") > -1);
			bool isDate = TStr(query.fieldByName("fDate")).Pos("-00") == -1;
			SSIBlock("&isDate",isDate);
			int DayWeek = TStr(query.fieldByName("dayweek")).ToIntDef(-1);
			SSIBlock("&isRed",isDate && ( DayWeek == 1 || DayWeek == 7 ) );
			f = !f;
			SSIValue("fBalance",BalanceToStr(query.fieldByName("fBalance")));
			SSIValue("fIn_SUM",CashToStr(query.fieldByName("fIn_SUM")));
			SSIValue("fOut_SUM",BalanceToStr(query.fieldByName("fOut_SUM")));
			SSIValue("fRF_SUM",BalanceToStr(query.fieldByName("fRF_SUM")));
			if( Date == query.fieldByName("fDate") ) {
				int c1 = (Date.CopyBefore("-")+Date.CopyAfter("-").CopyBefore("-")).ToInt();
				int c2 = (DateMonth.CopyBefore("-")+DateMonth.CopyAfter("-").CopyBefore("-")).ToInt();
				if( c1 >= c2 )
					s_month1 += SSIRes("AdmCasinoList_Item");
				else
					s_month2 += SSIRes("AdmCasinoList_Item");
			} else
				s += SSIRes("AdmCasinoList_Item");
		}
		query.next();
	}
	SSIValue("Stream_CasinoList",s_month1+s+s_month2);

	return ScriptPage("AdmCasinoList");
}


TStr TMain::ScriptAdmDateList()
{
	SSIBlock("&isCommon",true);
	TStr Date(p_ValueList->Values("Date"));
	TStr s;
	m_classes::TMySqlQuery query(p_database);
	s += GamePeriodList(this,&query,cdateDAY,Date) + SSIRes("AdmDateList_HR");
	s += GamePeriodList(this,&query,cdateMONTH,Date) + SSIRes("AdmDateList_HR");
	s += GamePeriodList(this,&query,cdateYEAR,Date);
	SSIValue("Stream_DateList",s);

	return ScriptPage("AdmDateList");
}


TStr TMain::ScriptAdmGCLList()
{
	SSIValue("DateBegin",fDateBegin);
	SSIBlock("&isDateBegin",!fDateBegin.isEmpty());
	SSIValue("DateEnd",fDateEnd);
	SSIBlock("&isDateEnd",!fDateEnd.isEmpty());
	TStr Day(p_ValueList->Values("Day")); 
	SSIValue("Day",Day);
	SSIBlock("&isDay",!Day.isEmpty());

	m_classes::TMySqlQuery query(p_database);

	query.execSql(SSIRes("SQL_Gcl_Tmp"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_index"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Users"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Usersindex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Usersindex2"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Url"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Urlindex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Signups"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Signupsindex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Deposit"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_DepositIndex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Cashout"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_CashoutIndex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Losses"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_LossesIndex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Balance"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Balanceindex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Game_real"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Game_realindex"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Game_fun"));
	query.execSql(SSIRes("SQL_Gcl_Tmp_Game_funindex"));

	SSIValue("WHERE",OnPageWhere("AdmGCLList_Search"));
	SSIValue("SORT",OnPageSort("AdmGCLList_Sort"));

	query.openSql(SSIRes("SQL_Gcl"));
	TStr s;
	bool f = true;
	int sum[10];
	for( int i = 0; i < 10; i++ )
		sum[i] = 0;
	while( !query.eof() )
	{
		int c =
			abs(TStr(query.fieldByName("Hits")).ToInt()) +
			abs(TStr(query.fieldByName("realPlayers")).ToInt()) +
			abs(TStr(query.fieldByName("DepositSum")).ToInt()) +
			abs(TStr(query.fieldByName("CashoutSum")).ToInt());

		QueryToSSI(this,&query);
		SSIValue("DepositSum",CashToStr(query.fieldByName("DepositSum")),false);
		SSIValue("CashoutSum",CashToStr(query.fieldByName("CashoutSum")),false);
		SSIValue("Balance",BalanceToStr(query.fieldByName("Balance")),false);
		SSIValue("FunBets",CashToStr(query.fieldByName("FunBets")),false);
		SSIValue("FunPayout",CashToStr(query.fieldByName("FunPayout")),false);
		SSIValue("FunProfit",BalanceToStr(query.fieldByName("FunProfit")),false);
		SSIValue("RealBets",CashToStr(query.fieldByName("RealBets")),false);
		SSIValue("RealPayout",CashToStr(query.fieldByName("RealPayout")),false);
		SSIValue("RealProfit",BalanceToStr(query.fieldByName("RealProfit")),false);

		sum[0] += query.fieldByNameAsInt("Hits");
//		sum[1] += query.fieldByNameAsInt("HitsLoad");
		sum[2] += query.fieldByNameAsInt("Hosts");
		sum[3] += query.fieldByNameAsInt("Signups");
		sum[4] += query.fieldByNameAsInt("DepositSum");
		sum[5] += query.fieldByNameAsInt("Balance");
		sum[6] += query.fieldByNameAsInt("realPlayers");
		sum[7] += query.fieldByNameAsInt("realRounds");
		sum[8] += query.fieldByNameAsInt("funPlayers");
		sum[9] += query.fieldByNameAsInt("funRounds");

		if( c > 0 ) {
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			s += SSIRes("AdmGCLList_Item");
		}
		query.next();
	}
	SSIValue("Title","GCL");
	SSIValue("Hits",sum[0],false);
//	SSIValue("HitsLoad",sum[1],false);
	SSIValue("Hosts",sum[2],false);
	SSIValue("Signups",sum[3],false);
	SSIValue("DepositSum",CashToStr(sum[4]),false);
	SSIValue("Balance",BalanceToStr(sum[5]),false);
	SSIValue("realPlayers",sum[6],false);
	SSIValue("realRounds",sum[7],false);
	SSIValue("funPlayers",sum[8],false);
	SSIValue("funRounds",sum[9],false);
	s = SSIRes("AdmGCLList_ItemTitle")+s;
/*
	SSIValue("Title","CASINO");
	query.openSql(SSIRes("SQL_Gcl_Casino_Deposit"));
	SSIValue("DepositSum",CashToStr(query.fieldByName("fsum")),false);
	query.openSql(SSIRes("SQL_Gcl_Casino_Balance"));
	SSIValue("Balance",BalanceToStr(query.fieldByName("fsum")),false);
	query.openSql(SSIRes("SQL_Gcl_Casino_Hits"));
	QueryToSSI(this,&query);
//	query.openSql(SSIRes("SQL_Gcl_Casino_URLLoad"));
//	QueryToSSI(this,&query);
	query.openSql(SSIRes("SQL_Gcl_Casino_Fun"));
	QueryToSSI(this,&query);
	query.openSql(SSIRes("SQL_Gcl_Casino_Real"));
	QueryToSSI(this,&query);
	s = SSIRes("AdmGCLList_ItemTitle")+s;
*/

	SSIValue("Stream_gcl",s);

	return ScriptPage("AdmGCLList");
}

// ScriptAdmUrlList

TStr TMain::ScriptAdmUrlList()
{
	SSIValue("DateBegin",fDateBegin);
	SSIBlock("&isDateBegin",!fDateBegin.isEmpty());
	SSIValue("DateEnd",fDateEnd);
	SSIBlock("&isDateEnd",!fDateEnd.isEmpty());
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_UrlList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		SSIValue("fBet",CashToStr(query.fieldByName("fBet")));
		SSIValue("fWin",CashToStr(query.fieldByName("fWin")));
		s += SSIRes("AdmUrlList_Item");
		query.next();
	}
	SSIValue("Stream_UrlList",s);

	return ScriptPage("AdmUrlList");
}

// ScriptAdmIDList

TStr TMain::ScriptAdmIDList()
{
	SSIValue("DateBegin",fDateBegin);
	SSIBlock("&isDateBegin",!fDateBegin.isEmpty());
	SSIValue("DateEnd",fDateEnd);
	SSIBlock("&isDateEnd",!fDateEnd.isEmpty());
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_IDList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		SSIValue("fBet",CashToStr(query.fieldByName("fBet")));
		SSIValue("fWin",CashToStr(query.fieldByName("fWin")));
		s += SSIRes("AdmIDList_Item");
		query.next();
	}
	SSIValue("Stream_IDList",s);

	return ScriptPage("AdmIDList");
}

//
bool TMain::TRCheck(const TStr& ID)
{
	m_classes::TMySqlQuery query(p_database);
	query.openSql(
		"SELECT fid_status,fid_system "
		"FROM casino_pay.pay  "
		"WHERE fOrder="+ID);
	if( !query.eof() ) {
		TStr s;
		switch( query.fieldByNameAsInt("fid_status") )
		{
			case 10: s = "ERROR"; break;
			case 11: s = "OK"; break;
			case 20: if( query.fieldByNameAsInt("fid_system") == 2 ) s = "OK"; break;
		}
		if( !s.isEmpty() ) {
			if( Connect() ) {
				Send("<casino command=pay><pay status="+s+" id="+ID+" /></casino>");
				s = Read();
			}
		}
		return true;
	}
	query.execSql("UPDATE log_pay_order SET fid_status=4,flast=now() WHERE id="+ID);
	return false;
}
//

// ScriptAdmTRList

TStr TMain::ScriptAdmTRList()
{
	if( fEditID > 0 )
		return ScriptAdmTREdit();

	SSIValue("WHERE",OnPageWhere("AdmTRList_Search"));
	SSIValue("SORT",OnPageSort("AdmTRList_Sort"));

	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Check").isEmpty() ) {
		TRCheck(p_ValueList->Values("Check"));
	} else
	if( !p_ValueList->Values("RF").isEmpty() ) {
		TStr ID(p_ValueList->Values("RF"));
		query.openSql(
			"SELECT r.fid_user,r.fid_system,r.fcash,r.fid_status,q.fid_status as qstatus,r.fComment,r.fInfo "
			"FROM log_pay_order as r LEFT JOIN log_pay_order as q ON r.id = q.fid_order "
			"WHERE r.ID="+ID);
		if( !query.eof() ) {
			if(
				query.fieldByNameAsInt("fid_status") == 11 &&
				query.fieldByNameAsInt("fid_system") == 2 &&
				query.fieldByNameAsInt("fcash") > 0 &&
				query.fieldByName("qstatus")[0] == '\0'
			) {
				TStr Comment(query.fieldByName("fComment"));
				TStr UserID(query.fieldByName("fid_user"));
				TStr Info(query.fieldByName("fInfo"));
				query.execSql(
					"INSERT INTO log_pay_order ("
					"  fdate,"
					"  ftime,"
					"  fid_user,"
					"  fid_system,"
					"  fcash,"
					"  fid_status,"
					"  fid_order,"
					"  flast "
					") VALUES ("
					"  now(),"
					"  now(),"
					+UserID+","
					+TStr(query.fieldByName("fid_system"))+","
					"-"+TStr(query.fieldByName("fcash"))+","
					+TStr("32")+","
					+ID+","
					"now()"
					")");
				TStr Body;
				Body += "TR\nhttps://www.va-bank.com/adm/?ADMTRLIST&Edit="+TStr(query.last_id());
				Body += "\n\nUser\nhttps://www.va-bank.com/adm/?ADMUSERLIST&Edit="+UserID;
				Body += "\n\n"+Comment;
				Body += "\n\n"+Info;
				SendMail(
					"soft@globo.ru",
					"VA-BANK",
					("RF id="+ID+" userid="+UserID).c_str(),
					Body.c_str(),
					"soft@globo.ru");
			}
		}
	}

	bool fPassive =
		p_ValueList->Values("StatusID") == "11" &&
		p_ValueList->Values("UsersID").isEmpty();
	SSIBlock("&isPASSIVE",fPassive);

	query.openSql(SSIRes("SQL_TRListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	SSIBlock("&isPASSIVE",false);
	query.openSql(SSIRes("SQL_TRList"));
	TStr s;
	bool f = true;
	int LastID = -1;
	if( query.eof() )
		s = SSIRes("AdmTRList_Empty");
	else
		while( !query.eof() )
		{
			bool fView = true;
			if( query.fieldByName("fPassive")[0] == '1' ) {
				if(
					p_ValueList->Values("UsersID").isEmpty() &&
					p_ValueList->Values("Page").ToIntDef(1) > 1
				)
					fView = false;
			}
			if( fView ) {
				if( LastID != query.fieldByNameAsInt("ID") ) {
					QueryToSSI(this,&query);
					SSIBlock("&isOdd",f);
					SSIBlock("&isEven",!f);
					f = !f;
					TXML xml;
					TStr fInfo;
					xml.FromStr(query.fieldByName("fInfo"));
					for( unsigned int i = 0; i < xml.NodesCount(); i++ )
					{
						TStr Value(xml.GetNode(i)->VarValue("value"));
						if( !Value.isEmpty() ) {
							if( xml.GetNode(i)->GetName() == "CCNUM" ) {
								for( int i = 0; i < Value.Length()-4; i++ )
									Value[i] = '*';
							}
							fInfo += "\n" + xml.GetNode(i)->GetName() + "=" + __unescape(Value);
						}
					}
					SSIBlock("&isCheck",
						(query.fieldByNameAsInt("systemID") == 1 || query.fieldByNameAsInt("systemID") == 3) &&
						query.fieldByNameAsInt("statusID") == 3 );
					SSIBlock("&isPayComment", query.fieldByNameAsInt("pay_user_comment") );
					SSIBlock("&isOrderID",query.fieldByNameAsInt("OrderID") > 0);
					SSIBlock("&isRF",query.fieldByName("RFID")[0] != '\0');
					SSIBlock("&isRFDO",query.fieldByNameAsInt("StatusID") == 11 && query.fieldByNameAsInt("SystemID") == 2 && query.fieldByName("RFID")[0] == '\0');
					SSIBlock("&isBlock",query.fieldByName("fActive")[0] == '0');
					SSIValue("fInfo",StrToHTML(fInfo));
					SSIValue("fCash",CashToStr(query.fieldByName("fCash")));
					SSIValue("UserLogin",StrToHTML(__unescape(query.fieldByName("UserLogin"))));
					SSIValue("fComment",StrToHTML(query.fieldByName("fComment")));
					SSIBlock("&isComment",strlen(query.fieldByName("fComment"))>0);
					SSIValue("UserComment",StrToHTML(query.fieldByName("UserComment")),false);
					SSIBlock("&isUserComment",strlen(query.fieldByName("UserComment"))>2,false);

					SSIBlock("&isWireOk", (query.fieldByNameAsInt("systemID") == 4 && query.fieldByNameAsInt("statusID") == 1 ));

					LastID = query.fieldByNameAsInt("ID");
					s += SSIRes("AdmTRList_Item");
				}
			}
			query.next();
		}
	SSIValue("Stream_TRList",s);

	query.openSql(SSIRes("SQL_PayStatus"));
	SSIValue("stream_paystatus",QueryToCombo(&query,p_ValueList->Values("StatusID")));
	query.openSql(SSIRes("SQL_PaySystem"));
	SSIValue("stream_paysystem",QueryToCombo(&query,p_ValueList->Values("SystemID")));

	return ScriptPage("AdmTRList");
}


// ScriptAdmTRCreate

TStr TMain::ScriptAdmTRCardCreate()
{
	if( !p_ValueList->Values("submit").isEmpty() ) {
	/*
		<adm command="casher">
		<info cash="20" system="credit" type="in" info="
			ttype=VISA;
			holder=1;
			num=1;
			dateend=0103;
			cvc2=1;
			bank=1;
			city=1;
			state=3;
			zip=1;
			country=2;
			phones=1;
			code_phones=1;
			address=1;
			club_check=2;" />
		</adm>
	*/
		TStr s("<info");
		s += " cash=" + p_ValueList->Values("Amount");
		s += " system=credit";
		s += " type=in";
		s += " info=\"";
		s += "ttype="+p_ValueList->Values("TType")+";";
		s += "holder="+p_ValueList->Values("Name")+";";
		s += "num="+p_ValueList->Values("num")+";";
		TStr Month(p_ValueList->Values("month"));
		if( Month.Length() == 1 )
			Month = "0" + Month;
		s += "dateend="+Month+p_ValueList->Values("year").CopyAfter(1)+";";
		s += "cvc2="+p_ValueList->Values("cvc2")+";";
		s += "bank="+p_ValueList->Values("bank")+";";
		s += "zip="+p_ValueList->Values("zip")+";";
		s += "city="+p_ValueList->Values("city")+";";
		s += "state="+p_ValueList->Values("state")+";";
		s += "country="+p_ValueList->Values("country")+";";
		s += "phones="+p_ValueList->Values("phones")+";";
		s += "code_phones="+p_ValueList->Values("code_phones")+";";
		s += "address="+p_ValueList->Values("address")+";";
		if( !p_ValueList->Values("club").isEmpty() )
			s += "club_check=2;";
		s += "\" />";
		s += "<user id="+p_ValueList->Values("userid")+" />";
		if( Connect() ) {
			Send("<adm command=casher>"+s+"</adm>");
			s = Read();
			TXML xml(s.c_str());
			fEditID = xml.VarValue("adm/pay/id").ToIntDef(0);
			if( fEditID > 0 ) {
				p_ValueList->SetValue("Submit","");
				return ScriptAdmTREdit();
			}
		}
		SSIBlock("&isError",true);
	}

	m_classes::TMySqlQuery query(p_database);
	for( int i = 0; i < p_ValueList->Count(); i++ )
		SSIValue(p_ValueList->Names(i),p_ValueList->Values(i),false);

	SSIBlock("&isCLUB",!p_ValueList->Values("club").isEmpty());
	SSIBlock("&isVISA",p_ValueList->Values("ttype") == "VISA");
	SSIBlock("&isMASTERCARD",p_ValueList->Values("ttype") == "MASTERCARD");
	SSIBlock("&isSelectMonth"+p_ValueList->Values("month"),true,false);
	SSIBlock("&isSelectYear"+p_ValueList->Values("Year"),true,false);
	query.openSql(SSIRes("SQL_Country"));
	SSIValue("stream_country",QueryToCombo(&query,p_ValueList->Values("country")));
	query.openSql(SSIRes("SQL_State"));
	SSIValue("stream_state",QueryToCombo(&query,p_ValueList->Values("state")));


	return ScriptPage("AdmTRCardCreate");
}


// ScriptAdmTREdit

TStr TMain::ScriptAdmTREdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return Script("AdmTRList");
	SSIValue("fEditID",fEditID);

	if( !p_ValueList->Values("Check").isEmpty() ) 
		TRCheck(p_ValueList->Values("Check"));

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		SSIValue("Comment",StrToSQL(p_ValueList->Values("Comment")));
		query.execSql(SSIRes("SQL_TRUpdate"));
	}

	TStr Status;
	if( !p_ValueList->Values("Mnogo").isEmpty() ) {
		Status = "wait";
	} else
	if( !p_ValueList->Values("Out").isEmpty() )
		Status = "out";
	else
	if( !p_ValueList->Values("Cancel").isEmpty() )
		Status = "cancel";
	else
	if( !p_ValueList->Values("Block").isEmpty() )
		Status = "block";
	if( !Status.isEmpty() ) {
		TStr xml;
		xml =
			"<casino command=pay>"
			"<pay id="+TStr(fEditID)+" status="+Status;
		TStr Cash(p_ValueList->Values("cashupdate"));
		if( !Cash.isEmpty() )
			xml += " cashupdate="+TStr(StrToCash(Cash));
/*
		TStr CashBonus(p_ValueList->Values("cashbonus"));
		if( !CashBonus.isEmpty() )
			xml += " cashbonus="+TStr(StrToCash(CashBonus));
*/
		xml += " />";
		xml += "</casino>";
		if( Connect() ) {
			Send(xml);
			Read();
		}
	}

	query.openSql(SSIRes("SQL_TREdit"));
	if( !query.eof() ) {
		TStr UserID(query.fieldByName("UserID"));
		int  Cash = TStr(query.fieldByName("fcash")).ToInt();

		SSIValue("Stream_IP",OnStreamUserIP(UserID));
		SSIValue("Stream_Game1",OnStreamUserGame(UserID,1));
		SSIValue("Stream_Game2",OnStreamUserGame(UserID,2));
		SSIValue("Stream_PaySystem",OnStreamUserPaySystem(UserID));
		SSIValue("Stream_Order",OnStreamUserOrderMethod(UserID,fEditID));

		QueryToSSI(this,&query);

		SSIValue("UserComment",StrToHTML(query.fieldByName("UserComment")),false);
		SSIBlock("&isUserComment",strlen(query.fieldByName("UserComment"))>2,false);

		SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
		SSIValue("fCash_Free",CashToStr(query.fieldByName("fCash_Free")));
		SSIValue("fCash_FreeBets",CashToStr(query.fieldByName("fCash_FreeBets")));

		SSIValue("UserEmail",StrToHTML(__unescape(query.fieldByName("UserEmail"))));
		SSIValue("UserLogin",StrToHTML(__unescape(query.fieldByName("UserLogin"))));
		SSIValue("fInfo",StrToHTML(query.fieldByName("fInfo")));
		SSIValue("fCash",CashToStr(Cash));

		TXML xml(query.fieldByName("UserInfo"));
		TXMLNode* p = xml.FindNode("user");
		if( p != NULL ) {
			if( p->VarValue("Bonus") == '1' ) {
				SSIBlock("&notBonus",false);
				SSIBlock("&isBonus",true);
			} else {
				SSIBlock("&notBonus",true);
				SSIBlock("&isBonus",false);
			}
			SSIValue("UserPhone",__unescape(p->VarValue("Phone")));
			TStr Date(p->VarValue("Date"));
			TStr Year(Date.GetBefore("|"));
			Year = Year.ToIntDef(1) + 1919;
			TStr Month(Date.GetBefore("|"));
			SSIValue("UserBirthDay",Date+"."+Month+"."+Year);
		}

		TStr StatusID(query.fieldByName("StatusID"));
		bool ADM_OUT = StatusID == payADM && Cash <= 0;
		if( ADM_OUT && query.fieldByNameAsInt("systemID") == 61 ) { // Mnogoru
			SSIBlock("&isAdmMnogoru",ADM_OUT);
		} else {
			SSIBlock("&isAdmOut",ADM_OUT);
			if( ADM_OUT )
				SSIValue("CashUpdate",CashToStr(-Cash));
		}


		SSIBlock("&isCheck",
			(query.fieldByNameAsInt("systemID") == 1 || query.fieldByNameAsInt("systemID") == 3) &&
			query.fieldByNameAsInt("statusID") == 3 );

	}

	return ScriptPage("AdmTREdit");
}


// ScriptAdmTRCreate

TStr TMain::ScriptAdmTRCreate()
{
	m_classes::TMySqlQuery query(p_database);

	TStr UserID(p_ValueList->Values("UserID"));
	TStr Amount(p_ValueList->Values("Amount"));
	TStr Status(p_ValueList->Values("Status"));
	TStr System(p_ValueList->Values("System"));
	TStr Comment(p_ValueList->Values("Comment"));
	SSIValue("UserID",UserID);
	if( !UserID.isEmpty() ) {
		query.openSql("select ID from tbl_users where factive=1 and id="+UserID);
		if( query.eof() )
			UserID.Clear();
	}
	if(
		!p_ValueList->Values("Submit").isEmpty() &&
		UserID.ToIntDef(0) > 0 &&
		Status.ToIntDef(0) > 0 &&
		System.ToIntDef(0) > 0
	) {
		SSIValue("Amount",StrToCash(Amount));
		if( Status == "11" )
			SSIValue("Status","3");
		else
			SSIValue("Status",Status);
		SSIValue("System",System);
		SSIValue("Comment",StrToSQL(Comment));
		query.execSql(SSIRes("SQL_TRCreate"));
		fEditID = query.last_id();
		if( Status == "11" ) {
			if( Connect() ) {
				TStr s;
				s = "<casino command=pay><pay status=ok id="+TStr(fEditID)+" /></casino>"; 
				Send(s);
				s = Read();
			}
		}
		return ScriptAdmTREdit();
	}
	SSIValue("Comment",Comment);
	SSIValue("Amount",Amount);
	{
		TStr s;
		query.openSql(
			"select ID,fTitle from const_paysystem where forder=1 and (fid_admin=0 or fid_admin="+GetSessionValue("fAdm")+") order by id");
		while( !query.eof() )
		{
			s += "<option ";
			if( System == query.fields(0) )
				s += "selected ";
			s += " value=\""+TStr(query.fields(0))+"\">"+TStr(query.fields(1));
			query.next();
		}
		SSIValue("stream_system",s);
	}
	{
		TStr s;
		query.openSql(
			"select ID,fTitle from const_paystatus where (fid_admin=0 or fid_admin="+GetSessionValue("fAdm")+") order by id");
		while( !query.eof() )
		{
			s += "<option ";
			if( Status == query.fields(0) )
				s += "selected ";
			s += " value=\""+TStr(query.fields(0))+"\">"+TStr(query.fields(1));
			query.next();
		}
		SSIValue("stream_status",s);
	}

	return ScriptPage("AdmTRCreate");
}


// ScriptAdmOnlineList()

	struct TAdmSession {
		static int i;

		TStr GUID;

		int  UserID;
		TStr UserLogin;
		TStr UserNick;
		TStr UserCash;
		TStr UserMode;
		TStr IP_HTTP;
		TStr IP_Connect;

		TStr RefID;
		TStr Create;
		int  Last;
		int  Count;

		TStr Game;
		TStr GameID;
		bool GameMulti;

		bool f;

		void Clear() {
			GUID = "";
			UserID = 0;
			UserLogin.Clear();
			UserNick.Clear();
			UserMode = "gift";
			UserCash = "";
		}

		TStr ToSSI(
				m_ssi::TSSIControl *ssi,
				m_classes::TMySqlQuery &query)
		{
			ssi->SSIValue("Num",i++,false);
			ssi->SSIValue("GUID",GUID,false);
			if( UserID > 0 ) {
				ssi->SSIValue("UserID",UserID);
				ssi->SSIValue("UserMode",UserMode);
			} else {
				ssi->SSIValue("UserMode","gift");
				ssi->SSIValue("UserID","");
			}
			ssi->SSIValue("UserLogin",StrToHTML(__unescape(UserLogin)),false);
			ssi->SSIValue("UserNick",StrToHTML(__unescape(UserNick)),false);
			ssi->SSIValue("UserCash",UserCash);
			ssi->SSIValue("Game",Game);
			ssi->SSIValue("Last",Last);
			ssi->SSIValue("IP_HTTP",IP_HTTP,false);
			ssi->SSIValue("IP_Connect",IP_Connect,false);
			ssi->SSIValue("Create",Create);
			ssi->SSIValue("Count",Count);
//			ssi->SSIValue("GameID",GameID);
			ssi->SSIBlock("&isGame",!Game.isEmpty(),false);
//			ssi->SSIBlock("&isGame",!GameID.isEmpty(),false);
			ssi->SSIBlock("&isMulti",GameMulti,false);
			ssi->SSIValue("RefID",RefID,false);
			query.openSql(ssi->SSIRes("SQL_OnlineList_Refid"));
			QueryToSSI(ssi,&query);
			if( !query.eof() ) {
				ssi->SSIBlock("&isReferrer",strlen(query.fieldByName("freferrer"))>10);
				ssi->SSIBlock("&isID",TStr(query.fieldByName("fid")).Trim().Length()>5);
			} else {
				ssi->SSIBlock("&isReferrer",false);
				ssi->SSIBlock("&isID",false);
			}
			return ssi->SSIRes("AdmOnlineList_Item");
		}
	};

	int TAdmSession::i = 1;

TStr TMain::ScriptAdmOnlineList()
{
	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("msg").isEmpty() ) {
		TStr msg(__escape(p_ValueList->Values("msg")));
		TStr s("<adm command=\"msg\" msg=\""+msg+"\">");
		for( int i = 0; i < p_ValueList->Count(); i++ )
			if( p_ValueList->Names(i) == "session" )
				s += "<adm session=\""+p_ValueList->Values(i)+"\" >";
		s += "</adm>";
		if( Connect() ) {
			Send(s);
			s = Read();
		}
	}                          

	unsigned int Count;
	TStr s("<adm command=view />");
	if( Connect() ) {
		Send(s);
		s = Read();
	}
	TXML xml(s.c_str());
	s.Clear();

	SSIBlock("&isCommon",true);
	
	TAdmSession list[1000];
	int count = 0;
	for( unsigned int i = 0; i < xml.NodesCount(); i++ )
	{
		TXMLNode* p = xml.GetNode(i);
		if( p->GetName() == "session" ) {
			int UserID = p->VarValue("user").ToIntDef(0);
//			if( UserID.ToIntDef(0) > 0 ) {
				bool f = true;
				int j = 0;
				if( UserID > 0 )
					for( j = 0; j < count; j++ )
						if( list[j].UserID == UserID ) {
							f = false;
							break;
						}
				if( f ) {
					j = count;
					list[j].Clear();
					list[j].UserID = UserID;
					list[j].f = false;
					count++;
				}
				list[j].GUID = p->VarValue("guid");
				list[j].Game = p->VarValue("game");
				list[j].GameID = p->VarValue("game_id");
				list[j].GameMulti = !p->VarValue("gamemulti").isEmpty();
				list[j].IP_HTTP = p->VarValue("IP_HTTP");
				list[j].IP_Connect = p->VarValue("IP_Connect");
				list[j].Last = p->VarValue("last").ToIntDef(0);
				list[j].Count = p->VarValue("Count").ToIntDef(0);
				list[j].Create = p->VarValue("Create");
				list[j].RefID = p->VarValue("RefID");
//			}
		} else
		if( p->GetName() == "user" ) {
			int UserID = p->VarValue("id").ToIntDef(0);
			for( int j = 0; j < count; j++ )
				if( list[j].UserID == UserID ) {
					list[j].UserLogin = p->VarValue("Login");
					list[j].UserCash = CashToStr(p->VarValue("Cash"));
					if( p->VarValue("Status") == "1" )
						list[j].UserMode = "real";
					else
						list[j].UserMode = "fun";
				}
		}
	}
	TStr RefIDList;
	TStr sRealGame,sReal,sFun,sFunGame;
	int iReal = 0, iFun = 0, iGame = 0, iC = 0, iRealC = 0, iFunC = 0, iGameC = 0;
	SSIBlock("&isOdd",true);
	SSIBlock("&isEven",false);
	for( int i = 0; i < count; i++ )
	{
			if( list[i].Last < 60 )
				iC++;
			if( !list[i].Game.isEmpty() ) {
				iGame++;
				if( list[i].Last < 60 )
					iGameC++;
			}
			TStr ss(list[i].ToSSI(this,query));
			if( list[i].UserMode == "real" ) {
				iReal++;
				if( list[i].Last < 60 )
					iRealC++;
				if( list[i].Game.isEmpty() )
					sReal += ss;
				else
					sRealGame += ss;
			} else
			if( list[i].UserMode == "fun" ) {
				iFun++;
				if( list[i].Last < 60 )
					iFunC++;
				if( list[i].Game.isEmpty() )
					sFun += ss;
				else
					sFunGame += ss;
			} else
				s += ss;
			if( !list[i].RefID.isEmpty() ) {
				if( !RefIDList.isEmpty() )
					RefIDList += ",";
				RefIDList += "'"+list[i].RefID+"'";
			}
			list[i].f = true;
	}
	SSIValue("OnlineCount",count);
	SSIValue("OnlineCountC",iC);
	SSIValue("OnlineFunCount",iFun);
	SSIValue("OnlineGameCount",iGame);
	SSIValue("OnlineRealCount",iReal);
	SSIValue("OnlineFunCountC",iFunC);
	SSIValue("OnlineGameCountC",iGameC);
	SSIValue("OnlineRealCountC",iRealC);
	s = sReal + sRealGame + sFun + sFunGame + s;
	if( s.Length() == 0 )
		s = SSIRes("AdmOnlineList_Empty");
	SSIValue("Stream_OnlineList",s);

	//
	if( !RefIDList.isEmpty() )
		RefIDList = "WHERE id not in ("+RefIDList+")";
	SSIValue("WHERE",RefIDList);
	query.openSql(SSIRes("SQL_OnlineList_load"));
	s.Clear();
	f = true;
	while( !query.eof() ) {
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		QueryToSSI(this,&query);
		SSIBlock("&isReferrer",TStr(query.fieldByName("freferrer")).Trim().Length()>10);
		SSIBlock("&isID",TStr(query.fieldByName("fid")).Trim().Length()>5);
		s += SSIRes("AdmOnlineList_Load");
		query.next();
	}
	SSIValue("Stream_LoadList",s);

	return ScriptPage("AdmOnlineList");
}


// ScriptAdmUserPaymentList

TStr TMain::ScriptAdmUserPaymentList()
{
	SSIValue("Stream_PayList",OnStreamUserGamePay(p_ValueList->Values("ID")));

	return ScriptPage("AdmUserPaymentList");
}


// ScriptAdmPayList

TStr TMain::ScriptAdmPayList()
{
	SSIValue("WHERE",OnPageWhere("AdmPayList_Search"));

/*
	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_PayListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_PayList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmPayList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("fCash",CashToStr(query.fieldByName("fCash")));
			SSIValue("fBalance",CashToStr(query.fieldByName("fBalance")));
			s += SSIRes("AdmPayList_Item");
			query.next();
		}
	SSIValue("Stream_PayList",s);
	query.openSql(SSIRes("SQL_PaySystem"));
	SSIValue("stream_paysystem",QueryToCombo(&query,p_ValueList->Values("SystemID")));
*/
	SSIValue("Stream_PayList",OnStreamUserGamePay(p_ValueList->Values("ID")));

	return ScriptPage("AdmPayList");
}

// ScriptAdmPlayerList

TStr TMain::ScriptAdmPlayerList()
{
	if( fEditID > 0 )
		return ScriptAdmUserEdit();

	TPageWhere r = OnPageWhereMulti("AdmPlayerList_Search");
	SSIValue("WHERE",r.Search0);
	SSIValue("WHERE_PERIOD",r.Search[0]);
	SSIValue("SORT",OnPageSort("AdmPlayerList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	m_classes::TMySqlQuery q(p_database);
	query.execSql(SSIRes("SQL_PlayerList_History"));
	query.execSql(SSIRes("SQL_PlayerList_History_Index"));
	query.execSql(SSIRes("SQL_PlayerList_Losses"));
	query.execSql(SSIRes("SQL_PlayerList_Losses_Index"));
	query.execSql(SSIRes("SQL_PlayerList_Deposit"));
	query.execSql(SSIRes("SQL_PlayerList_Deposit_Index"));
	query.execSql(SSIRes("SQL_PlayerList_User"));
	query.execSql(SSIRes("SQL_PlayerList_UserHistory"));
	query.execSql(SSIRes("SQL_PlayerList_UserLosses"));
	query.execSql(SSIRes("SQL_PlayerList_UserDeposit"));

	unsigned int Count;
	query.openSql(SSIRes("SQL_PlayerListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_PlayerList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmPlayerList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			if( strlen(query.fieldByName("DepositCount")) == 0 )
				SSIValue("DepositCount","0");
			if( strlen(query.fieldByName("LossesCount")) == 0 )
				SSIValue("LossesCount","0");
			if( strlen(query.fieldByName("RealRounds")) == 0 )
				SSIValue("RealRounds","0");
			if( strlen(query.fieldByName("RealSessions")) == 0 )
				SSIValue("RealSessions","0");
			SSIValue("UserLogin",StrToHTML(__unescape(query.fieldByName("UserLogin"))));
			SSIValue("DepositSum",CashToStr(query.fieldByName("DepositSum")));
			SSIValue("LossesSum",CashToStr(query.fieldByName("LossesSum")));
			SSIValue("RealCash",CashToStr(query.fieldByName("RealCash")));
			SSIValue("RealBets",CashToStr(query.fieldByName("RealBets")));
			SSIValue("RealProfit",BalanceToStr(query.fieldByName("RealProfit")));
			SSIValue("RealPayout",CashToStr(query.fieldByName("RealPayout")));
			SSIBlock("&isRealNegative",query.fieldByName("RealProfit")[0] == '-');
			SSIBlock("&isBlock",query.fieldByName("fActive")[0] == '0');

			s += SSIRes("AdmPlayerList_Item");
			query.next();
		}
	SSIValue("Stream_PlayerList",s);

	return ScriptPage("AdmPlayerList");
}

// ScriptAdmUserList

TStr unescape(char *url)
{
	TStr s(url);
	unescapeUrl(s.c_str());
	return s.Copy(0,strlen(s.c_str()));
}

TStr TMain::ScriptAdmUserActionList()
{
	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_UserActionListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_UserActionList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmUserActionList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
			SSIValue("fNick",StrToHTML(__unescape(query.fieldByName("fNick"))));
			SSIValue("fEMail",StrToHTML(__unescape(query.fieldByName("fEMail"))));
			s += SSIRes("AdmUserActionList_Item");
			query.next();
		}
	SSIValue("Stream_UserActionList",s);

	return ScriptPage("AdmUserActionList");
}

// ScriptAdmUserActionEdit

TStr TMain::ScriptAdmUserActionEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return ScriptAdmUserActionList();

	long dt = Now();

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		TStr ActionDate(
				p_ValueList->Values("Year") + "-" +
				p_ValueList->Values("Month") + "-" +
				p_ValueList->Values("Day"));
		SSIValue("fComment",StrToSQL(p_ValueList->Values("fcomment")));
		SSIValue("fDate_plan",StrToSQL(ActionDate));
		query.execSql(SSIRes("SQL_UserActionUpdate"));
	}

	query.openSql(SSIRes("SQL_UserActionView"));
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		dt = StrToDateTime(query.fieldByName("fDate_Plan"));
		SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
	}
	SSIBlock("&isSelectDay"+TStr(DateTimeToDay(dt)),true);
	SSIBlock("&isSelectMonth"+TStr(DateTimeToMonth(dt)),true);
	SSIBlock("&isSelectYear"+TStr(DateTimeToYear(dt)),true);

	return ScriptPage("AdmUserActionEdit");
}

// AdmUserSearchList

TStr TMain::ScriptAdmUserSearchList()
{
	SSIValue("WHERE_ORDER",OnPageWhere("AdmUserSearchList_SearchOrder"));
	SSIValue("WHERE",OnPageWhere("AdmUserSearchList_Search"));
	SSIValue("SORT",OnPageSort("AdmUserSearchList_Sort"));

	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	TStr s;
	query.execSql(SSIRes("SQL_UserSearchList_Deposit"));
	query.execSql(SSIRes("SQL_UserSearchList_DepositIndex"));
	query.execSql(SSIRes("SQL_UserSearchList_Cashout"));
	query.execSql(SSIRes("SQL_UserSearchList_CashoutIndex"));
	query.execSql(SSIRes("SQL_UserSearchList_RF"));
	query.execSql(SSIRes("SQL_UserSearchList_RFIndex"));
	query.execSql(SSIRes("SQL_UserSearchList_Balance"));
	query.execSql(SSIRes("SQL_UserSearchList_BalanceIndex"));
/*
	// Active = 1
	SSIBlock("&isOdd",true);
	SSIBlock("&isEven",false);
	SSIBlock("&isActive",true);
	SSIBlock("&isAll",false);
	query.openSql(SSIRes("SQL_UserSearchListCount"));
	QueryToSSI(this,&query);
	SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
	SSIValue("fDeposit",CashToStr(query.fieldByName("fDeposit")));
	SSIValue("fCashout",CashToStr(query.fieldByName("fCashout")));
	SSIValue("fRF",CashToStr(query.fieldByName("fRF")));
	s += SSIRes("AdmUserSearchList_Title");
	// Active
	SSIBlock("&isOdd",false);
	SSIBlock("&isEven",true);
	SSIBlock("&isActive",false);
	SSIBlock("&isAll",true);
	query.openSql(SSIRes("SQL_UserSearchListClose"));
	SSIValue("fDepositClose",CashToStr(query.fieldByName("fDepositClose")));
	SSIValue("fLossesClose",CashToStr(query.fieldByName("fLossesClose")));
//	query.openSql(SSIRes("SQL_UserSearchListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	QueryToSSI(this,&query);
	SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
	SSIValue("fDeposit",CashToStr(query.fieldByName("fDeposit")));
	SSIValue("fCashout",CashToStr(query.fieldByName("fCashout")));
	SSIValue("fRF",CashToStr(query.fieldByName("fRF")));
	s += SSIRes("AdmUserSearchList_Title");
*/
	query.openSql(SSIRes("SQL_UserSearchListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	//
	query.openSql(SSIRes("SQL_UserSearchList"));
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmUserSearchList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			SSIBlock("&isReferrer",strlen(query.fieldByName("fReg_referrer")) > 4);
			SSIBlock("&isGcl",strlen(query.fieldByName("fReg_gcl")) > 4);
			TStr Referrer(query.fieldByName("fReg_Referrer"));
			if( Referrer.CopyBefore(7) == "http://" )
				Referrer = Referrer.CopyAfter(6);
			if( Referrer.CopyBefore(8) == "https://" )
				Referrer = Referrer.CopyAfter(7);
			SSIValue("fReferrer",Referrer.CopyBefore(30));
			f = !f;
			SSIBlock("&isBlock",query.fieldByName("fActive")[0] == '0');
			SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
			SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
			SSIValue("fDeposit",CashToStr(query.fieldByName("fDeposit")));
			SSIValue("fCashout",CashToStr(query.fieldByName("fCashout")));
			SSIValue("fRF",CashToStr(query.fieldByName("fRF")));
			SSIValue("fDepositClose",CashToStr(query.fieldByName("fDepositClose")));
			SSIValue("fLossesClose",CashToStr(query.fieldByName("fLossesClose")));
			s += SSIRes("AdmUserSearchList_Item");
			query.next();
		}
	SSIValue("Stream_UserSearchList",s);

	return ScriptPage("AdmUserSearchList");
}

// AdmGCLSearchList

TStr TMain::ScriptAdmGCLSearchList()
{
	SSIValue("WHERE_ORDER",OnPageWhere("AdmGCLSearchList_SearchOrder"));
	SSIValue("WHERE",OnPageWhere("AdmGCLSearchList_Search"));
	SSIValue("SORT",OnPageSort("AdmGCLSearchList_Sort"));

	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	TStr s;
	query.execSql(SSIRes("SQL_GCLSearchList_Deposit"));
	query.execSql(SSIRes("SQL_GCLSearchList_DepositIndex"));
	query.execSql(SSIRes("SQL_GCLSearchList_Cashout"));
	query.execSql(SSIRes("SQL_GCLSearchList_CashoutIndex"));
	query.execSql(SSIRes("SQL_GCLSearchList_RF"));
	query.execSql(SSIRes("SQL_GCLSearchList_RFIndex"));
	query.execSql(SSIRes("SQL_GCLSearchList_Balance"));
	query.execSql(SSIRes("SQL_GCLSearchList_BalanceIndex"));
	//
	query.openSql(SSIRes("SQL_GCLSearchList"));
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmGCLSearchList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("fDeposit",CashToStr(query.fieldByName("fDeposit")));
			SSIValue("fCashout",CashToStr(query.fieldByName("fCashout")));
			SSIValue("fRF",CashToStr(query.fieldByName("fRF")));
			SSIValue("fDepositClose",CashToStr(query.fieldByName("fDepositClose")));
			SSIValue("fLossesClose",CashToStr(query.fieldByName("fLossesClose")));
			s += SSIRes("AdmGCLSearchList_Item");
			query.next();
		}
	SSIValue("Stream_GCLSearchList",s);

	return ScriptPage("AdmGCLSearchList");
}

// AdmUserList

TStr TMain::ScriptAdmUserList()
{
	if( fEditID > 0 )
		return ScriptAdmUserEdit();

	SSIValue("WHERE",OnPageWhere("AdmUserList_Search"));
	SSIValue("SORT",OnPageSort("AdmUserList_Sort"));

	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_UserListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_UserList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmUserList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIBlock("&isBlock",query.fieldByName("fActive")[0] == '0');
			SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
			SSIValue("fNick",StrToHTML(__unescape(query.fieldByName("fNick"))));
			SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
			SSIValue("fCash_Fun",CashToStr(query.fieldByName("fCash_Fun")));
			SSIValue("fCash_Bonus",CashToStr(query.fieldByName("fCash_Bonus")));
			SSIValue("fCash_Free",CashToStr(query.fieldByName("fCash_Free")));
			SSIValue("fCash_FreeBonus",CashToStr(query.fieldByName("fCash_FreeBonus")));
			SSIBlock("&isComment",strlen(query.fieldByName("fComment")) > 3);
			s += SSIRes("AdmUserList_Item");
			query.next();
		}
	SSIValue("Stream_UserList",s);

	return ScriptPage("AdmUserList");
}

// ScriptAdmUserEdit

TStr TMain::ScriptAdmUserEdit()
{
	SSIValue("FEDITID",fEditID);

	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return ScriptPage("AdmUserEditID");

	m_classes::TMySqlQuery query(p_database);

/*
	if( !p_ValueList->Values("New").isEmpty() ) {
		query.execSql(SSIRes("SQL_User_Insert"));
		fEditID = query.last_id();
		p_ValueList->Add("Submit","ok");
	}
*/

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		
		TStr UserCash = 
			UserCashToXML("fCash_Real_Min","fCash_Real_Min_Dn") +
			UserCashToXML("fCash_Real_Max","fCash_Real_Max_Up") +
			UserCashToXML("fCash_Real") +
			UserCashToXML("fCash_Free") +
			UserCashToXML("fCash_FreeK") +
			UserCashToXML("fCash_Bonus") +
			UserCashToXML("fCash_Fun");
		if( !UserCash.isEmpty() )
			if( Connect() ) {
				Send("<adm command=\"update\">"+UserCash+"</adm>");
				Read();
			}
		if( p_ValueList->Values("Active").isEmpty() ) {
			if( Connect() ) {
				Send(
					"<casino command=user>"
					" <user action=block id="+TStr(fEditID)+" />"
					"</casino>"
				);
				Read();
			}
			SSIValue("Active","0");
		} else
			SSIValue("Active","1");
		SSIValue("EMAIL",__escape(p_ValueList->Values("email")));
		SSIValue("REG_GCL",p_ValueList->Values("REG_GCL"));
		SSIValue("REG_GCLID",p_ValueList->Values("REG_GCLID"));
		query.execSql(SSIRes("SQL_UserUpdate"));
		TStr ActionDate(p_ValueList->Values("ActionDate"));
		if( ActionDate.isEmpty() ) {
			ActionDate =
				p_ValueList->Values("Year") + "-" +
				p_ValueList->Values("Month") + "-" +
				p_ValueList->Values("Day");
		}
		TStr ActionComment(p_ValueList->Values("ActionComment"));
		if( !ActionComment.isEmpty() && !ActionDate.isEmpty() ) {
			SSIValue("ActionDate",ActionDate);
			SSIValue("ActionComment",ActionComment);
			query.execSql(SSIRes("SQL_UserUpdateAction"));
		}
		if( !p_ValueList->Values("NewPassword").isEmpty() ) {
			SSIValue("Password",__escape(p_ValueList->Values("NewPassword")));
			query.execSql(SSIRes("SQL_UserUpdatePassword"));
		}
	}
	TStr ActionID(p_ValueList->Values("ActionID"));
	if( !ActionID.isEmpty() ) {
		SSIValue("ActionID",ActionID);
		if( p_ValueList->Values("Action") == "cancel" )
			query.execSql(SSIRes("SQL_UserUpdateActionCancel"));
		if( p_ValueList->Values("Action") == "complete" )
			query.execSql(SSIRes("SQL_UserUpdateActionComplete"));
	}
	SSIBlock("&isSelectDay"+TStr(DateTimeToDay(Now())),true);
	SSIBlock("&isSelectMonth"+TStr(DateTimeToMonth(Now())),true);
	SSIBlock("&isSelectYear"+TStr(DateTimeToYear(Now())),true);

	query.openSql(SSIRes("SQL_UserEdit"));
	if( !query.eof() ) {
		m_classes::TMySqlQuery q(p_database);

		TStr s;
		q.openSql(SSIRes("SQL_UserAction"));
		while( !q.eof() )
		{
			QueryToSSI(this,&q);
			bool f = TStr(q.fieldByName("fDate_complete")) == "0000-00-00";
			SSIBlock("&isComplete",!f,false);
			SSIBlock("&notComplete",f,false);
			s += SSIRes("AdmUserEdit_Action_Item");
			q.next();
		}
		SSIValue("Stream_Action",s);
		SSIBlock("&isAction",!s.isEmpty());

		q.openSql(SSIRes("SQL_UserURLReg"));
		if( !q.eof() )
			QueryToSSI(this,&q);
		QueryToSSI(this,&query);

		if( strlen(query.fieldByName("freg_ip")) > 3 )
			SSIValue("RegIP",query.fieldByName("freg_ip"));
		SSIValue("Reg_GCLID",query.fieldByName("freg_gclID"));
		SSIValue("Reg_GCL",query.fieldByName("freg_gcl"));
		if( strlen(query.fieldByName("freg_referrer")) > 3 )
			SSIValue("RegReferrer",query.fieldByName("freg_referrer"));

		SSIValue("fCash_Real_Min",CashToStr(query.fieldByName("fCash_Real_Min")));
		SSIValue("fCash_Real_Max",CashToStr(query.fieldByName("fCash_Real_Max")));

		SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
		SSIValue("fCash_Fun",CashToStr(query.fieldByName("fCash_Fun")));
		SSIValue("fCash_Bonus",CashToStr(query.fieldByName("fCash_Bonus")));
		SSIValue("fCash_Free",CashToStr(query.fieldByName("fCash_Free")));
		SSIValue("fCash_FreeBets",CashToStr(query.fieldByName("fCash_FreeBets")));
		SSIValue("fEmail",StrToHTML(__unescape(query.fieldByName("fEMail"))));
		SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
		SSIValue("fNick",StrToHTML(__unescape(query.fieldByName("fNick"))));
		SSIValue("Country",__unescape(query.fieldByName("Country")));
		TXML xml(query.fieldByName("fInfo"));
		TXMLNode* p = xml.FindNode("user");
		if( p != NULL ) {
			if( p->VarValue("Bonus") == '1' )
				SSIBlock("&isBonus",true);
			SSIValue("fPhone",__unescape(p->VarValue("Phone")));

			switch( StrToInt(__unescape(p->VarValue("Game"))) ){
				case 2:
					SSIValue("fGame","Roulette");
					break;
				case 3:
					SSIValue("fGame","BlackJack");
					break;
				case 4:
					SSIValue("fGame","Poker");
					break;
				case 5:
					SSIValue("fGame","Baccarat");
					break;
				case 6:
					SSIValue("fGame","Craps");
					break;
			}

			TStr Date(p->VarValue("Date"));
			TStr Year(Date.GetBefore("|"));
//			Date = Date.CopyAfter(0);
			Year = Year.ToIntDef(1) + 1919;
			TStr Month(Date.GetBefore("|"));
//			Date = Date.CopyAfter(0);
			SSIValue("fBirthDay",Date+"."+Month+"."+Year);
		}
		q.openSql(SSIRes("SQL_Status"));
		SSIValue("stream_status",QueryToCombo(&q,query.fieldByName("fid_status")));

//		q.openSql(SSIRes("SQL_Country"));
//		SSIValue("stream_country",QueryToCombo(&q,query.fieldByName("fid_country")));

		if( query.fieldByName("fActive")[0] == '1' )
			SSIValue("Active","checked");

//		SSIValue("Stream_AdmPay",OnStreamUserPayAdm(fEditID));

		SSIValue("Stream_Game1",OnStreamUserGame(fEditID,1));
//		SSIValue("Stream_Game2",OnStreamUserGame(fEditID,2));
		{
			TStr s1(OnStreamUserGame(fEditID,1,"'"+DateToStr(Now(),"%Y-%m-%d")+"'"));
			TStr s2;
			for( int i = 0; i < 7; i++ )
			{
				if( !s2.isEmpty() )
					s2 += ",";
				s2 += "'"+DateToStr(DateTimeIncDay(Now(),-i),"%Y-%m-%d")+"'";
			}
			s2 = OnStreamUserGame(fEditID,1,s2);
			if( !s1.isEmpty() || !s2.isEmpty() ) {
				SSIValue("Stream_Game1_Now",s1);
				SSIValue("Stream_Game2_Now",s2);
				SSIBlock("&isGameNow",true);
			}
		}

		SSIValue("Stream_Order",OnStreamUserPay(fEditID,1,30));

		SSIValue("ModeID",1);
		q.openSql(SSIRes("SQL_Common_User_Session"));
		if( !q.eof() ) {
			SSIValue("Sessions_real",q.fieldByName("sessions"));
			SSIValue("TimeAvg_real",q.fieldByName("avg"));
			SSIValue("TimeTotal_real",q.fieldByName("total"));
		}
		SSIValue("ModeID",2);
		q.openSql(SSIRes("SQL_Common_User_Session"));
		if( !q.eof() ) {
			SSIValue("Sessions_fun",q.fieldByName("sessions"));
			SSIValue("TimeAvg_fun",q.fieldByName("avg"));
			SSIValue("TimeTotal_fun",q.fieldByName("total"));
		}

		SSIValue("Stream_PaySystem",OnStreamUserPaySystem(fEditID));
		SSIValue("Stream_PayStatus",OnStreamUserPayStatus(fEditID));

		SSIValue("Stream_IP",OnStreamUserIP(fEditID));
	} else
		ScriptPage("AdmUserEditID");

	return ScriptPage("AdmUserEdit");
}

//
// ScriptAdmSessionList
//
TStr TMain::ScriptAdmSessionList()
{
	if( fEditID > 0 )
		return ScriptAdmUserEdit();

	SSIValue("WHERE",OnPageWhere("AdmSessionList_Search"));
	SSIValue("SORT",OnPageSort("AdmSessionList_Sort"));

	unsigned int Count;
	m_classes::TMySqlQuery query(p_database);
	m_classes::TMySqlQuery q(p_database);
	query.openSql(SSIRes("SQL_SessionListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_SessionList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmSessionList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("UserLogin",StrToHTML(__unescape(query.fieldByName("UserLogin"))));
			SSIBlock("&notNULL",query.fieldByName("fBets")[0] != '0');
			SSIValue("fBets",CashToStr(query.fieldByName("fBets")));
			SSIValue("fTours",query.fieldByName("fTours"));
			SSIValue("fProfit",BalanceToStr(query.fieldByName("fProfit")));
			SSIValue("fPayout",CashToStr(query.fieldByName("fPayout")));
			SSIBlock("&isNegative",query.fieldByName("fprofit")[0] == '-');
			SSIValue("CashEnter",CashToStr(query.fieldByName("fCashEnter")));
			SSIValue("CashLeave",CashToStr(query.fieldByName("fCashLeave")));
			s += SSIRes("AdmSessionList_Item");
			query.next();
		}
	SSIValue("Stream_SessionList",s);

	return ScriptPage("AdmSessionList");
}

// ScriptAdmLogList

TStr TMain::ScriptAdmLogList()
{
	SSIValue("WHERE",OnPageWhere("AdmLogList_Search"));
	SSIValue("SORT",OnPageSort("AdmLogList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_LogListCount"));
	unsigned int Count = TStr(query.fields(0)).ToInt();
	if( Count > 32*fRecords )
		Count = 32*fRecords;
	OnPages(Count);
	query.openSql(SSIRes("SQL_LogList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmLogList_Empty");
	else
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("UserLogin",StrToHTML(__unescape(query.fieldByName("UserLogin"))));
			SSIValue("profit",BalanceToStr(query.fieldByName("profit")));
			SSIValue("bet",CashToStr(query.fieldByName("bet")));
			SSIValue("win",CashToStr(query.fieldByName("win")));
			SSIValue("cash",CashToStr(query.fieldByName("cash")));
			SSIValue("payout",CashToStr(query.fieldByName("payout")));
			TStr Info(query.fieldByName("info"));
			SSIValue("info",GameInfoToStr(query.fieldByName("fid_game"),Info));
			s += SSIRes("AdmLogList_Item");
			query.next();
		}
	SSIValue("Stream_LogList",s);

	return ScriptPage("AdmLogList");
}

// ScriptAdmLogView

TStr TMain::ScriptAdmLogView()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return Script("AdmLogList");

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_LogView"));
	SSIBlock("&isOdd",true);
	SSIBlock("&isEven",false);
	if( !query.eof() )
	{
		QueryToSSI(this,&query);
		TStr Info(query.fieldByName("finfo"));
		SSIValue("fInfo",StrToHTML(Info));
		SSIValue("Bet",CashToStr(query.fieldByName("Bet")));
		SSIValue("Win",CashToStr(query.fieldByName("Win")));
	}

	return ScriptPage("AdmLogView");
}

// ScriptAdmLogSessionList

TStr TMain::ScriptAdmLogSessionList()
{
	SSIValue("WHERE",OnPageWhere("AdmLogSessionList_Search"));
	SSIValue("SORT",OnPageSort("AdmLogSessionList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_LogSessionListCount"));
	unsigned int Count = TStr(query.fields(0)).ToInt();
	if( Count > 32*fRecords )
		Count = 32*fRecords;
	OnPages(Count);
	query.openSql(SSIRes("SQL_LogSessionList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s += SSIRes("AdmLogSessionList_Empty");
	else {
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			SSIValue("profit",BalanceToStr(query.fieldByName("profit")));
			SSIValue("bet",CashToStr(query.fieldByName("bet")));
			SSIValue("win",CashToStr(query.fieldByName("win")));
			SSIValue("cash",CashToStr(query.fieldByName("cash")));
			SSIValue("info",GameInfoToStr(query.fieldByName("fid_game"),query.fieldByName("info")));
			s += SSIRes("AdmLogSessionList_Item");
			query.next();
		}
	}
	SSIValue("Stream_LogSessionList",s);

	return ScriptPage("AdmLogSessionList");
}

// ScriptAdmGameList

TStr TMain::ScriptAdmGameList()
{
	SSIValue("WHERE",OnPageWhere("AdmGameList_Search"));
	SSIValue("SORT",OnPageSort("AdmGameList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_GameList"));
	TStr s;
	bool f = true;
	if( query.eof() )
		s = SSIRes("AdmGameList_Empty");
	else {
		unsigned int Bets = 0, Wins = 0, Rounds = 0, Players = 0;
		while( !query.eof() )
		{
			QueryToSSI(this,&query);
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			int Profits = TStr(query.fieldByName("profits")).ToInt();
			Bets += TStr(query.fieldByName("bets")).ToInt();
			Wins += TStr(query.fieldByName("wins")).ToInt();
			Rounds += TStr(query.fieldByName("rounds")).ToInt();
			Players += TStr(query.fieldByName("players")).ToInt();
			SSIBlock("&isNegative",Profits < 0,false);
			SSIValue("profits",BalanceToStr(Profits));
			SSIValue("payouts",CashToStr(TStr(query.fieldByName("payouts")).ToInt()));
			SSIValue("bets",CashToStr(query.fieldByName("bets")));
			SSIValue("wins",CashToStr(query.fieldByName("wins")));
			s += SSIRes("AdmGameList_Item");
			query.next();
		}
		int Profits = Bets - Wins;
		SSIBlock("&isNegative",Profits < 0,false);
		SSIValue("profits",BalanceToStr(Profits));
		SSIValue("payouts",CashToStr(int(Wins * 10000.0 / Bets)));
		SSIValue("bets",CashToStr(Bets));
		SSIValue("wins",CashToStr(Wins));
		SSIValue("rounds",Rounds);
		SSIValue("players",Players);
		s += SSIRes("AdmGameList_Total");
	}
	SSIValue("Stream_GameList",s);

	return ScriptPage("AdmGameList");
}

//
// ScriptAdmGameEdit
//
/*
TStr TMain::ScriptAdmGameEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return Script("AdmGameList");

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_GameEdit"));
	TStr sTable;
	if( !query.eof() ) {
		TStr s;
		TStr Last(query.fieldByName("TableTitle"));
		bool f = false;
		while( !query.eof() )
		{
			SSIBlock("&isOdd",f);
			SSIBlock("&isEven",!f);
			f = !f;
			if( Last != query.fieldByName("TableTitle") ) {
				Last = query.fieldByName("TableTitle");
				SSIValue("STREAM_MONEYTABLE",s);
				s.Clear();
				sTable += SSIRes("AdmMoneyEdit_Table");
			}
			QueryToSSI(this,&query);
			SSIValue("fCash",CashToStr(query.fieldByName("fCash")));
			if( strlen(query.fieldByName("GameTableTitle")) > 0 )
				SSIValue("TableTitle",query.fieldByName("GameTableTitle"));
			s += SSIRes("AdmMoneyEdit_Table_Item");
			query.next();
		}
		SSIValue("STREAM_MONEYTABLE",s);
		sTable += SSIRes("AdmMoneyEdit_Table");
	}
	SSIValue("STREAM_MONEY",sTitle + sCash + sTable);

	return ScriptPage("AdmMoneyEdit");
}
*/

// ScriptAdmMoneyList

TStr TMain::ScriptAdmMoneyList()
{
	if( fEditID > 0 )
		return ScriptAdmMoneyEdit();

	TStr Sort(OnPageHead("AdmMoneyList_head"));

	m_classes::TMySqlQuery query(p_database);
	m_classes::TMySqlQuery q(p_database);
	m_classes::TMySqlQuery qq(p_database);
	query.openSql(SSIRes("SQL_MoneyList_Games"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		q.openSql(SSIRes("SQL_MoneyList_Bet"));
		SSIValue("BET1","");
		SSIValue("BET2","");
		SSIValue("BET3","");
		SSIValue("CASH1","");
		SSIValue("CASH2","");
		SSIValue("CASH3","");
		while( !q.eof() )
		{
			SSIValue(TStr("BET")+q.fieldByName("FID_Money"),q.fieldByName("FBet"));
			SSIValue("FID_Money",q.fieldByName("FID_Money"));
			qq.openSql(SSIRes("SQL_MoneyList_Cash"));
			if( !qq.eof() )
				SSIValue(TStr("CASH")+q.fieldByName("FID_Money"),CashToStr(qq.fieldByName("FCash")));
			q.next();
		}
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmMoneyList_Item");
		query.next();
	}
	SSIValue("Stream_MoneyList",s);

	return ScriptPage("AdmMoneyList");
}

// ScriptAdmMoneyEdit

TStr TMain::ScriptAdmMoneyEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return Script("AdmMoneyList");

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		TStr s;
		for( unsigned int i = 0; i < 4; i++ )
		{
			TStr Bet(p_ValueList->Values("fBet"+TStr(i)));
			if( !Bet.isEmpty() ) {
				SSIValue("GameID",fEditID);
				SSIValue("ModeID",fModeID);
				SSIValue("MoneyID",i);
				SSIValue("Bet",Bet);
				s += SSIRes("AdmMoneyEdit_XMLMoneyBet");
			}
			TStr Cash(p_ValueList->Values("fCash"+TStr(i)));
			if( !Cash.isEmpty() ) {
				SSIValue("GameID",fEditID);
				SSIValue("ModeID",fModeID);
				SSIValue("MoneyID",i);
				int iCash = StrToCash(Cash);
				SSIValue("Cash",iCash);
				s += SSIRes("AdmMoneyEdit_XMLMoneyCash");
			}
			for( unsigned int j = 0; j < 140; j++ )
			{
				TStr Bet(p_ValueList->Values("fBet"+TStr(i)+"Table"+TStr(j)));
				if( !Bet.isEmpty() ) {
					SSIValue("GameID",fEditID);
					SSIValue("ModeID",fModeID);
					SSIValue("MoneyID",i);
					SSIValue("TableID",j);
					SSIValue("Bet",Bet);
					s += SSIRes("AdmMoneyEdit_XMLMoneyTableBet");
				}
				TStr Cash(p_ValueList->Values("fCash"+TStr(i)+"Table"+TStr(j)));
				if( !Cash.isEmpty() ) {
					SSIValue("GameID",fEditID);
					SSIValue("ModeID",fModeID);
					SSIValue("MoneyID",i);
					SSIValue("TableID",j);
					int iCash = StrToCash(Cash);
					SSIValue("Cash",iCash);
					s += SSIRes("AdmMoneyEdit_XMLMoneyTableCash");
				}
			}
		}
		if( !s.isEmpty() ) {
			SSIValue("STREAM",s);
			s = SSIRes("AdmMoneyEdit_XMLMoney");
			Connect();
			Send(s);
			s = Read();
		}
	}
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_MoneyEdit"));
	TStr sBet, sCash, sTable, sTableBet;
	SSIBlock("&isOdd",true);
	SSIBlock("&isEven",false);
	while( !query.eof() )
	{
		if( query.fieldByName("fid_money")[0] == '2' ) {
			m_classes::TMySqlQuery q(p_database);
			SSIValue("MoneyID",2);
			q.openSql(SSIRes("SQL_MoneyEdit_Table"));
			TStr lRow;
			TStr sRow, sRowBet;
			while( !q.eof() )
			{
				QueryToSSI(this,&q);
				SSIValue("Cash",CashToStr(q.fieldByName("fCash")));
				SSIValue("Title",q.fieldByName("fTitle"));
				if( q.fieldByName("fTitleRow")[0] != '\0' )
					SSIValue("Title",q.fieldByName("fTitleRow"));
				sRow += SSIRes("AdmMoneyEdit_Table_Item");
				sRowBet += SSIRes("AdmMoneyEdit_Table_Bet");
				q.next();
				if( q.eof() || lRow != q.fieldByName("frow") ) {
					if( !lRow.isEmpty() ) {
						SSIValue("Stream",sRow);
						sTable += SSIRes("AdmMoneyEdit_Table");
						SSIValue("Stream",sRowBet);
						sTableBet += SSIRes("AdmMoneyEdit_Table");
						sRow.Clear();
						sRowBet.Clear();
					}
					if( !q.eof() ) {
						lRow = q.fieldByName("frow");
					}
				}
			}
		}
		QueryToSSI(this,&query);
		SSIValue("fCash",CashToStr(query.fieldByName("fCash")));
		sBet += SSIRes("AdmMoneyEdit_Bet");
		sCash += SSIRes("AdmMoneyEdit_Cash");
		query.next();
	}
	SSIValue("STREAM_BET",sBet);
	SSIValue("STREAM_CASH",sCash);
	SSIValue("STREAM_TABLE",sTable);
	SSIValue("STREAM_TABLEBET",sTableBet);

	return ScriptPage("AdmMoneyEdit");
}

// ScriptAdmPayLogList

TStr TMain::ScriptAdmPayLogList()
{
	return ScriptPage("AdmPayLogList");
}

// ScriptAdmSetupList

TStr TMain::ScriptAdmSetupList()
{
	if( fEditID > 0 )
		return ScriptAdmSetupEdit();

	SSIValue("SORT",OnPageSort("AdmSetupList_Sort"));

	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_SetupList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmSetupList_Item");
		query.next();
	}
	SSIValue("Stream_SetupList",s);
	
	return ScriptPage("AdmSetupList");
}

// ScriptAdmSetupEdit

TStr TMain::ScriptAdmSetupEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() )
		fEditID = 0;
	if( fEditID == 0 )
		return ScriptAdmSetupList();

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		query.execSql(SSIRes("SQL_SetupUpdate"));
	}

	query.openSql(SSIRes("SQL_SetupView"));
	if( !query.eof() )
		QueryToSSI(this,&query);

	return ScriptPage("AdmSetupEdit");
}

// ScriptAdmPopupList

TStr TMain::ScriptAdmPopupList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql("DELETE FROM popup WHERE ID="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmPopupEdit();

	if( fEditID > 0 )
		return ScriptAdmPopupEdit();

	OnPageSort("AdmPopupList_Sort");

	query.openSql(SSIRes("SQL_PopupList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmPopupList_Item");
		query.next();
	}
	SSIValue("Stream_PopupList",s);

	return ScriptPage("AdmPopupList");
}

// ScriptAdmPopupEdit

TStr TMain::ScriptAdmPopupEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmPopupList();
	}

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		TStr DateStart(p_ValueList->Values("fDate_Start"));
		if( DateStart.isEmpty() ) {
			DateStart =
				p_ValueList->Values("StartYear") + "-" +
				p_ValueList->Values("StartMonth") + "-" +
				p_ValueList->Values("StartDay");
		}
		TStr DateFinish(p_ValueList->Values("fDate_Finish"));
		if( DateFinish.isEmpty() ) {
			DateFinish =
				p_ValueList->Values("FinishYear") + "-" +
				p_ValueList->Values("FinishMonth") + "-" +
				p_ValueList->Values("FinishDay");
		}
		ValueListToSSISQL(this,p_ValueList);
		SSIValue("fURL",StrToSQL(p_ValueList->Values("fURL").Replace("\r\n","\n")));
		SSIValue("fWidth",StrToSQL(p_ValueList->Values("fWidth").Replace("\r\n","\n")));
		SSIValue("fHeight",StrToSQL(p_ValueList->Values("fHeight").Replace("\r\n","\n")));
		SSIValue("fDate_start",DateStart);
		SSIValue("fDate_finish",DateFinish);
		if( fEditID == 0 ) {
			query.execSql(SSIRes("SQL_PopupInsert"));
			fEditID = query.last_id();
			SSIValue("FEDITID",fEditID);
		} else
			query.execSql(SSIRes("SQL_PopupUpdate"));
	}

	TStr DateStart, DateFinish;

	m_classes::TMySqlQuery q(p_database);
	query.openSql(SSIRes("SQL_PopupView"));
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		DateStart = query.fieldByName("fDate_Start");
		DateFinish = query.fieldByName("fDate_Finish");
	} 

	SSIValue("Stream_DateStart",DateToCombo(this,"Start",DateStart));
	SSIValue("Stream_DateFinish",DateToCombo(this,"Finish",DateFinish));

	return ScriptPage("AdmPopupEdit");
}


// ScriptAdmNewsList

TStr TMain::ScriptAdmNewsList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql("DELETE FROM news WHERE ID="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmNewsEdit();

	if( fEditID > 0 )
		return ScriptAdmNewsEdit();

	OnPageSort("AdmNewsList_Sort");

	query.openSql(SSIRes("SQL_NewsList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIValue("fText",__unescape(query.fieldByName("fText")));
		SSIValue("fTitle",__unescape(query.fieldByName("fTitle")));
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmNewsList_Item");
		query.next();
	}
	SSIValue("Stream_NewsList",s);

	return ScriptPage("AdmNewsList");
}

// ScriptAdmNewsEdit

TStr TMain::ScriptAdmNewsEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmNewsList();
	}

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		SSIValue("fText",StrToSQL(__escape(p_ValueList->Values("fText"))));
		SSIValue("fTitle",StrToSQL(__escape(p_ValueList->Values("fTitle"))));
		if( fEditID == 0 ) {
			query.execSql(SSIRes("SQL_NewsInsert"));
			fEditID = query.last_id();
			SSIValue("FEDITID",fEditID);
		} else
			query.execSql(SSIRes("SQL_NewsUpdate"));
	}

	m_classes::TMySqlQuery q(p_database);
	query.openSql(SSIRes("SQL_NewsView"));
	TStr LangID;
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		LangID = query.fieldByName("fid_lang");
		SSIValue("fText",__unescape(query.fieldByName("fText")));
		SSIValue("fTitle",__unescape(query.fieldByName("fTitle")));
	}
	q.openSql(SSIRes("SQL_Common_Lang"));
	SSIValue("stream_lang",QueryToCombo(&q,LangID));

	return ScriptPage("AdmNewsEdit");
}


// ScriptAdmMoneyLimitList

TStr TMain::ScriptAdmMoneyLimitList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql("DELETE FROM money_gamestablelimit WHERE ID="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmMoneyLimitEdit();
	if( fEditID > 0 )
		return ScriptAdmMoneyLimitEdit();

	query.openSql(SSIRes("SQL_MoneyLimitList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		SSIValue("fMax",CashToStr(query.fieldByName("fMax")));
		s += SSIRes("AdmMoneyLimitList_Item");
		query.next();
	}
	SSIValue("Stream_MoneyLimitList",s);
	
	return ScriptPage("AdmMoneyLimitList");
}

// ScriptAdmMoneyLimitEdit

TStr TMain::ScriptAdmMoneyLimitEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmMoneyLimitList();
	}

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		SSIValue("max",StrToCash(p_ValueList->Values("max")));
		if( fEditID == 0 ) {
			query.execSql(SSIRes("SQL_MoneyLimitInsert"));
			fEditID = query.last_id();
			SSIValue("FEDITID",fEditID);
		} else
			query.execSql(SSIRes("SQL_MoneyLimitUpdate"));
	}

	query.openSql(SSIRes("SQL_MoneyLimitView"));
	SSIBlock("&isEdit",!query.eof());
	SSIBlock("&isNew",query.eof());
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("fMax",CashToStr(query.fieldByName("fMax")));
	} else {
		query.openSql(SSIRes("SQL_Games"));
		SSIValue("Stream_Game",QueryToCombo(&query,TStr("")));
	}

	return ScriptPage("AdmMoneyLimitEdit");
}

// ScriptAdmPromoUsersList

TStr TMain::ScriptAdmPromoUsersList()
{
	SSIValue("PromoID",p_ValueList->Values("PromoID"));
	m_classes::TMySqlQuery query(p_database);
	query.openSql(SSIRes("SQL_PromoUsersList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
		SSIValue("fCash_Real",CashToStr(query.fieldByName("fCash_Real")));
		SSIValue("fCash",CashToStr(query.fieldByName("fCash")));
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmPromoUsersList_Item");
		query.next();
	}
	SSIValue("Stream_PromoUsersList",s);

	return ScriptPage("AdmPromoUsersList");
}

// ScriptAdmPromoList

TStr TMain::ScriptAdmPromoList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql("DELETE FROM promo WHERE ID="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmPromoEdit();

	if( fEditID > 0 )
		return ScriptAdmPromoEdit();

//	OnPageSort("AdmPromoList_Sort");

	SSIValue("WHERE",OnPageWhere("AdmPromoList_Search"));
	unsigned int Count;
	query.openSql(SSIRes("SQL_PromoListCount"));
	OnPages(TStr(query.fields(0)).ToInt());
	query.openSql(SSIRes("SQL_PromoList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIValue("UsersCash",CashToStr(query.fieldByName("UsersCash")));
		SSIValue("fDate_Start",query.fieldByName("fDate_Start"));
		SSIValue("fDate_End",query.fieldByName("fDate_end"));
		SSIValue("fSer",query.fieldByName("fSer"));
		SSIValue("fActive",query.fieldByName("fActive"));
		SSIValue("fInfo",StrToHTML(query.fieldByName("fInfo")));
		SSIBlock("&isActive",query.fieldByName("fActive")[0] != '0',false);
		SSIBlock("&isActiveRecord",query.fieldByName("Active1")[0] == '1' && query.fieldByName("Active2")[0] == '1');
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmPromoList_Item");
		query.next();
	}
	SSIValue("Stream_PromoList",s);

	return ScriptPage("AdmPromoList");
}

// ScriptAdmPromoEdit

TStr TMain::ScriptAdmPromoEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmPromoList();
	}

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		TStr DateStart(p_ValueList->Values("fDate_Start"));
		if( DateStart.isEmpty() ) {
			DateStart =
				p_ValueList->Values("StartYear") + "-" +
				p_ValueList->Values("StartMonth") + "-" +
				p_ValueList->Values("StartDay");
		}
		TStr DateEnd(p_ValueList->Values("fDate_End"));
		if( DateEnd.isEmpty() ) {
			DateEnd =
				p_ValueList->Values("EndYear") + "-" +
				p_ValueList->Values("EndMonth") + "-" +
				p_ValueList->Values("EndDay");
		}
		TStr Active(2);
		TStr Ser(p_ValueList->Values("fSer"));
		TStr Info(p_ValueList->Values("fInfo"));
		if( Info.isEmpty() ) {
			Info = "<promo ";
			Info += " num="+p_ValueList->Values("Num");
			if( p_ValueList->Values("fType") == "deposit" ) {
				Info += " type=deposit";
				Info += " deposit_sum="+TStr(StrToCash(p_ValueList->Values("deposit_sum")));
				Info += " deposit_min="+TStr(StrToCash(p_ValueList->Values("deposit_min")));
				Info += " deposit_bets="+TStr(StrToCash(p_ValueList->Values("deposit_bets")));
			} else
			if( p_ValueList->Values("fType") == "procent" ) {
				Info += " type=procent";
				Info += " procent_sum="+p_ValueList->Values("procent_sum");
				Info += " procent_min="+TStr(StrToCash(p_ValueList->Values("procent_min")));
				Info += " procent_max="+TStr(StrToCash(p_ValueList->Values("procent_max")));
				Info += " procent_bets="+p_ValueList->Values("procent_bets");
			} else
			if( p_ValueList->Values("fType") == "amount" ) {
				Info += " type=amount";
				Info += " amount_sum="+TStr(StrToCash(p_ValueList->Values("Amount_Sum")));
				Info += " amount_bets="+TStr(StrToCash(p_ValueList->Values("Amount_Bets")));
			}
			Info += " />";
		}
//		if( !Info.isEmpty() ) {
			SSIValue("fDate_start",DateStart);
			SSIValue("fDate_end",DateEnd);
			SSIValue("fSer",Ser);
			SSIValue("fActive",Active);
			SSIValue("fInfo",Info);
			SSIValue("fComment",p_ValueList->Values("fComment"));
			if( fEditID == 0 ) {
				query.execSql(SSIRes("SQL_PromoInsert"));
				fEditID = query.last_id();
				SSIValue("FEDITID",fEditID);
			} else
				query.execSql(SSIRes("SQL_PromoUpdate"));
//		}
	}

	m_classes::TMySqlQuery q(p_database);
	query.openSql(SSIRes("SQL_PromoView"));
	TStr LangID;

	TStr DateStart, DateEnd;
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		TXML xml(query.fieldByName("fInfo"));
		SSIValue("num",xml.VarValue("PROMO/NUM"));
		if( xml.VarValue("PROMO/TYPE") == "deposit" ) {
			SSIValue("deposit_sum",CashToStr(xml.VarValue("PROMO/DEPOSIT_SUM")));
			SSIValue("deposit_min",CashToStr(xml.VarValue("PROMO/DEPOSIT_MIN")));
			SSIValue("deposit_bets",CashToStr(xml.VarValue("PROMO/DEPOSIT_BETS")));
		} else
		if( xml.VarValue("PROMO/TYPE") == "procent" ) {
			SSIValue("procent_sum",xml.VarValue("PROMO/PROCENT_SUM"));
			SSIValue("procent_min",CashToStr(xml.VarValue("PROMO/PROCENT_MIN")));
			SSIValue("procent_max",CashToStr(xml.VarValue("PROMO/PROCENT_MAX")));
			SSIValue("procent_bets",xml.VarValue("PROMO/PROCENT_BETS"));
		} else
		if( xml.VarValue("PROMO/TYPE") == "amount" ) {
			SSIValue("amount_sum",CashToStr(xml.VarValue("PROMO/AMOUNT_SUM")));
			SSIValue("amount_bets",CashToStr(xml.VarValue("PROMO/AMOUNT_BETS")));
		}
		SSIBlock("&isProcent",xml.VarValue("PROMO/TYPE") == "procent");
		SSIBlock("&isDeposit",xml.VarValue("PROMO/TYPE") == "deposit");
		SSIBlock("&isCash",xml.VarValue("PROMO/TYPE") == "amount");
		DateStart = query.fieldByName("fDate_Start");
		DateEnd = query.fieldByName("fDate_End");
	}
	SSIValue("Stream_DateStart",DateToCombo(this,"Start",DateStart));
	SSIValue("Stream_DateEnd",DateToCombo(this,"End",DateEnd));

	return ScriptPage("AdmPromoEdit");
}


// ScriptAdmClubCardList

TStr TMain::ScriptAdmClubCardList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql(
			"DELETE FROM tbl_users_club "
			"WHERE FID_User="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmClubCardEdit();

	if( fEditID > 0 )
		return ScriptAdmClubCardEdit();

	query.openSql(SSIRes("SQL_ClubCardList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		SSIValue("fLogin",StrToHTML(__unescape(query.fieldByName("fLogin"))));
		SSIBlock("&isStatus1",TStr(query.fieldByName("fid_status")) == "0" );
		SSIBlock("&isStatus2",TStr(query.fieldByName("fid_status")) == "1" );
		SSIBlock("&isStatus3",TStr(query.fieldByName("fid_status")) == "2" );
		f = !f;
		s += SSIRes("AdmClubCardList_Item");
		query.next();
	}
	SSIValue("Stream_ClubCardList",s);

	return ScriptPage("AdmClubCardList");
}

// ScriptAdmClubCardEdit

TStr TMain::ScriptAdmClubCardEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmClubCardList();
	}

	m_classes::TMySqlQuery query(p_database);

	SSIValue("fDate",DateTimeToStr(Now(),"%Y-%m-%d"));
	if( !p_ValueList->Values("Submit").isEmpty() ) {
		SSIValue("fNum",p_ValueList->Values("fNum"));
		SSIValue("fComment",StrToSQL(p_ValueList->Values("fComment")));
		SSIValue("fid_status",p_ValueList->Values("fid_status"));
		if( fEditID == 0 ) {
			fEditID = p_ValueList->Values("fid_user").ToIntDef(0);
			if( fEditID != 0 ) {
				SSIValue("FEDITID",fEditID);
				query.execSql(SSIRes("SQL_ClubCardInsert"));
			}
		} else
			query.execSql(SSIRes("SQL_ClubCardUpdate"));
	}

	m_classes::TMySqlQuery q(p_database);
	query.openSql(SSIRes("SQL_ClubCardView"));
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIBlock("&isEdit",true);
		SSIBlock("&isNew",false);
		SSIBlock("&isStatus1",TStr(query.fieldByName("fid_status")) == "0" );
		SSIBlock("&isStatus2",TStr(query.fieldByName("fid_status")) == "1" );
		SSIBlock("&isStatus3",TStr(query.fieldByName("fid_status")) == "2" );
	} else {
		SSIBlock("&isEdit",false);
		SSIBlock("&isNew",true);
	}

	return ScriptPage("AdmClubCardEdit");
}


// ScriptAdmSubscribeList

TStr TMain::ScriptAdmSubscribeList()
{
	m_classes::TMySqlQuery query(p_database);
	if( !p_ValueList->Values("del").isEmpty() )
		query.execSql(
			"DELETE FROM Subscribe "
			"WHERE ID="+p_ValueList->Values("del"));
	if( p_ValueList->Values("action") == "add" )
		return ScriptAdmSubscribeEdit();

	if( fEditID > 0 )
		return ScriptAdmSubscribeEdit();

	OnPageSort("AdmSubscribeList_Sort");

	query.openSql(SSIRes("SQL_SubscribeList"));
	TStr s;
	bool f = true;
	while( !query.eof() )
	{
		QueryToSSI(this,&query);
		SSIValue("fTitle",__unescape(query.fieldByName("fTitle")));
		SSIBlock("&isOdd",f);
		SSIBlock("&isEven",!f);
		f = !f;
		s += SSIRes("AdmSubscribeList_Item");
		query.next();
	}
	SSIValue("Stream_SubscribeList",s);
	
	return ScriptPage("AdmSubscribeList");
}

// ScriptAdmSubscribeEdit

TStr TMain::ScriptAdmSubscribeEdit()
{
	if( !p_ValueList->Values("Close").isEmpty() ) {
		fEditID = 0;
		return ScriptAdmSubscribeList();
	}
	SSIBlock("&isEdit",fEditID != 0);
	SSIBlock("&isNew",fEditID == 0);

	m_classes::TMySqlQuery query(p_database);

	if( !p_ValueList->Values("Submit").isEmpty() ) {
		ValueListToSSISQL(this,p_ValueList);
		SSIValue("fText",__escape(p_ValueList->Values("fText")));
		SSIValue("fTitle",__escape(p_ValueList->Values("fTitle")));
		if( fEditID == 0 ) {
			query.execSql(SSIRes("SQL_SubscribeInsert"));
			fEditID = query.last_id();
			SSIValue("FEDITID",fEditID);
		}
	}

	query.openSql(SSIRes("SQL_SubscribeView"));
	TStr LangID;
	if( !query.eof() )
		LangID = query.fieldByName("fid_lang");
	m_classes::TMySqlQuery q(p_database);
	q.openSql(SSIRes("SQL_Common_lang"));
	SSIValue("Stream_Lang",QueryToCombo(&q,LangID));
	if( !query.eof() ) {
		QueryToSSI(this,&query);
		SSIValue("fText",__unescape(query.fieldByName("fText")));
		SSIValue("fTitle",__unescape(query.fieldByName("fTitle")));
	}

	return ScriptPage("AdmSubscribeEdit");
}


// --

void TMain::Script(TStr *Result, TStr *Header, m_ssi::TStringList *ValueList)
{

	TApp::Script(Result,Header,ValueList);

	OnPageBegin();

	*Result = Script(FormName);

	OnPageEnd();
}

TStr TMain::Script(const TStr& Name) {

{


/* ************************************************************************** */
/*
	m_classes::TMySqlQuery q(p_database);
	TStr UserID("17407");
	{
		TFile f;
		char mas[20000];
		f.OpenLoad("money.txt");
		int size = fread(mas,1,20000,f.stream);
		int sj = 0;
		int sjCount = 0;
		TStr s[10];
		for( int i = 0; i < size; i++ )
		{
			if( mas[i] == '\n' ) {
				sj++;
				if( s[sj-1].isEmpty() )
					sj--;
				if( sjCount == 0 )
					sjCount = sj;
				q.execSql(
					TStr("insert into log_pay_1 (")+
					"fdate,ftime,fid_user,fid_system,fcash,fbalance) values ("
					"'"+s[0]+"','"+s[0]+"','"+UserID+"','"+s[1]+"','"+s[2]+"','"+s[3]+"')");
				for( sj = 0; sj < sjCount; sj++ ) {
					printf("%s ",s[sj].c_str());
					s[sj].Clear();
				};
				printf("\n");
				sj = 0;
			} else
			if( mas[i] == ';' )
				sj++;
			else
				s[sj] += mas[i];
		}
	}
	{
		TFile f;
		char mas[20000];
		TStr GameID = "20";
		f.OpenLoad("bj.txt");
		int size = fread(mas,1,20000,f.stream);
		int sj = 0;
		int sjCount = 0;
		TStr s[10];
		for( int i = 0; i < size; i++ )
		{
			if( mas[i] == '\n' ) {
				sj++;
				if( s[sj-1].isEmpty() )
					sj--;
				if( sjCount == 0 )
					sjCount = sj;
				q.execSql(
					TStr("insert into log_history_1 (")+
					"fdate,ftime,fid_game,fcash_bet,fcash_win,fcash,fid_user) values ("
					"'"+s[0]+"','"+s[0]+"','"+GameID+"','"+s[1]+"','"+s[2]+"','"+s[3]+"','"+s[4]+"','"+UserID+"')");
				for( sj = 0; sj < sjCount; sj++ ) {
					printf("%s ",s[sj].c_str());
					s[sj].Clear();
				};
				printf("\n");
				sj = 0;
			} else
			if( mas[i] == ';' )
				sj++;
			else
				s[sj] += mas[i];
		}
	}
	{
		TFile f;
		char mas[20000];
		f.OpenLoad("oasis.txt");
		int size = fread(mas,1,20000,f.stream);
		int sj = 0;
		int sjCount = 0;
		TStr s[10];
		TStr GameID = "30";
		for( int i = 0; i < size; i++ )
		{
			if( mas[i] == '\n' ) {
				sj++;
				if( s[sj-1].isEmpty() )
					sj--;
				if( sjCount == 0 )
					sjCount = sj;
				q.execSql(
					TStr("insert into log_history_1 (")+
					"fdate,ftime,fid_game,fcash_bet,fcash_win,fcash,fid_user) values ("
					"'"+s[0]+"','"+s[0]+"','"+GameID+"','"+s[1]+"','"+s[2]+"','"+s[3]+"','"+s[4]+"','"+UserID+"')");
				for( sj = 0; sj < sjCount; sj++ ) {
					printf("%s ",s[sj].c_str());
					s[sj].Clear();
				};
				printf("\n");
				sj = 0;
			} else
			if( mas[i] == ';' )
				sj++;
			else
				s[sj] += mas[i];
		}
	}
*/
/* ************************************************************************** */
}

#ifdef TARGET_WIN32
//	ScriptAdmPopupList();
//	ScriptAdmOnlineList();
//	ScriptAdmCasinoList();
//	OnStreamUserGamePay(48435);
//	FormName = "AdmGCLSearchList";
//	ScriptAdmOnlineList();
//	fEditID = 2577;
//	ScriptAdmTREdit();
//	p_ValueList->Add("Submit","ok");
//	ScriptAdmPromoEdit();
#endif

	TStr IP(p_ValueList->Values("REMOTE_ADDR").Trim());

#ifndef CASINO_TWIST
#ifndef TARGET_WIN32
	if(
		!IP.isEmpty() &&
		IP != "127.0.0.1" &&
		IP != "195.91.154.16" &&
		IP != "212.15.115.195" &&
		IP != "212.15.115.194" &&
		IP != "172.17.9.74" &&
		IP.CopyBefore(8) != "192.168." &&
		IP.CopyBefore(8) != "195.190." &&
		IP.CopyBefore(8) != "195.239." &&
		IP.CopyBefore(7) != "212.46." &&
		IP.CopyBefore(10) != "212.11.129"
	)
		return p_ValueList->Values("REMOTE_ADDR") + " : Access fail";
#endif
#endif

	if( FormName == "AdmLogout" ) {
		SetSessionValue("fAdm",0);
	}

	SSIBlock("&isCommon",true);
	SSIBlock("&isAdditional",false);
	SSIBlock("&isSelected",false);
	SSIBlock("&isDisabled",false);

	if( !Name.isEmpty() )
		FormName = Name;

	SSIValue("FormName",FormName);

	SSIValue("GlobalTime",DateTimeToStr(Now(),"%d.%m.%Y  %H:%M"));
	SSIValue("TodayDate",DateTimeToStr(Now(),"%Y-%m-%d"),false);

	if( FormName == "AdmLogin" && !p_ValueList->Values("fSubmit").isEmpty() ) {
		m_classes::TMySqlQuery query(p_database);
		TStr Login(p_ValueList->Values("flogin"));
		TStr Password(p_ValueList->Values("fpassword"));
		query.openSql(
			"SELECT ID,fCommand "
			"FROM password "
			"WHERE "
			"  fLogin='"+StrToSQL(Login)+"' and "
			"  fPassword='"+StrToSQL(Password)+"'");
		if( !query.eof() ) {
			SetSessionValue("fAdm",query.fieldByName("id"));
			SetSessionValue("fAdmCommand",query.fieldByName("fcommand"));
		} else
			SetSessionValue("fAdm","0");
	}
//#ifndef TARGET_WIN32
	if( !IP.isEmpty() && GetSessionValue("fAdm").ToIntDef(0) == 0 )
		return SSIRes("AdmLogin");
//#endif
	TStr Command(GetSessionValue("fAdmCommand"));
	if( !Command.isEmpty() ) {
		if( FormName == "AdmLogin" ) {
			FormName = Command.CopyBefore(";");
			p_ValueList->SetValue("DateBegin",fDateBegin = DateTimeToStr(Now(),"%Y-%m-%d"));
		}
		if( !FormName.isEmpty() && Command.Pos(FormName+";") == -1 )
			return SSIRes("AdmLogin");
	}
	if( FormName == "AdmLogin" )
		FormName = "AdmCasinoList";

	if( FormName == "AdmEMail" ) {
		TStr s;
		TStr Lang(p_ValueList->Values("Lang"));
		if( Lang == "ru" )
			Lang = " and fid_lang='1'";
		else
		if( Lang == "en" )
			Lang = " and fid_lang='2'";
		else
			Lang = "";
		TStr Day(p_ValueList->Values("Day"));
		if( !Day.isEmpty() )
			Day = " and fdate_reg='"+Day+"'";
		m_classes::TMySqlQuery query(p_database);
		query.openSql("SELECT distinct femail FROM tbl_users WHERE 1=1 " + Lang + Day);
		while( !query.eof() )
		{
			s += __unescape(query.fields(0)) + "<br>";
			query.next();
		}
		return s;
	}
	if( FormName == "AdmGCLList" ) return ScriptAdmGCLList();
	if( FormName == "AdmCasinoList" ) return ScriptAdmCasinoList();
	if( FormName == "AdmDateList" ) return ScriptAdmDateList();
	if( FormName == "AdmPeriodList" ) return ScriptAdmPeriodList();

	if( FormName == "AdmUrlList" ) return ScriptAdmUrlList();
	if( FormName == "AdmIDList" ) return ScriptAdmIDList();

	if( FormName == "AdmTRList" ) return ScriptAdmTRList();
	if( FormName == "AdmTREdit" ) return ScriptAdmTREdit();
	if( FormName == "AdmTRCreate" ) return ScriptAdmTRCreate();
	if( FormName == "AdmTRCardCreate" ) return ScriptAdmTRCardCreate();

	if( FormName == "AdmNewsList" ) return ScriptAdmNewsList();
	if( FormName == "AdmNewsEdit" ) return ScriptAdmNewsEdit();

	if( FormName == "AdmMethodList" ) return ScriptAdmMethodList();
	if( FormName == "AdmMethodEdit" ) return ScriptAdmMethodEdit();

	if( FormName == "AdmPayList" ) return ScriptAdmPayList();

	if( FormName == "AdmOnlineList" ) return ScriptAdmOnlineList();

	if( FormName == "AdmLogSessionList" ) return ScriptAdmLogSessionList();
	if( FormName == "AdmSessionList" ) return ScriptAdmSessionList();

	if( FormName == "AdmPayLogList" ) return ScriptAdmPayLogList();
	if( FormName == "AdmPayList" ) return ScriptAdmPayList();

	if( FormName == "AdmLogList" ) return ScriptAdmLogList();
	if( FormName == "AdmLogView" ) return ScriptAdmLogView();
	if( FormName == "AdmGameList" ) return ScriptAdmGameList();

	if( FormName == "AdmNewsList" ) return ScriptAdmNewsList();

	if( FormName == "AdmMoneyList" ) return ScriptAdmMoneyList();
	if( FormName == "AdmMoneyEdit" ) return ScriptAdmMoneyEdit();
	if( FormName == "AdmMoneyLimitList" ) return ScriptAdmMoneyLimitList();
	if( FormName == "AdmMoneyLimitEdit" ) return ScriptAdmMoneyLimitEdit();

	if( FormName == "AdmUserSearchList" ) return ScriptAdmUserSearchList();
	if( FormName == "AdmGCLSearchList" ) return ScriptAdmGCLSearchList();

	if( FormName == "AdmPlayerList" ) return ScriptAdmPlayerList();
	if( FormName == "AdmUserList" ) return ScriptAdmUserList();
	if( FormName == "AdmUserActionList" ) return ScriptAdmUserActionList();
	if( FormName == "AdmUserActionEdit" ) return ScriptAdmUserActionEdit();
	if( FormName == "AdmUserEdit" ) return ScriptAdmUserEdit();

	if( FormName == "AdmSetupList" ) return ScriptAdmSetupList();
	if( FormName == "AdmSetupEdit" ) return ScriptAdmSetupEdit();

	if( FormName == "AdmPopupList" ) return ScriptAdmPopupList();
	if( FormName == "AdmPopupEdit" ) return ScriptAdmPopupEdit();

	if( FormName == "AdmPromoList" ) return ScriptAdmPromoList();
	if( FormName == "AdmPromoEdit" ) return ScriptAdmPromoEdit();
	if( FormName == "AdmPromoUsersList" ) return ScriptAdmPromoUsersList();

	if( FormName == "AdmClubCardList" ) return ScriptAdmClubCardList();
	if( FormName == "AdmClubCardEdit" ) return ScriptAdmClubCardEdit();

	if( FormName == "AdmUserPaymentList" ) return ScriptAdmUserPaymentList();

	if( FormName == "AdmSubscribeList" ) return ScriptAdmSubscribeList();
	if( FormName == "AdmSubscribeEdit" ) return ScriptAdmSubscribeEdit();

	if( FormName == "server" ) return ScriptServer();

	return ScriptView();
}



