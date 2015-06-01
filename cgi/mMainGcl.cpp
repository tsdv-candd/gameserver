#include "sql/mMySQL.h"
#include "cgi/mCgi.h"
#include "xml/m_xmlparser.h"
#include "mail/m_mail.h"
#include "ssi/mf_ssi.h"
#include "convert/mConvert.h"
#include "m_string.h"
#include <stdio.h>

class TCM {
private:
	m_classes::TMySqlConnect* db;
	TCGI* cgi;
	//
	void OnBegin() {
	}
	void OnStart() {
		db->connect();
	}
	void OnStop() {
	}
	void OnEnd() {
		if( db->isActive() )
			db->disconnect();
	}
public:
	TCM(m_classes::TMySqlConnect* db, TCGI* cgi): db(db), cgi(cgi) {}

	void RunDate(const bool fDeposit, TStr& s)
	{
		TStr Day(cgi->getParamValueByName("Day"));
		TStr DateBegin(cgi->getParamValueByName("DateBegin"));
		TStr DateEnd(cgi->getParamValueByName("DateEnd"));
		if( Day.isEmpty() && (DateBegin.isEmpty() || DateEnd.isEmpty()) ) {
			s = "date is empty";
			return;
		}

		OnStart();

		m_classes::TMySqlQuery query(db);
		m_ssi::TSSIControl ssi;
		ssi.SSILoad("gcl.ini");
		ssi.SSIValue("DateBegin",DateBegin,false);
		ssi.SSIBlock("&isDateBegin",!DateBegin.isEmpty(),false);
		ssi.SSIValue("DateEnd",DateEnd,false);
		ssi.SSIBlock("&isDateEnd",!DateEnd.isEmpty(),false);
		ssi.SSIValue("Day",Day,false);
		ssi.SSIBlock("&isDay",!Day.isEmpty(),false);

		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Index"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Users"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_UsersIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_UsersIndex2"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Signups"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_SignupsIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Deposit"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_DepositIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Losses"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_LossesIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Cashout"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_CashoutIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Balance"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Balanceindex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Url"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Urlindex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Game_real"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Game_realIndex"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Game_fun"));
		query.execSql(ssi.SSIRes("SQL_Gcl_Tmp_Game_funIndex"));

		query.openSql(ssi.SSIRes("SQL_Gcl"));
		while( !query.eof() )
		{
			if( fDeposit )
				RunDateDeposit(query,s);
			else
				RunDatePeriod(query,s);
			query.next();
		}
		OnStop();
	}

	void RunDateDeposit(m_classes::TMySqlQuery& query, TStr& s)
	{
		s +=
			TStr(query.fieldByName("fTitle"))+";"+
			query.fieldByName("Signups")+";"+
			query.fieldByName("DepositCount")+";"+
			query.fieldByName("DepositSum")+";"+
			TStr(query.fieldByNameAsInt("CashoutCount")+query.fieldByNameAsInt("LossesCount"))+";"+
			TStr(query.fieldByNameAsInt("CashoutSum")+query.fieldByNameAsInt("LossesSum"))+";"
			;
		s += "\n";
	}

	void RunDatePeriod(m_classes::TMySqlQuery& query, TStr& s)
	{
		s +=
			TStr(query.fieldByName("fTitle"))+";"+
			query.fieldByName("Hits")+";"+
			"0;"+
			query.fieldByName("Signups")+";"+
			query.fieldByName("DepositSum")+";"+
			query.fieldByName("realPlayers")+";"+
			query.fieldByName("realRounds")+";"+
			query.fieldByName("funPlayers")+";"+
			query.fieldByName("funRounds")+";";
		s += "\n";
	}

	void RunTools(const bool Del, TStr& s)
	{
		TStr GCLID(cgi->getParamValueByName("id"));
		if( GCLID.Length() == 0 ) {
			s = "unknow GCLID";
			return;
		}
		OnStart();
		int  Amount = TStr(cgi->getParamValueByName("amount")).ToIntDef(10000);
		int  Count = TStr(cgi->getParamValueByName("count")).ToIntDef(1);
		m_classes::TMySqlQuery query(db),q(db);
		query.openSql(
			"SELECT tbl_users.id,tbl_users.freg_gclid,tbl_users.freg_gcl,tbl_users.flogin,count(log_pay_order.fcash) as Count,sum(log_pay_order.fcash) as Sum "
			"FROM tbl_users, log_pay_order "
			"WHERE tbl_users.id = log_pay_order.fid_user and log_pay_order.fid_status=11 and log_pay_order.fcash>0 and LENGTH(tbl_users.freg_gclid)>4 "
			"GROUP BY tbl_users.id "
			"HAVING Sum>="+TStr(Amount)+" and Count>="+TStr(Count));
		int i = 0, j = 0;
		while( !query.eof() )
		{
			bool f = TStr(query.fieldByName("freg_gcl")).Copy(0,GCLID.Length()) == GCLID;
			if( f && Del ) {
				try {
					q.execSql("UPDATE tbl_users SET freg_gclid=\"\" WHERE ID="+TStr(query.fieldByName("id")));
					i++;
				} catch( ... ) {
					printf("<p>ERROR: cm=del");
				}
			}
			if( f && !Del ) {
				s +=
					TStr("id=") + TStr(query.fieldByName("id")) +
					";login=" + __unescape(query.fieldByName("flogin")) +
					";sum=" + query.fieldByName("Sum") +
					";count=" + query.fieldByName("Count") +
					";gcl=" + __unescape(query.fieldByName("freg_gclid")) +
					";<br>\n";
				j++;
			}
			query.next();
		}
		s += "<p>gclid="+GCLID+"  amount="+TStr(Amount)+"  count="+TStr(Count)+"<p>";
		if( Del )
			s += "<p>change "+TStr(i)+" records";
		else
			s += "<p>list "+TStr(j)+" records";
		OnEnd();
	}

	void RunUserReg(TStr& s)
	{
		TStr Day(cgi->getParamValueByName("Day"));
		if( Day.isEmpty() ) {
			s = "date is empty";
			return;
		}
		OnStart();
		m_classes::TMySqlQuery query(db);
		query.openSql(
			"select "
			" tbl_users.ID, "
			" tbl_users.freg_gcl, "
			" tbl_users.freg_ip "
			"from "
			" tbl_users "
			"where "
			" length(tbl_users.freg_gclid)>2 and "
			" tbl_users.fdate_reg = '"+Day+"' "
		);
		while( !query.eof() )
		{
			TStr sGCL(query.fieldByName("freg_gcl"));
//			sGCL = sGCL.CopyBefore(",") + "," + sGCL.CopyAfter(",").CopyBefore(",");
			s +=
				TStr(query.fieldByName("ID"))+";"+
				sGCL+";"+
				query.fieldByName("freg_ip")+";";
			s += "\n";
			query.next();
		}
		OnEnd();
	}

	void RunUserBalance(TStr& s)
	{
		TStr sql =
			"SELECT "
			" tbl_users.id,"
			" tbl_users.freg_gcl,"
			" MIN(tbl_users.fdate_reg) as fdate_reg,"
			" SUM(tbl_users_balance.fcash_deposit) as deposits,"
			" SUM(tbl_users_balance.fcash_losses) as losses "
			"FROM  "
			" tbl_users, tbl_users_balance "
			"WHERE  "
			" LENGTH(tbl_users.freg_gcl) > 3 and "
			" tbl_users.ID = tbl_users_balance.FID_User ";
		TStr Day(cgi->getParamValueByName("Day"));
		TStr DateBegin(cgi->getParamValueByName("DateBegin"));
		TStr DateEnd(cgi->getParamValueByName("DateEnd"));
		if( !Day.isEmpty() )
			sql += " and tbl_users_balance.fdate = '"+Day+"' ";
		else
		if( !DateBegin.isEmpty() )
			sql += " and tbl_users_balance.fdate >= '"+DateBegin+"' ";
		else
		if( !DateEnd.isEmpty() )
			sql += " and tbl_users_balance.fdate <= '"+DateEnd+"' ";
		else {
			s = "date is empty";
			return;
		}
		sql += " GROUP BY tbl_users.ID, tbl_users.freg_gclid";
		OnStart();
		m_classes::TMySqlQuery query(db);
		query.openSql(sql);
		while( !query.eof() )
		{
			s +=
				TStr(query.fieldByName("freg_gcl")) + ";" +
				query.fieldByName("id") + ";" +
				query.fieldByName("deposits") + ";" +
				query.fieldByName("losses") + ";" +
				TStr(
					TStr(query.fieldByName("deposits")).ToIntDef(0) +
					TStr(query.fieldByName("losses")).ToIntDef(0)
				) + ";" +
				query.fieldByName("fdate_reg") + ";\n";
			query.next();
		}
		OnEnd();
	}

	bool Run(const TStr& cm, TStr& s)
	{
		bool f = true;
		OnBegin();
		if( cm == "deposit" )
			RunDate(true,s);
		else
		if( cm == "period" )
			RunDate(false,s);
		else
		if( cm == "tools_view" )
			RunTools(false,s);
		else
		if( cm == "tools_del" )
			RunTools(true,s);
		else
		if( cm == "user_reg" )
			RunUserReg(s);
		else
		if( cm == "user_balance" )
			RunUserBalance(s);
		else
			f = false;
		OnEnd();
		return f;
	}
};

int main()
{
	TCGI cgi;
	m_classes::TMySqlConnect db;

	cgi.makeHeader(
			"text/html; charset=Windows-1251"
			"\nExpires: Mon, 26 Jul 1997 05:00:00 GMT"
			"\nX-Accel-Expires: 0"
//			"\nPragma: no-cache"
			"\nCache-Control: none");

	cgi.init();

	TStr cm(cgi.getParamValueByName("cm"));

	db.setHostName("localhost");
	db.setDatabase("casino_last");
	db.setLogin("root");
	db.setPasswd("");

	TCM r(&db,&cgi);

	TStr s;

	try {
		if( !r.Run(cm,s) )
			s = "unknow cm="+cm;
	} catch (TExcept& e) {
		s = e.getError();
	}

	printf("%s",s.c_str());

	return 0;
}



