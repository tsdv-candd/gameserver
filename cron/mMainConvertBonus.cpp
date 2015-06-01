#include <stdio.h>
#include "utils/m_datetime.h"
#include "socket/m_socket.h"
#include "sql/mMySQL.h"
#include "m_string.h"

m_classes::TMySqlConnect db;
TSocketClient fSock;

bool Connect()
{
	if( fSock.isOpen() )
		return true;
	return fSock.Connect("127.0.0.1",1024);
}

TStr Read()
{
	return fSock.Read(10000);
}

void Send(const TStr& s)
{
	fSock.Send(s);
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
		m_classes::TMySqlQuery query(&db),q(&db);
		query.openSql("select ID,fCash_bonus from tbl_users where factive=1 and fcash_bonus >= 2000");
		Connect();
		while( !query.eof() )
		{
			int Cash = query.fieldByNameAsInt("fCash_bonus");
			Cash = (Cash - Cash%2000) / 100;
			TStr s(Cash);
			s = "<adm command=\"update\"><user id=\""+TStr(query.fieldByName("ID"))+"\" fCash_Real=\""+s+"\" /><user id=\""+TStr(query.fieldByName("ID"))+"\" fCash_Bonus=\"-"+s+"\" /></adm>";
/*
			SendMail(
				"pay@va-bank.com",
				"pay@va-bank.com",
				"convert bonus points",
				("userid="+TStr(query.fieldByName("ID"))+"<br><a href=\"https://www.va-bank.com/adm/?AdmUserList&Edit="+TStr(query.fieldByName("ID"))+"\">link</a><br>"+DateTimeToStr(Now(),"%Y-%m-%d")+" - convert bonus points: $"+TStr(Cash)).c_str(),
				""
			);
*/
			Send(s);
			s = Read();
			q.execSql("UPDATE tbl_users SET fcomment=CONCAT('"+DateTimeToStr(Now(),"%Y-%m-%d")+" - convert bonus points: $"+TStr(Cash)+"\n',fcomment) WHERE ID="+TStr(query.fieldByName("ID")));
			query.next();
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



