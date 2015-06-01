#include <stdio.h>
#include "utils/m_datetime.h"
#include "sql/mMySQL.h"
#include "m_string.h"

m_classes::TMySqlConnect db;

TStr CheckNULL(const TStr& Value)
{
	return Value.ToIntDef(0);
}

bool LogHistoryDay(
		const TStr& ID,
		const TStr& Date)
{
	m_classes::TMySqlQuery query(&db);
	TStr sql;
	sql =
		"REPLACE INTO stat_day_history_"+ID+" "
		"SELECT "
		"  fdate,"
		"  fid_user,"
		"  fid_session,"
		"  fid_game,"
		"  Sum(fcash_bet),"
		"  Sum(fcash_win),"
		"  Count(*),"
		"  Min(fcash_bet),"
		"  Avg(fcash_bet),"
		"  Max(fcash_bet),"
		"  Min(fcash_win),"
		"  Avg(fcash_win),"
		"  Max(fcash_win), "
		"  now() "
		"FROM log_history_"+ID+" "
		"WHERE fDate='"+Date+"' "
		"GROUP BY fid_user, fid_session, fid_game";
	query.execSql(sql);
	return true;
}

bool LogURLDay(const TStr& Date)
{
	m_classes::TMySqlQuery query(&db);
	query.openSql(
		"SELECT "
		"  Count(ID) as fhits,"
		"  Count(distinct fip) as fhosts,"
		"  Count(distinct fid_user) as fusers "
		"FROM log_url "
		"WHERE fDate='"+Date+"' "
	);
	query.execSql(
		"REPLACE stat_day_url SET "
		" flast=now(), "
		" fhits = "+TStr(query.fieldByName("fhits"))+","
		" fhosts = "+TStr(query.fieldByName("fhosts"))+", "
		" fusers = "+TStr(query.fieldByName("fusers"))+", "
		" fdate='"+Date+"'");
	query.openSql(
		"SELECT "
		"  Count(ID) as fhits,"
		"  Count(distinct fip) as fhosts "
		"FROM log_url "
		"WHERE fDate='"+Date+"' and ftimeload<>'00:00:00'"
	);
	query.execSql(
		"UPDATE stat_day_url SET "
		" flast=now(), "
		" fhitsload = "+TStr(query.fieldByName("fhits"))+","
		" fhostsload = "+TStr(query.fieldByName("fhosts"))+" "
		"WHERE fdate='"+Date+"'");
	return true;
}

bool LogURLGCLDay(const TStr& Date)
{
	m_classes::TMySqlQuery query(&db);
	TStr sql;
	sql =
		"REPLACE INTO stat_day_urlgcl "
		"SELECT "
		"  fDate,"
		"  fidkey,"
		"  Count(ID) as fhits,"
		"  Count(distinct fip) as fhosts,"
		"  Count(distinct fid_user) as fusers,"
		"  Sum(freg) as fsignups, "
		"  now() "
		"FROM log_url "
		"WHERE fdate='"+Date+"' "+
		"GROUP BY fidkey";
	query.execSql(sql);
	return true;
}

bool LogToStat(TStr Date)
{
	int Mode = 0;
	TStr WHERE;
	{
		TStr Year(Date.CopyBefore("-"));
		TStr Month(Date.CopyAfter("-"));
		if( Month.CopyAfter("-").isEmpty() ) {
			Mode = 2;
			WHERE = " Month(fDate)='"+Month+"' and Year(fDate)='"+Year+"' ";
		} else {
			Mode = 1;
			WHERE = " fDate='"+Date+"' ";
		}
	}
	if( Mode == 0 )
		return false;
	m_classes::TMySqlQuery query(&db);
	// log_url
	query.openSql(
		"SELECT SUM(fHits) as fHits,SUM(fHosts) as fHosts,SUM(fHitsLoad) as fHitsLoad "
		"FROM stat_day_url "
		"WHERE "+WHERE);
	TStr Hits(CheckNULL(query.fieldByName("fHits")));
	TStr Hosts(CheckNULL(query.fieldByName("fHosts")));
	TStr HitsLoad(CheckNULL(query.fieldByName("fHitsLoad")));
	// log_history_1
	query.openSql(
		"SELECT Sum(frounds) as Rounds, Count(distinct fid_user) as Players "
		"FROM stat_day_history_1 "
		"WHERE "+WHERE);
	TStr RealRounds(CheckNULL(query.fieldByName("Rounds")));
	TStr RealPlayers(query.fieldByName("Players"));
	// log_history_2
	query.openSql(
		"SELECT Sum(frounds) as Rounds, Count(distinct fid_user) as Players "
		"FROM stat_day_history_2 "
		"WHERE "+WHERE);
	TStr FunRounds(CheckNULL(query.fieldByName("Rounds")));
	TStr FunPlayers(query.fieldByName("Players"));
	// tbl_users
	query.openSql(
		"SELECT Count(*) as Signups "
		"FROM tbl_users "
		"WHERE "+WHERE.Replace("fDate","fDate_reg"));
	TStr Signups(query.fieldByName("Signups"));
	// log_pay_order 
	query.openSql(
		"SELECT Count(*) as Count, Sum(fCash) as Sum "
		"FROM log_pay_1 "
		"WHERE "+WHERE+" and fid_system not in (8,20,21,50,51,100) and fcash > 0 and fpassive=0");
/*
		"SELECT Count(*) as Count, Sum(fCash) as Sum "
		"FROM log_pay_order "
		"WHERE "+WHERE+" and fid_status=11 and fcash > 0");
*/
	TStr InCount(query.fieldByName("Count"));
	TStr InSum(CheckNULL(query.fieldByName("Sum")));
	// !!!
	query.openSql(
		"SELECT Count(*) as Count "
		"FROM log_pay_1 "
		"WHERE "+WHERE+" and fid_system not in (8,20,21,50,51,100) and fcash > 0 and fpassive=0");
	// !!!
	InCount = query.fieldByName("Count");
	query.openSql(
		"SELECT Count(*) as Count, Sum(fCash) as Sum "
		"FROM log_pay_1 "
		"WHERE "+WHERE+" and fid_system not in (2,8,20,21,50,51,100) and fcash < 0 and fpassive=0");
/*
		"SELECT Count(*) as Count, Sum(fCash) as Sum "
		"FROM log_pay_order "
		"WHERE "+WHERE+" and fid_status=11 and fcash < 0");
*/
	TStr OutCount(query.fieldByName("Count"));
	TStr OutSum(CheckNULL(query.fieldByName("Sum")));
	query.openSql(
		"SELECT Count(*) as Count, Sum(fCash) as Sum "
		"FROM log_pay_order "
		"WHERE "+WHERE+" and fid_status in (30,31,32,33)");
	TStr RfCount(query.fieldByName("Count"));
	TStr RfSum(CheckNULL(query.fieldByName("Sum")));
/*
	query.openSql(
		"SELECT Sum(fCash) as Sum "
		"FROM log_pay_order "
		"WHERE "+WHERE+" and fid_status in (11,30,31,32,33)");
	TStr Balance(CheckNULL(query.fieldByName("Sum")));
*/
	TStr Balance(InSum.ToIntDef(0)+OutSum.ToIntDef(0)+RfSum.ToIntDef(0));

	if( Mode == 2 )
		Date += "-00";
	TStr sql =
		"REPLACE INTO stat_day ("
		" fdate,"
		" fhits,"
		" fhosts,"
		" fhitsload,"
		" fsignups,"
		" fin_count,"
		" fin_sum,"
		" fout_count,"
		" fout_sum,"
		" frf_count,"
		" frf_sum,"
		" fbalance,"
		" freal_players,"
		" freal_rounds,"
		" ffun_players,"
		" ffun_rounds,"
		" flast "
		") VALUES ("
		"'"+Date+"',"
		+Hits+","
		+Hosts+","
		+HitsLoad+","
		+Signups+","
		+InCount+","
		+InSum+","
		+OutCount+","
		+OutSum+","
		+RfCount+","
		+RfSum+","
		+Balance+","
		+RealPlayers+","
		+RealRounds+","
		+FunPlayers+","
		+FunRounds+","
		"now()"
		")";
	query.execSql(sql);
	return true;
}

void LogDay(const TStr& Date)
{
	long dt;
	// History1
	dt = GetTimeMs();
	printf("\nStatDayHis%1s  : %s ","1",Date.c_str());
	LogHistoryDay("1",Date);
	printf(": %d ms",GetTimeMs()-dt);
	// History2
	dt = GetTimeMs();
	printf("\nStatDayHis%1s  : %s ","2",Date.c_str());
	LogHistoryDay("2",Date);
	printf(": %d ms",GetTimeMs()-dt);
	// URL
	dt = GetTimeMs();
	printf("\nStatDayURL   : %s ",Date.c_str());
	LogURLDay(Date);
	printf(": %d ms",GetTimeMs()-dt);
	// URLGCL
	dt = GetTimeMs();
	printf("\nStatDayURLGCL: %s ",Date.c_str());
	LogURLGCLDay(Date);
	printf(": %d ms",GetTimeMs()-dt);
	// Day
	dt = GetTimeMs();
	printf("\nStatDay      : %s ",Date.c_str());
	LogToStat(Date);
	printf(": %d ms",GetTimeMs()-dt);
}

TStr DateToYearMonth(const TStr& Date)
{
	TStr Month;
	int k = 0;
	for( int j = 0; j < Date.Length(); j++ )
	{
		if( Date[j] == '-' ) {
			k++;
			if( k >= 2 )
				break;
		}
		Month += Date[j];
	}
	return Month;
}

int main()
{
	printf("\n\nstart %s\n",DateTimeToStr(Now()).c_str());
	db.setHostName("localhost");
	db.setDatabase("casino_last");
	db.setLogin("root");
	db.setPasswd("");
	try {
	try {
		db.connect();
		m_classes::TMySqlQuery query(&db);
		query.openSql("select NOW()");
		TStr DateNow(query.fields(0));
		DateNow = DateNow.CopyBefore(" ");
		query.openSql("SELECT MIN(fDate) FROM log_url" /*WHERE fDate < Now()*/);
		if( !query.eof() ) {
			TStr DateMin(query.fields(0));
			query.openSql("SELECT MAX(fDate) FROM log_url" /*WHERE fDate < Now()*/);
			if( !query.eof() ) {
				TStr DateMax(query.fields(0));
				TStr Date(DateMin);
				TStr MonthList('&');
				int i = 0;
				// DAY
				while( Date != DateMax && Date != DateNow )
				{
					query.openSql(
						"SELECT * FROM stat_day "
						"WHERE fDate='"+Date+"' and fActive=1 "
						"LIMIT 1");
					if( query.eof() ) {
						LogDay(Date);
						query.execSql(
							"UPDATE stat_day SET fActive=1,flast=now() "
							"WHERE fDate='"+Date+"'");
						//
						TStr Month(DateToYearMonth(Date));
						Month += "&";
						if( MonthList.Pos("&"+Month) < 0 )
							MonthList += Month;
					}
					i++;
					query.openSql("select DATE_ADD('"+DateMin+"',INTERVAL "+TStr(i)+" DAY)");
					Date = query.fields(0);
					Date = Date.CopyBefore(" ");
				}
				// NOW
				Date = DateNow;
				LogDay(Date);
				Date = DateToYearMonth(Date)+"&";
				if( MonthList.Pos("&"+Date) < 0 )
					MonthList += Date;
				// MONTH
				MonthList = MonthList.CopyAfter(0);
				Date = MonthList.GetBefore("&");
				while( !Date.isEmpty() )
				{
					long dt = GetTimeMs();
					printf("\nStatMonth    : %s ",Date.c_str());
					LogToStat(Date);
					printf(": %d ms",GetTimeMs()-dt);
					Date = MonthList.GetBefore("&");
				}
			}
		}

	} catch (TExcept& e) {
		printf("\n%s",e.getError());
	}
	} catch (...) {
		printf("\nerror");
	}
	printf("\n");

	return 0;
}

