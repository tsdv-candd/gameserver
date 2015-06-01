#include <stdio.h>
#include "sql/mMySQL.h"

class TUserBalance {
private:
	m_classes::TMySqlConnect db;
public:
	void Init(const TStr& base) {
		db.setHostName("localhost");
		db.setDatabase(base.c_str());
		db.setLogin("root");
		db.setPasswd("");
		db.connect();
	}

	void Run() {
		m_classes::TMySqlQuery query(&db);
		m_classes::TMySqlQuery q(&db);
		q.openSql("SELECT now()");
		TStr NowDate(q.fields(0));
		query.openSql(
			"SELECT tbl_users.ID, tbl_users.fCash_real, tbl_users.fActive "
			"FROM   tbl_users,log_pay_order "
			"WHERE tbl_users.id = log_pay_order.fid_user "
			"GROUP BY tbl_users.ID");
		while( !query.eof() )
		{
			TStr UserID(query.fieldByName("ID"));
			TStr UserLastDate;
			int  UserCash = TStr(query.fieldByName("fCash_real")).ToInt();
			int  Deposit = 0;
			int  Losses = 0;
			q.openSql(
				"SELECT fdate,ftime "
				"FROM   tbl_users_balance "
				"WHERE  FID_User="+UserID+" "
				"ORDER BY fdate DESC,ftime DESC "
				"LIMIT 1");
			if( !q.eof() )
				UserLastDate = TStr(q.fieldByName("fdate")) + " " + q.fieldByName("ftime");

			q.openSql(
				"SELECT "
				" SUM(fcash) as Deposit "
				"FROM log_pay_order "
				"WHERE "
				" fid_user="+UserID+" and "
				" fCash > 0 and "
				" fid_status = 11 and "
				" DATE_ADD(fdate,INTERVAL ftime HOUR_SECOND) > '"+UserLastDate+"' and"
				" DATE_ADD(fdate,INTERVAL ftime HOUR_SECOND) <= '"+NowDate+"'");
			if( !q.eof() )
				Deposit = TStr(q.fields(0)).ToIntDef(0);
			q.openSql(
				"SELECT "
				" SUM(fcash) as Losses "
				"FROM log_pay_order "
				"WHERE "
				" fid_user="+UserID+" and "
				" fCash < 0 and "
				" fid_status in (11,30,31,32) and "
				" DATE_ADD(fdate,INTERVAL ftime HOUR_SECOND) > '"+UserLastDate+"' and "
				" DATE_ADD(fdate,INTERVAL ftime HOUR_SECOND) <= '"+NowDate+"'");
			if( !q.eof() )
				Losses = TStr(q.fields(0)).ToIntDef(0);
			bool f = false;
			if( Losses != 0 || Deposit != 0 )
				if( query.fieldByName("fActive")[0] != '1' || UserCash == 0 )
					f = true;
			if( Deposit == 0 && Losses != 0 )
				f = true;
			if( UserCash != 0 && Deposit != 0 && (UserCash*100.0/Deposit) < 1 )
				f = true;
			if( f ) {
				q.execSql(
					"INSERT INTO tbl_users_balance ("
					" fid_user,"
					" fdate,"
					" ftime,"
					" fcash_deposit,"
					" fcash_losses,"
					" fcash "
					") VALUES ("
					+UserID+","
					+"'"+NowDate+"',"
					+"'"+NowDate+"',"
					+TStr(Deposit)+","
					+TStr(Losses)+","
					+TStr(UserCash)+
					")");
				printf("\nuserid=%s deposit=%d losses=%d",UserID.c_str(),Deposit,Losses);
			}
			query.next();
		}
	}
};

int main(void)
{
	TUserBalance UserBalance;
	try {
		UserBalance.Init("casino_last");
		UserBalance.Run();
	} catch( TExcept& e) {
		printf("%s\n",e.getError());
	}
	return 0;
}

