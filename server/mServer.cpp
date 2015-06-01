#include "mCommon.h"
#include "mServer.h"
#include "mUser.h"
#include "mGame.h"
#include "mMoney.h"
#include "mail/m_mail.h"
#include "utils/m_utils.h"
#include "http/m_http.h"

#include <stdio.h>


TStr CreateGUID(const int len)
{
	TStr s(TStr((long)time(NULL)));
	while( s.Length() < len )
	{
		s = TStr(getRnd(10)) + s;
	}
	return s;
}

#ifdef CASINO_VABANK
class TGCLThread: public TThread
{
private:
	TStr RefID;
	TStr URL;
	TStr Param;
protected:
	void Run() {
		try {
		if( RefID.ToIntDef(0) == 0 )
			return;
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql(
			"SELECT fid, fidkey "
			"FROM log_url "
			"WHERE ID="+RefID);
		if( !query.eof() ) {
			if( strlen(query.fields(0)) > 3 ) {
				TStr Key(query.fields(1));
				Key = Key.Copy(0,4);
				char *host = NULL;
				char *server = NULL;
				host = "127.0.0.1";
				server = "www.va-bank.com";
				if( host != NULL && server != NULL ) {
					THTTPConnect c(host,80,server);
					TStr _URL;
					if( Key == "daae" )
						_URL = "cgi-bin/gc2/"+URL;
					else
					if( Key == "daaf" )              // ���
						_URL = "cgi-bin/gc3/"+URL;
					else
					if( Key == "daak" )              // ���
						_URL = "cgi-bin/gc4/"+URL;
					else
					if( Key == "daal" )
						_URL = "cgi-bin/gc5/"+URL;
					else
					if( Key == "eaaa" )
						_URL = "cgi-bin/gc7/"+URL;
					else
					if( Key == "eaab" )
						_URL = "cgi-bin/gc8/"+URL;
					else
					if( Key.Copy(0,3) == "daa" )
						_URL = "cgi-bin/gc/"+URL;
					if( !_URL.isEmpty() )
						c.Run(_URL+"?"+ToLowerCase(TStr(query.fields(0))+Param));
				}
			}
		}
		} catch(...) {
		}
	}

public:
	TGCLThread(const TStr& _RefID, const TStr& _URL, const TStr& _Param = "")
	:RefID(_RefID), URL(_URL), Param(_Param)
	{}

	void Create() { TThread::Create(true); }

};
#endif


class TMailThread: public TThread
{
private:
	TStr fSubject;
	TStr fBody;
	TStr fEMail;
protected:
	void Run() {
		char* email = "";
		if( email != NULL )
			try {
				print("\n>> "+DateTimeToStr(Now(),"%Y-%m-%d %H:%M:%S")+" >> TMailThread::sendmail start "+fEMail);
				bool f = false;
				try {
#ifdef CASINO_TWIST
					THTTPConnect c(IP,Port,Host);
					c.Run(URL,"EMail="+fEMail+"&Subject="+fSubject+"&Body="+fBody,true);
#else
					TServerDB  rb;
					m_classes::TMySqlQuery query(rb.db());
					query.execSql(
						"INSERT DELAYED INTO mail (fDate,fTime,fSubject,fEMail,fBody) VALUES "
						"(now(),now(),'"+StrToSQL(fSubject)+"','"+StrToSQL(fEMail)+"','"+StrToSQL(fBody)+"')");
#endif
					f = true;
//					bool f = SendMail(email,fEMail.c_str(),fSubject.c_str(),fBody.c_str(),Header);
				} catch( ... ) {
				}
				TStr s("ok   ");
				if( !f )
					s = "error";
//				print("\n>> "+DateTimeToStr(Now(),"%Y-%m-%d %H:%M:%S")+" >> TMailThread::sendmail "+s+" "+fEMail);
			} catch(...) {
			}
	}
public:
	static TStr IP;
	static int  Port;
	static TStr URL;
	static TStr Host;

	TMailThread(const TStr& Subject, const TStr& Body, const TStr& EMail)
	:fSubject(Subject), fBody(Body), fEMail(EMail)
	{}
	void Create() { TThread::Create(true); }
};

TStr TMailThread::IP = "127.0.0.1";
int  TMailThread::Port = 80;
TStr TMailThread::URL = "/mail/?";
TStr TMailThread::Host = "";


/*
class TActionThread: public TThread
{
private:
	TStr fIP;
	int  fPort;
	TStr fServer;
	TStr fURL;
	bool fSSL;
protected:
	void Run() {
		TStr s1,s2;
		if( fSSL ) {
			TCreate<THTTPSConnect> c(new THTTPSConnect(fIP,fPort,fServer));
			c()->Run(fURL);
			s1 = c()->Request();
			s2 = c()->Header() + c()->Body();
		} else {
			TCreate<THTTPConnect> c(new THTTPConnect(fIP,fPort,fServer));
			c()->Run(fURL);
			s1 = c()->Request();
			s2 = c()->Body();
		}
		print("TActionThread::Run",s1+"\n\n"+s2);
	}

public:
	TActionThread(
			const TStr& IP,
			const TStr& Server,
			const TStr& URL,
			const bool SSL)
	:fIP(IP), fPort(80), fURL(URL), fServer(Server), fSSL(SSL)
	{
		if( SSL )
			fPort = 443;
	}
	void Create() { TThread::Create(true); }
};
*/


class TURLThread: public TThread
{
private:
	int  fPort;
	TStr fHost;
	TStr fIP;
	TStr fURL;
protected:
	void Run() {
		TCreate<THTTPConnect> c(new THTTPConnect(fIP,fPort,fHost));
		c()->Run(fURL,"",false);
		TStr s1 = c()->Request();
		TStr s2 = c()->Body();
		print("TURLThread::Run",s1+"\n\n"+s2);
	}
public:
	TURLThread(
			const TStr& IP,
			const TStr& Host,
			const TStr& URL)
	:fPort(80), fURL(URL), fHost(Host), fIP(IP)
	{}
	void Create() { TThread::Create(true); }
};


void HTTP_IP(const TStr& IP)
{
#ifdef CASINO_VABANK
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.openSql("SELECT fip FROM ip WHERE fip='"+IP+"'");
	if( query.eof() ) {
		query.execSql("REPLACE INTO ip (fip,finfo) VALUES ('"+IP+"','check')");
		TURLThread* p = new TURLThread("195.161.118.102","gcl.ru","s1.pl?ip="+IP);
		p->Create();
	}
#endif
}


void HTTP_Reg(
		const TStr& REFID,
		const TStr& USERID,
		const TStr& ID)
{
}


void HTTP_Lobby(const TStr& RefID) {
#ifdef CASINO_VABANK
	TGCLThread * p;
	p = new TGCLThread(RefID,"icount_2.cgi");
	p->Create();
#endif
}


void HTTP_Deposit(const TStr& RefID, const TStr& Cash) {
#ifdef CASINO_VABANK
	TGCLThread * p;
	p = new TGCLThread(RefID,"icount_3.cgi",","+Cash);
	p->Create();
#endif
}

//
// isCommand
//

struct TCommand {
	char fIP[16];
	long fTime;
};

template <int type, int maxc>
bool isCommand(const TStr& IP)
{
	int c = 0;
	bool f = true;
	static TCommand cm[1000];
	for( int i = 0; i < 1000; i++ ) {
		if( cm[i].fTime > 0 && (cm[i].fTime + 60) < time(NULL) )
			cm[i].fTime = 0;
		if( cm[i].fTime > 0 && IP == cm[i].fIP )
			c++;
		if( f && cm[i].fTime == 0 ) {
			strncpy(cm[i].fIP,IP.c_str(),16);
			cm[i].fTime = time(NULL);
			f = false;
		}
	}
	return c < maxc;
}

//
// Subscribe
//

void __SendMail_User(
	TServerSSI& rs,
	const TStr& Name,
	const TStr& UserID)
{
	try {
		TStr Param;
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		TStr r("ru");
		if( !UserID.isEmpty() ) {
			rs.ssi()->SSIValue("UserID",UserID);
			rs.ssi()->SSIValue("ID",UserID);
			TStr sql(rs.ssi()->SSIResDef("mail_"+Name+"_sql",""));
			if( sql.isEmpty() )
				sql = rs.ssi()->SSIResDef("mail_sql","");
			if( !sql.isEmpty() ) {
				query.openSql(sql);
				if( !query.eof() ) {

					Param = TStr("Login=")+query.fieldByName("fLogin")+";";
					Param += TStr("Password=")+query.fieldByName("fLogin")+";";
					Param += TStr("Amount=")+query.fieldByName("fCash_real")+";";
					Param += TStr("Bonus=")+query.fieldByName("fCash_bonus")+";";
					Param += TStr("Email=")+query.fieldByName("fEMail")+";";
					if( query.fieldByName("fid_mode")[0] == '1' )
						Param += "Mode=real;";
					else
						Param += "Mode=fun;";
					if( query.fieldByName("fid_Lang")[0] != '1' )
						Param += "Lang=en;";
					else
						Param += "Lang=ru;";

					rs.ssi()->SSIValue("code",query.fieldByName("fCode"),false);
					rs.ssi()->SSIValue("fLogin",__unescape(query.fieldByName("fLogin")),false);
					rs.ssi()->SSIValue("fPassword",__unescape(query.fieldByName("fPassword")),false);
					rs.ssi()->SSIValue("fNick",__unescape(query.fieldByName("fNick")),false);
					rs.ssi()->SSIValue("fCash_real",CashToStr(TStr(query.fieldByName("fCash_Real")).ToDouble()),false);
					rs.ssi()->SSIValue("real",CashToStr(TStr(query.fieldByName("fCash_Real")).ToDouble()),false);
					rs.ssi()->SSIValue("bonus",CashToStr(TStr(query.fieldByName("fCash_Bonus")).ToDouble()),false);
					rs.ssi()->SSIValue("free",CashToStr(TStr(query.fieldByName("fCash_FreeK")).ToDouble()),false);
					rs.ssi()->SSIValue("fEMail",__unescape(query.fieldByName("fEMail")),false);
					rs.ssi()->SSIBlock("&isReal",query.fieldByName("fid_mode")[0] == '1',false);
					rs.ssi()->SSIBlock("&isFun",query.fieldByName("fid_mode")[0] != '1',false);
					if( query.fieldByName("fid_Lang")[0] != '1' )
						r = "en";

				}
			}
		}
		TStr EMail(rs.ssi()->SSIResDef("mail_"+Name+"_email",""));
		if( EMail.isEmpty() )
			EMail = rs.ssi()->SSIResDef("mail_email","");
		if( !EMail.isEmpty() ) {
#ifdef CASINO_TWIST
			TMailThread* p = new TMailThread(
				Name,
				Param,
				EMail);
			p->Create();
#else
			TMailThread* p = new TMailThread(
				rs.ssi()->SSIRes("mail_"+Name+"_subject_"+r),
				rs.ssi()->SSIRes("mail_"+Name+"_body_"+r),
				EMail);
			p->Create();
#endif
		}
	} catch(TExcept& e) {
		print(e.getError());
	}
}

void __SendMail_User(
	const TStr& Name,
	const TStr& UserID = "" )
{
	TServerSSI rs;
	__SendMail_User(rs,Name,UserID);
}

void SendMailReg( const TStr& UserID )
{
	__SendMail_User("Reg",UserID);
}

void SendMailRemind( const TStr& UserID )
{
	__SendMail_User("Remind",UserID);
}

void SendMailDepositComplete(
	const TStr& UserID,
	const TStr& TRID,
	const TStr& Amount )
{
	try {
		TServerSSI rs;
		rs.ssi()->SSIValue("TRID",TRID,false);
		rs.ssi()->SSIValue("Amount",Amount);
		__SendMail_User(rs,"DepositComplete",UserID);
	} catch(...) {
	}
}

void SendMailDepositError(
	const TStr& UserID,
	const TStr& TRID,
	const TStr& Amount )
{
	try {
		TServerSSI rs;
		rs.ssi()->SSIValue("TRID",TRID,false);
		rs.ssi()->SSIValue("Amount",Amount);
		__SendMail_User(rs,"DepositError",UserID);
	} catch(...) {
	}
}

void SendMailCashout(
	const TStr& UserID,
	const TStr& TRID,
	const TStr& Amount )
{
	try {
		TServerSSI rs;
		rs.ssi()->SSIValue("TRID",TRID,false);
		rs.ssi()->SSIValue("Amount",Amount);
		__SendMail_User(rs,"Cashout",UserID);
	} catch(...) {
	}
}

void SendMailEMailChange(
		const TStr& UserID,
		const TStr& OldEMail )
{
	try {
		TServerSSI rs;
		rs.ssi()->SSIValue("Old_email",OldEMail,false);
		__SendMail_User(rs,"change_email",UserID);
	} catch(...) {
	}
}

void SendMailCashoutComplete(
	const TStr& UserID,
	const TStr& TRID,
	const TStr& Amount )
{
	try {
		TServerSSI rs;
		rs.ssi()->SSIValue("TRID",TRID,false);
		rs.ssi()->SSIValue("Amount",Amount);
		__SendMail_User(rs,"CashoutComplete",UserID);
	} catch(...) {
	}
}

void SendMailBlock( const TStr& UserID )
{
	__SendMail_User("Block",UserID);
}

//
//
//

TServer* IServer::fServer = NULL;

class TAccept: public TThread, public IServer
{
private:
	TStr           fAcceptIP;
	TStr           fIP;
	unsigned int   fPort;
	bool           fExit;
	TSocketServer* fSock;
protected:
public:
	TAccept(): fPort(0), fExit(false), fSock(NULL)
	{}
	void Create(const unsigned int Port, const TStr& IP = "") {
		fIP = IP;
		fPort = Port;
		TThread::Create(false);
	}
	void Run() {
		GetTimeMs();
		TSocketServer Sock;
		if( Sock.Init(fPort,fIP) ) {
			fSock = &Sock;
			TSocket c;
			fExit = false;
			print("\nlisten port: ",fPort,"\n");
			print_log("listen  ip:"+fIP+"  port:"+TStr(fPort));
			while( !fExit )
			{
				fAcceptIP.Clear();
				c = fSock->Accept(fAcceptIP);
				if( fExit )
					break;
				if( fSock->isOpen() && c.isOpen() )
					OnAccept(c,fAcceptIP);
			}
		} else
			print("\nfailed port: ",fPort,"\n");
		fSock = NULL;
	}
	virtual void OnAccept(TSocket& c, const TStr& IP) = NULL;
	void Close() {
		fExit = true;
		TSocketClient s;
		s.Connect("127.0.0.1",fPort);
	}
};

// THTTP

class THTTPAccept: public TAccept
{
private:
protected:
	class THTTPThread: public TThread {
	public:
		TServer* fServer;
		TSocket  fSock;

		TServer* Server() { return fServer; }

		THTTPThread(TServer* Server, TSocket Sock): fServer(Server), fSock(Sock) {}

		void Run() {
			initRnd();
			while( true )
			{
				TStr s = fSock.Read(60000);
				if( s.isEmpty() || !fSock.isOpen() )
					break;
/*
				TXML xml;
				xml.FromStr(s.c_str());
				TStr sx;
				for( unsigned int i = 0; i < xml.NodesCount(); i++ )
				{
					TXMLNode* p = xml.GetNode(i);
					for( unsigned int k = 0; k < p->NodesCount(); k++ )
						sx += p->GetNode(k)->ToStr();
					if( i == xml.NodesCount()-1 ) {
						sx = TStr(">") + sx;
						for( int j = 0; j < p->VarCount(); j++ )
							sx = TStr(" ") + p->VarName(j)+TStr("=")+p->VarValue(j) + sx;
						sx = TStr("<")+p->GetName()+sx+TStr("</"+p->GetName()+">");
					}
				}
*/
				TServerRequest r(s);
				Server()->Request(r);
//				if( xml.FindNode("adm") == NULL && r.Session() == NULL )
				if( r.isClose() )
					fSock.Send("<server status=\"disconnect\" />");
				else
					fSock.Send(r.Send());
			}
			fSock.Close();
		}
	};
public:
	void OnAccept(TSocket& c, const TStr& IP) {
		THTTPThread* p = new THTTPThread(Server(),c);
		p->Create(true);
	}
};


// TConnect

class TConnectAccept: public TAccept
{
private:
protected:
public:
	void OnAccept(TSocket& c, const TStr& IP) {
		Server()->AcceptConnect(c,IP);
	}
};

unsigned int TConnect::TimeOut = 600;

void TConnect::Close()
{
#ifdef TARGET_WIN32
	fSocket.Close();
#endif
}

void TConnect::Run()
{
	initRnd();
	GetTimeMs();
	print("\n\nconnect ");
	while( true )
	{
		s = fSocket.Read(TimeOut);
		if( fSessionClose ) {
			print("!!! SERVER::CLOSE - SESSION");
			fSocket.Close();
		}
		if( s.isEmpty() || !fSocket.isOpen() )
			break;
		fCount++;
		fRead += s.Length();
		TServerRequest r(s,fSession,this);
		try {
			Server()->Request(r);
			bool f_IP = fSession == NULL;
			fSession = r.Session();
			if( f_IP && fSession != NULL ) {
				if( fIP.isEmpty() )
					fIP = fSocket.GetIP();
				fSession->fIP_Connect = fIP;
			}
		} catch( ... ) {
			print("!!! SERVER::CLOSE - ERROR");
			fSocket.Close();
		}
		if( r.isClose() ) {
			print("!!! SERVER::CLOSE - COMMAND");
			break;
		}
		if( !fSocket.Send(r.Send()) )
			break;
		fSend += r.Send().Length();
		fLast = Now();
	}
	print("\n\ndisconnect ");
	fSocket.Close();
	Server()->FreeConnect(this);
}

// TServer

TServer::TServer()
:
	dbStorage(NULL),
	ssiStorage(NULL),
	fTime(0),
	fSendLen(0),
	fReadLen(0),
	fCount(0)
{
	fConnectPool = new TObjectPool<TConnect>(1024*10,true);
	fSessionPool = new TObjectPool<TSession>(1024*10,true);
	fUserPool = new TObjectPool<TUser>(1024*10,true);
}

TServer::~TServer()
{
	if( dbStorage != NULL )
		delete dbStorage;
	if( ssiStorage != NULL )
		delete ssiStorage;
	delete fConnectPool;
	delete fSessionPool;
	delete fUserPool;
}

void TServer::Stop()
{
	print("!!! SERVER::STOP");
	fExit = true;
}

void TServer::Run()
{

/*
//  RAND !!!
	for( unsigned int fCardsCount = 0; fCardsCount < 52; fCardsCount++ )
		printf("%d ",getRnd(52-1+1)+1);
	printf("\n");

	unsigned int fCards[52];
	int i = 0;
	for( unsigned int fCardsCount = 0; fCardsCount < 52; fCardsCount++ )
	{
		for( unsigned int i = 0; i < fCardsCount; i++ )
			printf("%d ",fCards[i]);
		//
		if( !getRnd(
			fCards+fCardsCount,
			1,
			1,52,
			fCards,
			fCardsCount) )
			break;
		int Card = fCards[fCardsCount];
		printf("%d\n",Card);
		i++;
	}
	printf("\n=%d\n",i);
*/

	GetTimeMs();
	fCreate = Now();
	TStr IP = "";
	TStr IPHTTP = "";
	unsigned int Port = 8002;
	unsigned int PortHTTP = 1024;
	TUser::p_CasinoControl = &fCasinoControl;
	try {
		if( ssiStorage != NULL )
			delete ssiStorage;
		if( dbStorage != NULL )
			delete dbStorage;
		IServer::fServer = this;
		ssiStorage = new TSSIStorage(500,"server.ini");
		TServerSSI rs;
		dbStorage = new TMySqlStorage(
			500,
			rs.ssi()->SSIRes("db_host"),
			rs.ssi()->SSIRes("db_login"),
			rs.ssi()->SSIRes("db_password"),
			rs.ssi()->SSIRes("db_database"));
		TServerDB db;
		fCasinoControl.Load(db.db());
		TUser::Load(db.db());
		TConnect::TimeOut = rs.ssi()->SSIResDef("timeout","60000").ToIntDef(60000);
		fSessionTimer = rs.ssi()->SSIResDef("session_timeout","60000").ToIntDef(60000);
		fSleepMin = rs.ssi()->SSIResDef("sleepmin","0").ToIntDef(0);
		fSleepMax = rs.ssi()->SSIResDef("sleepmax","0").ToIntDef(0);
		Port = rs.ssi()->SSIResDef("port","8002").ToIntDef(8002);
		PortHTTP = rs.ssi()->SSIResDef("porthttp","1024").ToIntDef(1024);
		IP = rs.ssi()->SSIResDef("ip","");
		IPHTTP = rs.ssi()->SSIResDef("iphttp","");

		fLogPrint = rs.ssi()->SSIResDef("LogPrint","off") == "on";
		fLogFile = rs.ssi()->SSIResDef("LogFile","off") == "on";
		fLogBD = rs.ssi()->SSIResDef("LogBD","off") == "on";

		TMailThread::IP = rs.ssi()->SSIResDef("MailIP","127.0.0.1");
		TMailThread::Port = rs.ssi()->SSIResDef("MailPort","80").ToInt();
		TMailThread::URL = rs.ssi()->SSIResDef("MailURL","/cgi-bin/mail_send.cgi");
		TMailThread::Host = rs.ssi()->SSIResDef("MailHost","");

	} catch (TExcept &e) {
		print(e.getError());
		Sleep(10000);
		fOpen = false;
		return;
	}
	THTTPAccept AHTTP;
	AHTTP.Create(PortHTTP,IPHTTP);
	TConnectAccept AConnect;
	AConnect.Create(Port,IP);
	fConnectAdmChat = NULL;

	int i = 0;
	bool __fExit = false;
	while( !__fExit )
	{
		Sleep(500);
		if( fExit )
			i = 1000;
		try {
		try {
#ifdef TARGET_WIN32
		if( i++ >= 10 )
#else
		if( i++ >= 60 )
#endif
		{
			i = 0;
			TServerDB db;
			fCasinoControl.Save(db.db());
			TUser::Save(db.db());
			Timer();
			if( fExit ) {
				{
					TLockSection _l(&l);
					AConnect.Close();
					AHTTP.Close();
					fConnectPool->Lock();
					TConnect* Connect = fConnectPool->First();
					while( Connect != NULL )
					{
						Connect->Close();
						Connect = fConnectPool->Next(Connect);
					}
					fConnectPool->Unlock();
				}
				while( fConnectPool->Count() > 0 )
					Sleep(100);
				{
					TLockSection _l(&l);
					{
						TLockSection _ls(fSessionPool);
						TSession* Session = fSessionPool->First();
						while( Session != NULL )
						{
							TSession* Del = Session;
							Session = fSessionPool->Next(Session);
							SessionFree(Del);
						}
					}
					{
						TLockSection _lu(fUserPool);
						TUser* User = fUserPool->First();
						while( User != NULL )
						{
							TUser* Del = User;
							User = fUserPool->Next(User);
							Del->Save();
							Del->OnStatusLeave();
							fUserPool->Delete(Del,false);
						}
					}
					__fExit = true;
				}
			}
		}
		} catch( TExcept& e ) {
			print("SERVER::ERROR - "+TStr(e.getError()));
			throw e;
		}
		} catch( ... ) {
			print("SERVER::ERROR - TRY");
			if( !fExit )
				fExit = true;
			else
				__fExit = true;
		}
	}
	try {
		TServerDB db;
		fCasinoControl.Save(db.db());
	} catch( ... ) {
	}
	print("SERVER::CLOSE");
	fOpen = false;
}

TUser* TServer::UserCreate(TSession* Session)
{
	TUser* p;
	fUserPool->Insert(p = new TUser(),false);
	p->Auth(Session);
	return p;
}

TUser* TServer::UserAuth(
	const TStr& Login,
	const TStr& Password,
	TSession*   Session)
{
	TUser* p = NULL;
	p = fUserPool->First();
	while( p != NULL )
	{
		if( p->fLogin == Login && p->fPassword == Password ) {
			if( p->fClose ) {
//				Session->fClose = TSession::clBLOCK;
				p = NULL;
			} else {
				if( p->fSession != NULL )
					p->fSession->Close(TSession::clCLOSE);
				p->Close();
				p->Auth(Session);
			}
			break;
		}
		p = fUserPool->Next(p);
	}
	if( p == NULL ) {
		TServerSSI rs;
		rs.ssi()->SSIValue("LOGIN",Login);
		rs.ssi()->SSIValue("PASSWORD",Password);
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql(rs.ssi()->SSIRes("User_Auth"));
		if( !query.eof() ) {
			if( query.fieldByName("fActive")[0] == '0' ) {
				if( Session != NULL )
					Session->fClose = TSession::clBLOCK;
/*
			} else
			if( query.fieldByNameAsInt("fid_status") == 100 ) {
				if( Session != NULL )
					Session->fClose = TSession::clPASSIVE;
*/
			} else {
				fUserPool->Insert(p = new TUser(),false);
				p->Auth(query,Session);
				p->OnCashAdd();
			}
		}
	}
	if( p != NULL && Session != NULL && !Session->fRefID.isEmpty() ) {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.execSql(
			"UPDATE log_url "
			"SET "
			"  fid_user = "+TStr(p->fID)+", "
			"  fid_session = "+TStr(p->fLogID)+" "
			"WHERE ID='"+Session->fRefID+"'");
	}
	return p;
}

TStr TServer::RunAdm(TXMLNode *root)
{
	if( root == NULL )
		return "";
	TStr cm(root->VarValue("command"));
	if( cm == "lotto" ) {
		TStr r("<adm status=error />");
		unsigned int ID = root->TXMLNodes::VarValue("user/id").ToIntDef(0);
		if( ID > 0 ) {
			TStr Tickets(root->TXMLNodes::VarValue("user/tickets"));
			{
				TLockSection _ls(fSessionPool);
				TSession* Session = fSessionPool->First();
				while( Session != NULL )
				{
					if( Session->User() != NULL ) {
						if( Session->User()->ID() == ID ) {
							Session->fMessage += "<msg type=\"ticket\" value=\""+Tickets+"\" >";
							r = "<adm status=ok />";
							break;
						}
					}
					Session = fSessionPool->Next(Session);
				}
			}
		}
		return r;
	} else
	if( cm == "casher" ) {
		bool f = false;
		TStr s(OnCasher(
				root->FindNode("info"),
				root->TXMLNodes::VarValue("user/id").ToIntDef(0),
				"127.0.0.1",
				f));
		TStr r("<adm status=");
		if( f )
			r += "ok";
		else
			r += "error";
		r += ">"+s+"</adm>";
		return r;
	} else
	if( cm == "chat" ) {
		for( unsigned int i = 0; i < root->NodesCount(); i++ )
		{
			TXMLNode* p = root->GetNode(i);
			if( p->GetName() == "msg" ) {
				TSession* s = SessionFind(p->VarValue("GUID"));
				if( s != NULL )
					s->AddMsg(p->VarValue("txt"),true);
			}
		}
		TStr s;
		TSession* p = fSessionPool->First();
		while( p != NULL )
		{
			if( p->imas > 0 || p->fChatOpen ) {
				// ���� ��������� � ����
				TStr Nick("Guest "+TStr(p->ID()));
				if( p->fUser != NULL )
					Nick = p->fUser->fNick;
				s += "<session guid=\""+p->GUID()+"\" login=\""+Nick+"\" ";
				s += "lang=\""+TStr(p->fLang)+"\" ";
				if( p->fUser != NULL )
					s +=
						"id=\""+TStr(p->fUser->ID())+"\" "
						"mode=\""+TStr(3-p->fUser->Mode())+"\" "
						"cash=\""+TStr(p->fUser->Cash())+"\" ";
				s += ">";
				TStr tt;
				for( int i = 0; i < 20; i++ )
				{
					if( p->imas <= i )
						break;
					TStr t;
					t = "<msg id=\""+TStr(p->id[i])+"\" txt=\""+p->mas[i]+"\"";
					if( p->adm[i] )
						t += " adm=\"1\"" ;
					else
						t += " adm=\"0\"";
					t += " date=\""+p->date[i]+"\" ";
					t += " />";
					tt = t + tt;
				}
				s += tt;
				s += "</session>";
			}
			p = fSessionPool->Next(p);
		}
		return "<adm>"+s+"</adm>";
	}
	if( cm == "info" ) {
		TStr s("<system ");
		s += " start=\""+DateTimeToStr(fCreate,"[%H:%M:%S %d.%m.%Y]")+"\" ";
		s += " ttime=\""+DateTimeToStr(Now(),"[%H:%M:%S %d.%m.%Y]")+"\" ";
		s += " run=\""+TStr(TStr(fTime/1000) + "." + fTime%1000)+"\" ";
		s += " count=\""+TStr(fCount)+"\" ";
		s += " read=\""+TStr(fReadLen)+"\" ";
		s += " send=\""+TStr(fSendLen)+"\" ";
		s += " />\n";

		s += "<connectpool ";
		s += " count=\""+TStr(fConnectPool->Count())+"\"";
		s += " size=\""+TStr(fConnectPool->Size())+"\"";
		s += " />";

		s += "<sessionpool ";
		s += " count=\""+TStr(fSessionPool->Count())+"\"";
		s += " size=\""+TStr(fSessionPool->Size())+"\"";
		s += " />";

		s += "<userpool ";
		s += " count=\""+TStr(fUserPool->Count())+"\"";
		s += " size=\""+TStr(fUserPool->Size())+"\"";
		s += " />";

		s += fCasinoControl.XMLInfo();

		return s;
	}
	if( cm == "view" ) {
		TStr s("<system ");
		s += " start=\""+DateTimeToStr(fCreate,"[%H:%M:%S %d.%m.%Y]")+"\" ";
		s += " ttime=\""+DateTimeToStr(Now(),"[%H:%M:%S %d.%m.%Y]")+"\" ";
		s += " run=\""+TStr(TStr(fTime/1000) + "." + fTime%1000)+"\" ";
		s += " count=\""+TStr(fCount)+"\" ";
		s += " />\n";

		s += "<connectpool ";
		s += " count=\""+TStr(fConnectPool->Count())+"\"";
		s += " size=\""+TStr(fConnectPool->Size())+"\"";
		s += " />";

		s += "<sessionpool ";
		s += " count=\""+TStr(fSessionPool->Count())+"\"";
		s += " size=\""+TStr(fSessionPool->Size())+"\"";
		s += " />";

		s += "<userpool ";
		s += " count=\""+TStr(fUserPool->Count())+"\"";
		s += " size=\""+TStr(fUserPool->Size())+"\"";
		s += " />";

		s += fCasinoControl.XMLInfo();

/*
		fConnectPool->Lock();
		TConnect* Connect = fConnectPool->First();
		while( Connect != NULL )
		{
			s +=
				"<connect"
				" id=\""+TStr(Connect->ID())+"\" "
				" create=\""+DateTimeToStr(Connect->fCreate," %H:%M:%S")+"\""
				" last=\""+TStr(Now()-Connect->fLast)+"\""
				" read=\""+Connect->fRead+"\" "
				" send=\""+Connect->fSend+"\" "
				" count=\""+Connect->fCount+"\" ";
			if( Connect->fSession != NULL ) {
				s += " session=\""+TStr(Connect->fSession->ID())+"\" ";
				if( Connect->fSession->fUser != NULL )
					s += " user=\""+TStr(Connect->fSession->fUser->fID)+"\" ";
			}
			s += " />\n";
			Connect = fConnectPool->Next(Connect);
		}
		fConnectPool->Unlock();
*/
		TSession* Session = fSessionPool->First();
		while( Session != NULL )
		{
			s += "<session "
				" id=\""+TStr(Session->ID())+"\" "
				" create=\""+DateTimeToStr(Session->fCreate," %H:%M:%S")+"\" "
				" ip_http=\""+Session->fIP_HTTP+"\" "
				" ip_connect=\""+Session->fIP_Connect+"\" "
				" refid=\""+Session->fRefID+"\" "
				" last=\""+TStr(Now()-Session->fLast)+"\""
//				" read=\""+Session->fRead+"\""
//				" send=\""+Session->fSend+"\""
				" count=\""+Session->fCount+"\" ";
//				" run=\""+TStr(TStr(Session->fTime/1000) + "." + Session->fTime%1000)+"\"";
			if( Session->fUser != NULL )
				s += " user=\""+TStr(Session->fUser->fID)+"\" ";
			if( Session->fGame != NULL ) {
				s += " game=\""+TStr(Session->fGame->Name())+"\" ";
				if( Session->fGame->isMultiuser() )
					 s += " gamemulti=\"1\" ";
//				s += " game_id=\""+TStr(Session->fGame->ID())+"\" ";
			}
			s += " guid=\""+Session->fGUID+"\" ";
			s += "/>\n";
			Session = fSessionPool->Next(Session);
		}
		TUser* User = fUserPool->First();
		while( User != NULL )
		{
			s += "<user "
				" id=\""+TStr(User->fID)+"\" "
				" login=\""+User->fLogin+"\" "
				" status=\""+User->Mode()+"\" "
				" cash=\""+User->Cash()+"\" ";
			if( User->fSession != NULL )
				s += " session=\""+TStr(User->fSession->ID())+"\" ";
			s += "/>\n";
			User = fUserPool->Next(User);
		}
		return s;
	}
	if( cm == "update" )
		return RunAdm_Update(root);
	if( cm == "msg" ) {
		TStr Msg(root->VarValue("msg"));
		bool f = false;
		for( unsigned int i = 0; i < root->NodesCount(); i++ )
		{
			TStr SessionID(root->GetNode(i)->VarValue("session"));
			int  UserID = root->GetNode(i)->VarValue("user").ToIntDef(0);
			if( UserID > 0 ) {
				TUser* p = UserFind(UserID);
				if( p != NULL ) {
					p->fMessage += "<usermsg msg=\""+Msg+"\" />";
				}
				f = true;
			} else
			if( !SessionID.isEmpty() ) {
				TSession* p = SessionFind(SessionID);
				if( p != NULL ) {
					p->fMessage += "<usermsg msg=\""+Msg+"\" />";
				}
				f = true;
			}
		}
		if( !f ) {
			TSession* Session = fSessionPool->First();
			while( Session != NULL )
			{
				Session->fMessage += "<usermsg msg=\""+Msg+"\" />";
				Session = fSessionPool->Next(Session);
			}
		}
		return "";
	} 
	if( cm == "games" ) {
		return
			"<game id=1   title=\"roulette\" />"
			"<game id=2   title=\"slots\" />"
			"<game id=102 title=\"slots chance\" />"
			"<game id=3   title=\"videopoker\" />"
			"<game id=103 title=\"videopoker chance\" />"
			"<game id=4   title=\"keno\" />"
			"<game id=5   title=\"fruit\" />"
			"<game id=6   title=\"jokerpoker\" />"
			"<game id=106 title=\"jokerpoker chance\" />"
			"<game id=20  title=\"blackjack\" />"
			"<game id=30  title=\"oasis poker\" />"
			"<game id=31  title=\"carebbean poker\" />"
			"<game id=32  title=\"letitride poker\" />"
			"<game id=33  title=\"paigow poker\" />"
			"<game id=40  title=\"baccarat\" />"
			"<game id=41  title=\"reddog\" />"
			"<game id=42  title=\"casino war\" />"
			"<game id=60  title=\"jacksorbetter\" />"
			"<game id=63  title=\"jacksorbetter chance\" />"
			"<game id=61  title=\"deuceswild\" />"
			"<game id=64  title=\"deuceswild chance\" />"
			;
	}
	if( cm == "quit" )
		Stop();
	return "";
}

TStr TServer::RunAdm_Update(TXMLNode *r)
{
	TStr s;
	bool fCasinoControl = false;
	for( unsigned int i = 0; i < r->NodesCount(); i++ )
	{
		TXMLNode* p = r->GetNode(i);
		if( p->GetName() == "user" ) {
			unsigned int ID = p->VarValue("id").ToIntDef(0);
			if( ID > 0 && p->VarCount() >= 2 ) {
				TStr Name(p->VarName(1));
				EPaySystem System = sysAdm;
				if( p->VarCount() > 2 )
					if( p->VarValue(2) == "lotto" )
						System = sysLotto;
					else
					if( p->VarValue(2) == "parimatch" )
						System = sysParimatch;
				bool __Cash = Name.Copy(0,5) == "fcash";
				bool __Amount = Name.Copy(0,7) == "famount";
				if( __Cash || __Amount ) {
					s += "<user id=\""+TStr(ID)+"\" status=\"";
					bool f = false;
					TStr _Name,_Name2,_Value2;
					int  _Amount; 
					if( __Cash ) {
						_Name = Name.CopyAfter(5);
						_Amount = StrToCash(p->VarValue(1));
					} else {
						_Name = Name.CopyAfter(7);
						_Amount = p->VarValue(1).ToInt();
					}
					if( p->VarCount() > 2 ) {
						_Name2 = p->VarName(2);
						_Value2 = p->VarValue(2);
					}
					if( System != sysParimatch )
						f = UserCashUpdate(ID,_Name,_Amount,System,_Name2,_Value2);
					else
						f = UserCashAdd(0,sysParimatch,ID,_Amount);
					if( f )
						s += "ok";
					else
						s += "error";
					s += "\" " + Name + "=\"" + p->VarValue(1) + "\" />";
				} else
					UserUpdate(ID,Name,p->VarValue(1));
			}
		} else
		if( p->GetName() == "money" ) {
			unsigned int GameID = p->VarValue("gameid").ToIntDef(0);
			unsigned int ModeID = p->VarValue("modeid").ToIntDef(0);
			unsigned int MoneyID = p->VarValue("moneyid").ToIntDef(0);
			unsigned int TableID = p->VarValue("tableid").ToIntDef(0);
			if( !p->VarValue("bet").isEmpty() )
				if( TableID > 0 ) {
					TUser::p_CasinoControl->UpdateBet(ModeID,GameID,MoneyID,TableID,p->VarValue("bet").ToIntDef(-1));
				} else {
					TUser::p_CasinoControl->UpdateBet(ModeID,GameID,MoneyID,p->VarValue("bet").ToIntDef(-1));
				}
			else
			if( !p->VarValue("cash").isEmpty() ) {
				if( TableID > 0 ) {
					TUser::p_CasinoControl->UpdateCash(ModeID,GameID,MoneyID,TableID,p->VarValue("cash").ToDouble());
				} else {
					TUser::p_CasinoControl->UpdateCash(ModeID,GameID,MoneyID,p->VarValue("cash").ToDouble());
				}
			}
			fCasinoControl = true;
		}
	}
	if( fCasinoControl ) {
		TServerDB db;
		TUser::p_CasinoControl->Save(db.db());
	}
	return "<adm command=\"update\" status=\"ok\">"+s+"</adm>";
}

TUser* TServer::UserFind(const unsigned int ID)
{
	TLockSection _l(fUserPool);
	TUser* p = fUserPool->First();
	while( p != NULL )
	{
		if( p->ID() == ID )
			break;
		p = fUserPool->Next(p);
	}
	return p;
}

bool TServer::UserCashUpdate(
		const unsigned int ID,
		const TStr&        Name,
		const int          Amount,
		const EPaySystem   System,
		const TStr&        Name2,
		const TStr&        Value2)
{
	EUserCash UserType = cashUNDEFINED;
	if( Name == "real_max" || Name == "real_min" ) {
		TUser* User = UserFind(ID);
		if( User == NULL ) {
			TServerDB  rb;
			m_classes::TMySqlQuery query(rb.db());
			query.execSql(
				"UPDATE tbl_users SET "
				" fCash_"+Name+"='"+TStr(Amount)+"',"
				" "+Name2+"='"+Value2+"' "
				"WHERE ID="+TStr(ID));
		} else {
			if( Name == "real_max" ) {
				User->fCashRealMax = Amount;
				User->fCashRealMaxUp = Value2.ToInt();
			} else
			if( Name == "real_min" ) {
				User->fCashRealMin = Amount;
				User->fCashRealMinDn = Value2.ToInt();
			}
			User->Save();
		}
		return true;
	} else
	if( Name == "fun" )
		UserType = cashFUN;
	else
	if( Name == "real" )
		UserType = cashREAL;
	else
	if( Name == "bonus" )
		UserType = cashBONUS;
	else
	if( Name == "free" )
		UserType = cashFREE;
	else
	if( Name == "freebets" || Name == "freek" )
		UserType = cashFREEBETS;
	else
		return false;
	TUser* User = UserFind(ID);
	int UserBalance = 0;
	int UserBalanceFree = 0;
	if( User != NULL ) {
		UserBalance = User->Cash(UserType);
		if( UserType == cashREAL )
			UserBalanceFree = (int)User->fCashFree;
		if( !User->CashUpdate(UserType,Amount) )
			return false;
		User->Save();
	} else {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql(
			"SELECT "
			" fCash_fun,"
			" fCash_real, fCash_bonus, fCash_free, fCash_freeK "
			"FROM tbl_users WHERE ID="+TStr(ID)+" "
			"LIMIT 1");
		if( query.eof() )
			return false;
		TStr s;
		switch( UserType )
		{
			case cashFUN:
				s = "fCash_Fun";
				break;
			case cashREAL:
				s = "fCash_Real";
				UserBalanceFree = TStr(query.fieldByName("fCash_free")).ToInt();
				break;
			case cashBONUS:
				s = "fCash_Bonus";
				break;
			case cashFREE:
				s = "fCash_Free";
				break;
			case cashFREEBETS:
				s = "fCash_FreeK";
				break;
		}
		UserBalance = TStr(query.fieldByName(s)).ToInt();
		if( (UserBalance + Amount) < 0 )
			return false;
		query.execSql(
			"UPDATE tbl_users SET "
			 + s + "=" + s + "+("+TStr(Amount)+") "
			"WHERE ID="+TStr(ID));
	}
	switch( UserType )
	{
		case cashFUN: case cashREAL: {
			UserCashLog(
				UserType == cashREAL,
				System,
				ID,
				UserBalanceFree,
				Amount,
				UserBalance+Amount,
				0);
			break;
		}
	}
	return true;
}

void TServer::UserUpdate(
		const unsigned int ID,
		const TStr& Name,
		const TStr& Value)
{
	int  Balance = 0;
	int  BalanceFree = 0;
	int  TypeID = 0;
	TStr FName;
	TStr s;
	TUser* p = UserFind(ID);
	int v = StrToCash(Value);
	if( Name == "fCash_Real_Min" ) {
		s += "fCash_Fun=fCash_Fun+("+TStr(v)+")";
		if( p != NULL ) {
			p->fCashFun += v;
			Balance = p->fCashFun;
		}
		FName = "fCash_fun";
		TypeID = 2;
	} else
	if( Name == "fCash_Fun" ) {
		s += "fCash_Fun=fCash_Fun+("+TStr(v)+")";
		if( p != NULL ) {
			p->fCashFun += v;
			Balance = p->fCashFun;
		}
		FName = "fCash_fun";
		TypeID = 2;
	} else
	if( Name == "fCash_Real" ) {
		s += "fCash_Real=fCash_Real+("+TStr(v)+"),fCash_RealBonus=fCash_RealBonus+("+TStr(v)+")";
		if( p != NULL ) {
			if( ((int)(p->fCashReal) + v) < 0 )
				p->fCashReal = 0;
			else
				p->fCashReal += v;
			p->fCashRealBonus += v;
			if( p->fCashRealBonus < 0 )
				p->fCashRealBonus = 0;
			Balance = p->fCashReal;
			BalanceFree = (int)p->fCashFree;
		}
		FName = "fCash_Real";
		TypeID = 1;
	} else
	// Balance �� ��������
	if( Name == "fCash_Free" ) {
		s += "fCash_Free=fCash_Free+("+TStr(v)+")";
		if( p != NULL )
			p->fCashFree += v;
	} else
	if( Name == "fCash_FreeK" ) {
		s += "fCash_FreeK=fCash_FreeK+("+TStr(v)+")";
		if( p != NULL )
			p->fCashFreeBets += v;
	} else
	if( Name == "fCash_Bonus" ) {
		s += "fCash_Bonus=fCash_Bonus+("+TStr(v)+")";
		if( p != NULL )
			p->fCashBonus += v;
	}
	if( !s.isEmpty() ) {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.execSql("UPDATE tbl_users SET "+s+" WHERE ID="+ID);
		if( TypeID > 0 ) {
			if( Balance == 0 ) {
				query.openSql("SELECT "+FName+",fCash_free FROM tbl_users WHERE ID="+ID);
				if( !query.eof() ) {
					Balance = TStr(query.fields(0)).ToInt();
					BalanceFree = TStr(query.fields(1)).ToInt();
				}
				Balance += v;
			}
			UserCashLog(
				TypeID == 1,
				sysAdm,
				ID,
				BalanceFree,
				v,
				Balance,
				0);
		}
	}
}

void TServer::SessionFree(TSession* session)
{
//	TUser* p = session->fUser;
	session->Close(TSession::clNOT);
	fSessionPool->Delete(session,false);
/*
	if( p != NULL ) {
		p->Close();
		p->GameClear();
	}
*/
}

// TServer PUBLIC

bool TServer::SessionGUIDCheck(const TStr& GUID)
{
	TSession* Tmp = fSessionPool->First();
	while( Tmp != NULL )
	{
		if( Tmp->fGUID == GUID )
			return false;
		Tmp = fSessionPool->Next(Tmp);
	}
	return true;
}

TConnect* TServer::AcceptConnect(TSocket& c, const TStr& IP)
{
	print_log("AcceptConnect: "+IP);
	TLockSection _l(&l); 
	TConnect *pthread = NULL;
	fConnectPool->Insert(pthread = new TConnect(c),false);
	pthread->fIP = IP;
	pthread->fCreate = Now();
	pthread->fLast = Now();
	pthread->Create(false);
	return pthread;
}

void TServer::FreeConnect(TConnect* connect)
{
	print_log("FreeConnect: "+connect->fIP);
	TLockSection _l(&l);
	if( connect == fConnectAdmChat ) {
		fConnectAdmChat = NULL;
		TSession* Tmp = fSessionPool->First();
		while( Tmp != NULL )
		{
			Tmp->fMessage += "<bigchat status=\"off\" />";
			Tmp = fSessionPool->Next(Tmp);
		}
	}
	fConnectPool->Delete(connect,true);
}

void TServer::Timer()
{
	print_log("Timer");
	TLockSection _l(&l);
	long tm = GetTimeMs();
	TSession* Tmp = fSessionPool->First();
	while( Tmp != NULL )
	{
		TSession* Del = NULL;
/*
		if(
			Tmp->fGame != NULL &&
			Tmp->fGame->isMultiuser() &&
			Now() - Tmp->fLast > 40
		) {
			Del = Tmp;
		} else
*/
		if( Now() - Tmp->fLast > (int)fSessionTimer/1000 ) {
			Del = Tmp;
		}
		Tmp = fSessionPool->Next(Tmp);
		if( Del != NULL ) {
			fConnectPool->Lock();
			TConnect* Connect = fConnectPool->First();
			while( Connect != NULL )
			{
				if( Connect->fSession == Del ) {
// !!!
//					Connect->Close();
					Connect->fSession = NULL;
					Connect->fSessionClose = true;
					break;
				}
				Connect = fConnectPool->Next(Connect);
			}
			fConnectPool->Unlock();
			if( Del->fUser != NULL )
				if( Del->fUser->fSession == Del )
					Del->fUser->fSession = NULL;
			SessionFree(Del);
		}
	}
	TUser* User = fUserPool->First();
	while( User != NULL )
	{
		User->Save();
		TUser* Tmp = User;
		User = fUserPool->Next(User);
		if( Tmp->fSession == NULL ) {
			Tmp->OnStatusLeave();
			fUserPool->Delete(Tmp,false);
		}
	}
	fTime += GetTimeMs() - tm;
}

void TServer::Request(const TStr& s)
{
	TServerRequest r(s);
	Request(r);
}

void TServer::Request(TServerRequest& r)
{
	bool fSleep = false;
	bool fLog = true;
	TLockSection _l(&l);
	{

	if( fLogFile ) {
		TStr s_log_print;
		if( r.fSession != NULL ) {
			if( r.fSession->fUser != NULL ) {
				s_log_print += "  SessionID="+TStr(r.fSession->fUser->fLogID);
				s_log_print += "  UserID="+TStr(r.fSession->fUser->fID);
				s_log_print += "  _cash="+TStr(r.fSession->fUser->Cash());
				TTable* Table = r.fSession->fUser->CashBet();
				if( Table != NULL )
					s_log_print += "  _bet="+TStr(Table->Cash());
				Table = r.fSession->fUser->CashWin();
				if( Table != NULL )
					s_log_print += "  _win="+TStr(Table->Cash());
			}
		}
		print_log("  in "+s_log_print+"\n\n"+r.fRead.Replace("\"","").Replace("\n"," "));
	}

	long tm = GetTimeMs();
	try {
		TStr z;
		r.Init();
		TStr IP(r.fXml.VarValue("user/ip"));
		TXMLNode* root = r.fXml.FindNode("client");
		if( root != NULL ) {
			if( r.fSession != NULL )
				r.fSession = SessionFind(r.fSession);
			else {
				TStr ID(root->VarValue("session"));
				if( ID.isEmpty() ) {
					fSessionPool->Insert(r.fSession = new TSession(),false);
					r.fSession->fIP_HTTP = IP;
					r.fSession->fCreate = Now();
					r.fSession->fLast = Now();
				} else
					r.fSession = SessionFind(ID);
			}
			if( r.fSession != NULL ) {
				if( !IP.isEmpty() && r.fSession->fIP_HTTP.isEmpty() )
					r.fSession->fIP_HTTP = IP;
				if( r.fConnect != NULL && r.fSession->fIP_Connect.isEmpty() )
					r.fSession->fIP_Connect = r.fConnect->fIP;
				if(
					root->VarValue("command") == "ping" &&
					r.fSession->fGame != NULL )
				{
					// � ����� �� ������������
					fLog = false;
				}
				if( fLog ) {
					z += "  session="+TStr(r.fSession->ID());
					if( r.fSession->User() != NULL )
						z += "   user="+TStr(r.fSession->User()->fID);
					if( r.fSession->fGame != NULL ) {
						z += "   game="+TStr(r.fSession->fGame->Producer()->Name());
						if( r.fSession->fGame->isMultiuser() )
							 z += " multiuser ";
						z += "   game_id = "+TStr(r.fSession->fGame->ID());
					}
					r.fSend = RunClient(r.fSession,root);
					fSleep = true;
				}
				r.fSession->fCount++;
				r.fSession->fRead += r.fRead.Length();
				r.fSession->fSend += r.fSend.Length();
				fSendLen += r.fSend.Length();
				fReadLen += r.fRead.Length();
				r.fSession->fLast = Now();
				r.fSession->fTime += GetTimeMs() - tm;
			} else {
				r.fSend += "<server status=\"sessionlost\" />";
//				r.fClose = true;
			}
		} else {
			root = r.fXml.FindNode("adm");
			if( root != NULL ) {
				r.fSend = RunAdm(root);
				if(
					fConnectAdmChat == NULL &&
					root->VarValue("command") == "chat" )
				{
					fConnectAdmChat = r.fConnect;
					TSession* Tmp = fSessionPool->First();
					while( Tmp != NULL )
					{
						Tmp->fMessage += "<bigchat status=\"on\" />";
						Tmp = fSessionPool->Next(Tmp);
					}
				}
			} else {
				root = r.fXml.FindNode("casino");
				if( root != NULL )
					r.fSend = RunCasino(root);
				if( r.fSend.isEmpty() )
					r.fSend = "<server status=\"ok\" />";
			}
		}
		if( fLog ) {
			print("\n\n<<< read ",z,"\n",r.fRead,"\n\n>>> send ",z,"\n",r.fSend);
			TStr UserID(0);
			TStr SessionID(0);
			if(
				r.fRead != "<client command=ping />" ||
				r.fSend != "<server command=ping status=ok />"
			) {
			if( fLogBD && r.fSession != NULL && r.fSession->fUser != NULL ) {
				if( r.fSession->fUser->Status() == tREAL ) {
					UserID = r.fSession->fUser->fID;
					SessionID = r.fSession->fUser->fLogID;
					TStr fRead(r.fRead.Replace("\"","").Replace("'",""));
					TStr fSend(r.fSend.Replace("\"","").Replace("'",""));
					fSend = fSend.CopyBefore('\0');
					TStr sql;
					sql =
						"INSERT DELAYED INTO tbl_users_log ("
							" fdate,"
							" ftime,"
							" fid_user,"
							" fid_session,"
							" fread,"
							" fsend"
						") VALUES ("
							" now(),"
							" now(),"
							+UserID+","
							+SessionID+","
							"'"+StrToSQL(fRead)+"','";
					sql += StrToSQL(fSend);
					sql += "')";
					TServerDB  rb;
					m_classes::TMySqlQuery query(rb.db());
					query.execSql(sql);
				}
			}
			}
		}
	} catch (TExcept &e) {
		print("\n\n!!! error\n",e.getError());
		TStr body(e.getError());
		if( r.fSession != NULL )
			if( r.fSession->fUser != NULL ) {
				body += "\n<br>login="+r.fSession->fUser->fLogin+" ";
				body += "\n<br>id="+TStr(r.fSession->fUser->fID)+" ";
			}
		body += "\n<br>Read:\n<br>"+StrToHTML(r.fRead);
		body += "\n<br>Send:\n<br>"+StrToHTML(r.fSend);
		TMailThread* p = new TMailThread(
			"SERVER error",
			body.c_str(),
			"soft@globo.ru");
		p->Create();
		r.fSession = NULL;
		r.fClose = true;
	}
	fTime += GetTimeMs() - tm;
	fCount++;
	}
	{

	if( fLogFile ) {
		TStr s_log_print;
		if( r.fSession != NULL ) {
			if( r.fSession->fUser != NULL ) {
				s_log_print += "  SessionID="+TStr(r.fSession->fUser->fLogID);
				s_log_print += "  UserID="+TStr(r.fSession->fUser->fID);
				s_log_print += "  _cash="+TStr(r.fSession->fUser->Cash());
				TTable* Table = r.fSession->fUser->CashBet();
				if( Table != NULL )
					s_log_print += "  _bet="+TStr(Table->Cash());
				Table = r.fSession->fUser->CashWin();
				if( Table != NULL )
					s_log_print += "  _win="+TStr(Table->Cash());
			}
		}
		print_log("  out"+s_log_print+"\n\n"+r.fSend.Replace("\"","").Replace("\n"," "),"\n");
	}

	if( fSleep && fSleepMax > 0 ) {
		int fSleep = getRnd(abs(fSleepMax - fSleepMin))+fSleepMin;
		Sleep(fSleep);
	}
	}
}

// TServer PRIVATE

TSession* TServer::SessionFind(TSession* Session)
{
	TSession* Tmp = fSessionPool->First();
	while( Tmp != NULL )
	{
		if( Session == Tmp )
			break;
		Tmp = fSessionPool->Next(Tmp);
	}
	return Tmp;
}

TSession* TServer::SessionFind(const TStr& GUID)
{
	TSession* Tmp = fSessionPool->First();
	while( Tmp != NULL )
	{
		if( GUID == Tmp->GUID() )
			break;
		Tmp = fSessionPool->Next(Tmp);
	}
	return Tmp;
}

TStr TServer::RunCasino(TXMLNode* root)
{
	TStr s;
	TStr cm(root->VarValue("command"));
	/*
	for( .. )
	{
		<pay  ... />
		<user ... />
	}
	*/
	if( cm == "pay" ) {
		TXMLNode* p = root->FindNode("pay");
		if( p != NULL ) {
			s = OnCasherPay(p);
//			SendMailAdmPay("",p->ToStr()+"\n\n"+s);
		}
	}
	if( cm == "user" ) {
		TXMLNode* p = root->FindNode("user");
		if( p != NULL ) {
			if( p->VarValue("action") == "block" ) {
				TUser* user = UserFind(p->VarValue("ID").ToInt());
				if( user != NULL ) {
					user->CloseBlock();
					SendMailBlock( user->ID() );
				}
			}
		}
	}
	return s;
}

TStr TServer::RunClient(TSession* Session, TXMLNode* root)
{
	TStr Lang(root->VarValue("Lang"));
	if( !Lang.isEmpty() ) {
		if( Lang == "english" )
			Session->fLang = 2;
		else
			Session->fLang = 1;
	}
	int Num = root->VarValue("Num").ToIntDef(0);
	if( Num > 0 ) {
		if( Session->fNum < 1 )
			Session->fNum = Num;
		else
		if( Session->fNum >= Num )
			return "";
	}

	TStr RefID(root->VarValue("RefID"));
	if( Session->fRefID.isEmpty() && RefID.ToIntDef(0) == 0 ) {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		if( !Session->IP().isEmpty() ) {
			query.openSql(
			"SELECT id "
			"FROM log_url "
			"WHERE "
			" fip='"+Session->IP()+"' and fdate=now() and "
			" (TIME_TO_SEC(Now()) - TIME_TO_SEC(ftime)) < 30 "
			"ORDER BY ID DESC "
			"LIMIT 1");
			if( !query.eof() )
				RefID = query.fields(0);
		}
		if( RefID.ToIntDef(0) == 0 ) {
			query.execSql(
				"INSERT INTO log_url ("
				" fdate,ftime,ftimeload,fip,fquery "
				") VALUES ("
				" now(),now(),now(),'"+Session->IP()+"','"+StrToSQL(RefID)+"')");
			RefID = query.last_id();
		}
	}
	if( Session->fRefID.isEmpty() && !RefID.isEmpty() && RefID.ToIntDef(0) > 0 ) {
		Session->fRefID = RefID;
		TServerDB  rb;

		try {
			m_classes::TMySqlQuery query(rb.db());
			query.openSql(
				"SELECT fID,fQuery "
				"FROM log_url "
				"WHERE ID="+RefID);
			if( !query.eof() ) {
				TStr GCL(query.fieldByName("FID"));
				TStr QUERY(query.fieldByName("fQuery"));
				if( GCL.Length() < 4 && !QUERY.isEmpty() ) {
					TStr GCLID(QUERY.CopyBefore(","));
					GCL = QUERY;
					query.execSql(
						"UPDATE log_url SET "
						" fIDKey='"+StrToSQL(GCLID)+"', "
						" fID='"+StrToSQL(GCL)+"' "
						"WHERE ID="+RefID);
				}
			}
		} catch( ... ) {
		}

		HTTP_Lobby(Session->fRefID);

		try {
			m_classes::TMySqlQuery query(rb.db());
			query.execSql(
				"UPDATE log_url "
				"SET ftimeload = now() "
				"WHERE ID="+Session->fRefID);
			if( Session->fIP_HTTP.isEmpty() ) {
				query.openSql(
					"SELECT fIP "
					"FROM log_url "
					"WHERE ID="+Session->fRefID);
				if( !query.eof() )
					Session->fIP_HTTP = query.fields(0);
			}
		} catch( ... ) {
		}
	}
	TRequest  rr(
			Session,
			root,
			root->VarValue("command"),
			root->VarValue("gamenum").ToIntDef(-1));
	TRequest* r = &rr;
	if( Session->fClose != TSession::clNOT ) {
		if( Session->fClose == TSession::clBLOCK ) {
			r->AddCm("block");
			r->AddStatus("block");
			r->AddReturn("<action id=\"block\" msg=\"\" />");
		} else {
			r->AddCm("logout");
			r->AddStatus("logout");
			r->AddReturn("<action id=\"logout\" msg=\"\" />");
		}
		Session->Close(TSession::clNOT);
		Session->fClose = TSession::clNOT;
	} else {
		if( r->cm() == "ping" ) {
			r->AddStatus("ok");
		}
		if( Session->fUser != NULL && r->User() != NULL ) {
				if( r->cm() == "cashreal" ) {
					r->User()->ModeToREAL();
				} else
				if( r->cm() == "cashadd" ) {
					r->User()->OnCashAdd();
				}
				if( r->cm() == "historygame" ) {
					TStr ID(r->xml()->TXMLNodes::VarValue("Game/ID"));
					r->Add(Session->OnHistoryGame(ID),"ok");
				} else
				if( r->cm() == "historygameid" ) {
					TStr ID(r->xml()->TXMLNodes::VarValue("Game/LogID"));
					r->Add(Session->OnHistoryGameID(ID),"ok");
				} else
				if( r->cm() == "historycash" ) {
					r->Add(Session->OnHistoryCash(),"ok");
				} else
				if( r->cm() == "historycashorder" ) {
					r->Add(Session->OnHistoryCashOrder(),"ok");
				} else
				if( r->cm() == "casher" ) {
					TXMLNode* info = r->xml()->FindNode("info");
					r->AddStatus("err");
					if( info != NULL ) {
						TStr s;
						bool f = false;
//						s = Session->OnCasher(info,f);
						if( Session->fUser != NULL )
							s = OnCasher(info,Session->fUser->ID(),Session->IP(),f);
						if( f )
							r->AddStatus("ok");
						else
							r->AddStatus("error");
						r->AddReturn(s);
					}
				}
		}
		if( r->cm() == "chatclose" ) {
			Session->fChat = 0;
			r->AddStatus("ok");
		} else
		if( r->cm() == "chatopen" || r->cm() == "chat" ) {
			r->Add(Session->OnChat(root),"ok");
		} else
		if( r->cm() == "email" ) {
			if( isCommand<1,1>(Session->IP()) ) {
				TXMLNode *c = r->xml()->FindNode("user");
				bool f = false;
				if( c != NULL ) {
					TServerDB  rb;
					m_classes::TMySqlQuery q(rb.db()),q1(rb.db());
					q.openSql(
						"SELECT fLogin,fPassword,fEmail,ID "
						"FROM tbl_users "
						"WHERE fEmail LIKE \""+StrToSQL(c->VarValue("email"))+"\" "
						"ORDER by fdate_last DESC"
					);
					f =  !q.eof();
					if( f ) {
						while( !q.eof() )
						{
							f = false;
							q1.openSql("SELECT fIP FROM log_url WHERE fid_user="+TStr(q.fieldByName("ID")));
							char* c = Session->IP().c_str();
							while( !q1.eof() )
							{
								if( strncmp(q1.fields(0),c,6) == 0 ) {
									f = true;
									break;
								}
								q1.next();
							}
							if( f )
								SendMailRemind(q.fields(3));
							q.next();
						}
					}
				}
			}
			r->Add("email","ok");
		} else
		if( r->cm() == "userinfo" ) {
			if( Session->fUser != NULL ) {
				TServerSSI rs;
				r->UserToSSI(rs.ssi());
				r->Add(rs.ssi()->SSIRes("server_auth"),"ok");
			} else
				r->AddStatus("error");
		} else
		if( r->cm() == "auth" ) {
			if( isCommand<3,3>(Session->IP()) ) {
//				TXMLNode *c = r->xml()->FindNode("connect");
				Session->Close(TSession::clNOT);
				TXMLNode *p = r->xml()->FindNode("user");
				if( p != NULL ) {
					TStr Login(p->VarValue("login"));
					TStr Password(p->VarValue("password"));
					Session->fUser = UserAuth(
						Login,
						Password,
						Session);
					TServerSSI rs;
					if( Session->fUser != NULL ) {
						Session->fClose = TSession::clNOT;

						r->UserToSSI(rs.ssi());
						r->Add(rs.ssi()->SSIRes("server_auth"),"ok");

						r->AddReturn(OnCasherSetup(Session->fUser->ID()));
					} else {
						r->Add(rs.ssi()->SSIRes("server_error"),"error");
						if( Session->fClose == TSession::clBLOCK ) {
							r->AddReturn("<action id=\"block\" />");
							Session->fClose = TSession::clNOT;
						} else
						if( Session->fClose == TSession::clPASSIVE ) {
							r->AddReturn("<action id=\"passive\" />");
							Session->fClose = TSession::clNOT;
						}
					}
				}
			} else
				r->AddStatus("error");
		} else
		if( r->cm() == "register" || r->cm() == "edit" /*&& Session->fUser != NULL*/ ) {
			if( isCommand<2,4>(Session->IP()) ) {
				TXMLNode *p = r->xml()->FindNode("user");
				TStr s;
				TStr OldEMail;
				if( p != NULL ) {
					if( Session->fUser == NULL )
						Session->fUser = UserCreate(Session);
					if(r->cm() == "edit" && p->VarValue("EMail") != Session->fUser->EMail() )
						OldEMail = Session->fUser->EMail();
					s = Session->fUser->Edit(
						r->cm() == "register",
						p->VarValue("login"),
						p->VarValue("password"),
						p->VarValue("nick"),
						p->VarValue("EMail"),
						p->VarValue("Country"),
						p->VarValue("Phone"),
						p->VarValue("Lang"),
						p->VarValue("Sex"),
						p->VarValue("Icon"),
						p->VarValue("Game"),
						p->VarValue("Date"),
						p->VarValue("Type"),
						p->VarValue("Bonus"));
					r->AddReturn("<cookie value=\"Lang="+TStr(Session->fUser->fLangID)+"\" />");
				}
				TServerSSI rs;
				if( p == NULL )
					s = "  ";
				if( s.isEmpty() ) {
//					if( !Session->fUser->EMail().isEmpty() )
//						SendMailEMailChange(Session->fUser->ID(),Session->fUser->EMail());
					r->UserToSSI(rs.ssi());
					r->Add(rs.ssi()->SSIRes("server_auth"),"ok");
					if(
						r->cm() == "register" &&
						Session->fUser != NULL )
					{
						TStr IP(Session->IP());
						TStr GCL;
						TStr GCLID;
						TStr REFERRER;
						TStr QUERY;
						TServerDB  rb;
						m_classes::TMySqlQuery query(rb.db());
						if( !Session->fRefID.isEmpty() ) {
							query.execSql(
								"UPDATE log_url SET "
								" freg=1,"
								" fid_user = "+TStr(Session->fUser->fID)+", "
								" fid_session = "+TStr(Session->fUser->fLogID)+" "
								"WHERE ID="+Session->fRefID);
							query.openSql(
								"SELECT fID,fIDKey,fReferrer,fQuery "
								"FROM log_url "
								"WHERE ID="+Session->fRefID);
							if( !query.eof() ) {
								GCL = query.fieldByName("FID");
								GCLID = query.fieldByName("FIDKey");
								REFERRER = query.fieldByName("fReferrer");
								QUERY = query.fieldByName("fQuery");
								if( GCLID.isEmpty() && !QUERY.isEmpty() ) {
									GCLID = QUERY.CopyBefore(",");
									GCL = QUERY;
									query.execSql(
										"UPDATE log_url SET "
										" fIDKey='"+StrToSQL(GCLID)+"', "
										" fID='"+StrToSQL(GCL)+"' "
										"WHERE ID="+Session->fUser->fID);
								}
							}
							query.execSql(
								"UPDATE tbl_users "
								"SET fid_url='"+Session->fRefID+"' "
								"WHERE ID="+Session->fUser->fID);
						}
						query.execSql(
							"UPDATE tbl_users "
							"SET "
							"  freg_ip = '"+IP+"', "
							"  freg_gcl = '"+StrToSQL(GCL)+"', "
							"  freg_gclid = '"+StrToSQL(GCLID)+"', "
							"  freg_referrer = '"+StrToSQL(REFERRER)+"' "
							"WHERE ID="+Session->fUser->fID);

						SendMailReg(Session->fUser->fID);

						HTTP_Reg(Session->fRefID,Session->fUser->fID,QUERY);

						r->AddReturn(OnCasherSetup(Session->fUser->fID));
					}
				} else {
					rs.ssi()->SSIValue("TXT",s);
					r->Add(rs.ssi()->SSIRes("server_error"),"error");
				}
			} else
				r->AddStatus("error");
		} else
		if( r->cm() == "start" ) {
			TXMLNode *game = NULL;
			game = r->xml()->FindNode("game");
			if( game == NULL ) {
				TServerSSI rs;
				r->Add(rs.ssi()->SSIRes("lobby_start"),"ok");
			} else {
				if( Session->fUser != NULL ) {
					if( Session->fGame != NULL ) {
						IProducer::Free(Session->fGame);
						Session->fGame = NULL;
					}
					Session->fGame = IProducer::Create(game->VarValue("id").c_str(),Session);
					if( Session->fGame == NULL )
						r->AddStatus("error");
				}
			}
		}
		if( Session->fUser != NULL ) {
			if( r->cm() == "cashload" ) {
				r->AddReturn(OnCasherSetup(Session->fUser->fID));
			}
			if( r->cm() == "leavemoney" || r->cm() == "leave" /*&& r->Status() == "ok"*/ )
			{
				r->User()->OnCashAdd();
				if( Session->fGame != NULL ) {
					IProducer::Free(Session->fGame);
					Session->fGame = NULL;
				}
				TServerSSI rs;
				Session->fUser->ToSSI(rs.ssi());
				r->AddReturn(rs.ssi()->SSIRes("server_usertype"));
				r->AddStatus("ok");
			}
			if( r->cm() == "leavemoney" ) {
				Session->fUser->ModeToREAL();
				r->fcm = "leave";
			}
			if( Session->fGame != NULL )
				Session->fGame->Run(r);
			r->AddReturn(Session->fUser->Message());
			// !!!
			if( r->cm() == "news" ) {
				TXMLNode* p = r->xml()->FindNode("news");
				if( p != NULL ) {
					TServerDB  rb;
					m_classes::TMySqlQuery query(rb.db());
					query.execSql(
						"INSERT INTO news_user (fid_user,fid_news) "
						"VALUES ("+TStr(Session->fUser->fID)+","+p->VarValue("ID")+")");
				}
			}
			if( r->cm() == "auth" || r->cm() == "register" ) {
				TServerSSI rs;
				TServerDB  rb;
				m_classes::TMySqlQuery query(rb.db());
				TStr s("fid_lang=");
				s += TStr(Session->fUser->fLangID);
				query.openSql(
					"SELECT fid_news "
					"FROM news_user "
					"WHERE FID_User="+TStr(Session->fUser->fID));
				while( !query.eof() )
				{
					s += TStr(" and ID<>")+query.fields(0);
					query.next();
				}
				if( s.isEmpty() )
					s =
						"SELECT ID, fTitle, fText "
						"FROM news "
						"ORDER BY fDate DESC LIMIT 4";
				else
					s =
						"SELECT ID, fTitle, fText "
						"FROM news "
						"WHERE "+s+" "
						"ORDER BY fDate DESC LIMIT 4";
				query.openSql(s);
				s.Clear();
				while( !query.eof() )
				{
					rs.ssi()->SSIValue("ID",query.fieldByName("ID"));
					rs.ssi()->SSIValue("Title",query.fieldByName("fTitle"));
					rs.ssi()->SSIValue("Text",query.fieldByName("fText"));
					s += rs.ssi()->SSIRes("lobby_start_news");
					query.next();
				}
				r->AddReturn(s);
			}
			// !!!
		}
		// BIG CHAT
		if( r->cm() == "leave" || r->cm() == "start" ) {
			if( fConnectAdmChat != NULL )
				r->AddReturn("<bigchat status=\"on\" />");
			else
				r->AddReturn("<bigchat status=\"off\" />");
			if( r->cm() == "leave" && Session->fUser == NULL ) {
				r->AddCm("logout");
				r->AddStatus("logout");
				r->AddReturn("<action id=\"logout\" msg=\"\" />");
			}
		}
	}

	if( !Session->fMessage.isEmpty() ) {
		r->AddReturn(Session->fMessage);
		Session->fMessage.Clear();
	}

	TStr s("<server command=\""+ToLowerCase(r->cm())+"\"");
	if( Session->fGUID.isEmpty() )
		s += " session=\""+TStr(Session->GUID())+"\"";
	if( r->GameNum() > -1 )
		s += " gamenum=\""+TStr(r->GameNum())+"\"";
	if( r->isStatus() )
		s += " status=\""+r->Status()+"\"";
	if( r->Return().isEmpty() )
		s += " />";
	else
		s += ">" + r->Return() + "</server>";
	return s+'\0';
}

// TSession

const TStr& TSession::GUID()
{
	if( fGUID.isEmpty() ) {
		do {
			char tmp[33];
			randString(tmp,32,'0','9'+1);
			tmp[32] = '\0';
			fGUID = tmp;
		} while( Server()->SessionGUIDCheck(fGUID) );
	}
	return fGUID;
}

TStr TSession::OnChat(TXMLNode* p)
{
	TStr s;
	if( fGame == NULL ) {
		fChatOpen = true;
		TXMLNode* chat = p->FindNode("gchat");
		if( chat != NULL ) {
			s = chat->VarValue("cm");
			if( s == "add" )
				AddMsg(chat->VarValue("msg"),false);
			s.Clear();
		}
		for( int i = 19; i >= 0; i-- )
			if( imas - i > 0 ) {
				s += "<gchat id=\""+TStr(imas-i)+"\" msg=\"" + mas[i] + "\"";
				if( adm[i] )
					s += " adm=\"1\"";
				else
					s += " adm=\"0\"";
				s += " date=\""+date[i]+"\"";
				s += " />";
			}
	}
	fChat = Now();
	return s;
}

TStr TSession::OnHistoryGameID(TStr ID)
{
	if( fUser == NULL )
		return "";
	TServerSSI rs;
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	rs.ssi()->SSIValue("LOGID",ID);
	rs.ssi()->SSIValue("MODEID",fUser->Mode());
	query.openSql(rs.ssi()->SSIRes("LOG_GameID"));
	if( !query.eof() )
	{
		rs.ssi()->SSIValue("fDate",query.fieldByName("fDate"));
		rs.ssi()->SSIValue("fTime",query.fieldByName("fTime"));
		rs.ssi()->SSIValue("fBet",CashToUSD(query.fieldByName("fCash_Bet")));
		rs.ssi()->SSIValue("fWin",CashToUSD(query.fieldByName("fCash_Win")));
		rs.ssi()->SSIValue("fCash",CashToUSD(query.fieldByName("fCash")));
		TStr Info(query.fieldByName("fInfo"));
		rs.ssi()->SSIValue("fInfo",Info);
		return rs.ssi()->SSIRes("LOG_GameID_XML");
	}
	return "";
}

TStr TSession::OnHistoryGame(TStr ID)
{
	if( fUser == NULL )
		return "";
	TServerSSI rs;
	TStr s;

	if( ID == "luckystar" ) ID = "200"; else
	if( ID == "excalibur" ) ID = "201"; else

	if( ID == "roulette" ) ID = "1"; else
	if( ID == "roulette_2" ) ID = "50"; else
	if( ID == "keno" ) ID = "4"; else
	if( ID == "slots" ) ID = "2,102"; else
	if( ID == "videopoker" ) ID = "3,103"; else
	if( ID == "jacksorbetter" ) ID = "60,63"; else
	if( ID == "fruit" ) ID = "5"; else
	if( ID == "jokerpoker" ) ID = "6,106"; else
	if( ID == "deuceswild" ) ID = "61,64"; else
	if( ID == "oasispoker" ) ID = "30"; else
	if( ID == "caribbeanpoker" ) ID = "31"; else
	if( ID == "letitride" ) ID = "32"; else
	if( ID == "paigowpoker" ) ID = "33"; else
	if( ID == "baccarat" ) ID = "40"; else
	if( ID == "reddog" ) ID = "41"; else
	if( ID == "casinowar" ) ID = "42"; else
	if( ID == "magicforest" ) ID = "80"; else
	if( ID == "bigsam" ) ID = "81"; else
	if( ID == "blackjack" ) ID = "20";
	if( ID.ToIntDef(0) > 0 ) {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		rs.ssi()->SSIValue("USERID",fUser->ID());
		rs.ssi()->SSIValue("GAMEID",ID);
		rs.ssi()->SSIValue("MODEID",fUser->Mode());
		query.openSql(rs.ssi()->SSIRes("LOG_Game"));
		while( !query.eof() )
		{
			rs.ssi()->SSIValue("ID",query.fieldByName("ID"));
			rs.ssi()->SSIValue("fDate",query.fieldByName("fDate"));
			rs.ssi()->SSIValue("fTime",query.fieldByName("fTime"));
			rs.ssi()->SSIValue("fBet",CashToUSD(query.fieldByName("fCash_Bet")));
			rs.ssi()->SSIValue("fWin",CashToUSD(query.fieldByName("fCash_Win")));
			rs.ssi()->SSIValue("fCash",CashToUSD(query.fieldByName("fCash")));
			s += rs.ssi()->SSIRes("LOG_Game_XML");
			query.next();
		}
	}
	return s;
}

TStr TSession::OnHistoryCash()
{
	if( fUser == NULL )
		return "";
	TServerSSI rs;
	TStr s;
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	rs.ssi()->SSIValue("USERID",fUser->ID());
	rs.ssi()->SSIValue("MODEID",fUser->Mode());
	query.openSql(rs.ssi()->SSIRes("LOG_Money"));
	while( !query.eof() )
	{
		rs.ssi()->SSIValue("fDate",query.fieldByName("fDate"));
		rs.ssi()->SSIValue("fTime",query.fieldByName("fTime"));
		rs.ssi()->SSIValue("Cash",CashToUSD(query.fieldByName("fCash")));
		rs.ssi()->SSIValue("Total",CashToUSD(query.fieldByName("fTotal")));
		rs.ssi()->SSIValue("Before",CashToUSD(TStr(query.fieldByName("fTotal")).ToInt()-TStr(query.fieldByName("fCash")).ToInt()));
		rs.ssi()->SSIValue("System",query.fieldByName("fSystem"));
		s += rs.ssi()->SSIRes("LOG_Money_XML");
		query.next();
	}
	return s;
}

TStr TSession::OnHistoryCashOrder()
{
	if( fUser == NULL )
		return "";
	TServerSSI rs;
	TStr s;
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	rs.ssi()->SSIValue("USERID",fUser->ID());
	rs.ssi()->SSIValue("MODEID",fUser->Mode());
	query.openSql(rs.ssi()->SSIRes("LOG_Money_Order"));
	while( !query.eof() )
	{
		rs.ssi()->SSIValue("fDate",query.fieldByName("fDate"));
		rs.ssi()->SSIValue("fTime",query.fieldByName("fTime"));
		rs.ssi()->SSIValue("Cash",CashToUSD(query.fieldByName("fCash")));
		rs.ssi()->SSIValue("System",query.fieldByName("fSystem"));
		rs.ssi()->SSIValue("Status",query.fieldByName("fStatus"));
		s += rs.ssi()->SSIRes("LOG_Money_Order_XML");
		query.next();
	}
	return s;
}


void TSession::Close(EClose s)
{
	if( fGame != NULL ) {
		IProducer::Free(fGame);
		fGame = NULL;
	}
	if( fUser != NULL ) {
		fUser->fSession = NULL;
		fUser = NULL;
	}
	fClose = s;
}

bool TServer::UserCashUpdate(
	const unsigned int UserID,
	int                Cash)
{
	TUser* User = UserFind(UserID);
	if( User != NULL ) {
		if( Cash < 0 && (Cash + (int)User->Cash(tREAL)) < 0 )
			return false;
		User->fCashReal += Cash;
	} else {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql(
			"SELECT fCash_real "
			"FROM tbl_users "
			"WHERE ID="+TStr(UserID));
		if( Cash < 0 && (TStr(query.fields(0)).ToInt() + Cash) < 0 )
			return false;
		query.execSql(
			"UPDATE tbl_users "
			"SET fCash_real=fCash_real+("+TStr(Cash)+") "
			"WHERE ID="+TStr(UserID));
	}
	return true;
};

bool TServer::UserCashAdd(
	const unsigned int OrderID,
	EPaySystem         SystemID,
	const unsigned int UserID,
	int                Cash)
{
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	// CASH
	int  iBalance = 0;
	int  iBalanceFree = 0;
	bool iBonus = true;
	TUser* User = UserFind(UserID);
	if( User != NULL ) {
		iBalance = User->Cash(tREAL);
		iBalanceFree = (int)User->fCashFree;
		iBonus = User->fBonus != "1";
		if( User->fClose )
			return false;
	} else {
		query.openSql(
			"SELECT fCash_real, factive, finfo, fCash_free "
			"FROM tbl_users "
			"WHERE ID="+TStr(UserID));
		if( query.fields(1)[0] == '0' )
			return false;
		iBalance = TStr(query.fields(0)).ToInt();
		iBalanceFree = TStr(query.fields(3)).ToInt();
		TXML xml;
		xml.FromStr(query.fields(2));
		TXMLNode* p = xml.FindNode("user");
		if( p != NULL )
			iBonus = p->VarValue("Bonus") != "1";
	}
	if( Cash < 0 && (Cash + iBalance) < 0 )
		return false; 
	//
	UserCashLog(
		true,
		SystemID,
		UserID,
		iBalanceFree,
		Cash,
		iBalance += Cash,
		OrderID);
	// BONUS
	int CashBonus = 0;
	if( Cash > 0 && iBonus && ( 
		SystemID == sysParimatch || 
		SystemID == sysWm || 
		SystemID == sysCard || 
		SystemID == sysYandex || 
		SystemID == sysEport ||
		SystemID == sysLotto ||
		SystemID == sysRapida ||
		SystemID == sysCreditpilot ||
		SystemID == sysEgold ||
		SystemID == sysMoneyBookers ||
		SystemID == sysRupay ||
		SystemID == 62 ||
		SystemID == 63 ||
		SystemID == 64 ||
		SystemID == sysWire)
	) {
		query.openSql(
			"SELECT count(*) "
			"FROM log_pay_1 "
			"WHERE "
			" fid_user="+TStr(UserID)+" and "
			" fid_system="+TStr(sysSignup));
		if( query.eof() || query.fields(0)[0] == '0' ) {
			CashBonus += UserBonusAdd(
					OrderID,
					sysSignup,
					UserID,
					iBalanceFree,
					iBalance,
					Cash);
			iBalance += CashBonus;
		}
		CashBonus += UserBonusAdd(OrderID,SystemID,UserID,iBalanceFree,iBalance,Cash);
		if( CashBonus > 0 ) {
//			UserFreeAdd(UserID,CashBonus,(Cash+CashBonus)*5,User,query);
			UserFreeAdd(UserID,CashBonus,CashBonus*21,User,query);
		}
	}
	//
	if( User != NULL ) { 
		User->CashAdd(SystemID,Cash,CashBonus);
		User->fMessage += OnCasherSetup(UserID); 
	} else
		query.execSql(
			"UPDATE tbl_users "
			"SET fCash_real=fCash_real+("+TStr(Cash+CashBonus)+") "
			"WHERE ID="+UserID);
	return true;
}

void TServer::UserFreeAdd(
	const unsigned int     UserID,
	int                    CashFree,
	int                    CashFreeTotal,
	TUser*                 User,
	m_classes::TMySqlQuery &query)
{
/*
	double fCashFreeK = 0;
	double fCashFree = 0;
	if( User != NULL ) {
		fCashFreeK = User->fCashFreeTotal;
		fCashFree = User->fCashFree;
	} else {
		query.openSql(
			"SELECT fCash_FreeK,fCash_Free "
			"FROM tbl_users "
			"WHERE ID="+TStr(UserID));
		fCashFreeK = TStr(query.fields(0)).ToDouble();
		fCashFree = TStr(query.fields(1)).ToDouble();
	}
*/
	if( User != NULL ) {
		if( User->Cash() <= User->fCashFree ) {
			User->fCashFree = User->Cash();
			User->fCashFreeBets = User->Cash()*5;
			User->fCashRealBonus = User->Cash()*5;
		}
		User->fCashRealBonus += CashFree;
		User->fCashFree += CashFree;
		User->fCashFreeBets += CashFreeTotal;
	} else
		query.execSql(
			"UPDATE tbl_users SET "
			" fCash_RealBonus=fCash_RealBonus+"+TStr(CashFree)+","
			" fCash_Free=fCash_Free+"+TStr(CashFree)+","
			" fCash_FreeK=fCash_FreeK+"+TStr(CashFreeTotal)+" "
			"WHERE ID="+TStr(UserID));
}

unsigned int TServer::UserBonusAdd(
	const unsigned int OrderID,
	EPaySystem         SystemID,
	const unsigned int UserID,
	int                BalanceFree,
	int                Balance,
	int                Cash)
{
	int CashBonus = 0;
	if( Cash >= 1000 ) {
		switch( SystemID )
		{
			case sysSignup: {
				CashBonus = (int)(30.0*Cash/100.0);
				if( CashBonus > 30000 )
					CashBonus = 30000;
				break;
			}
			case sysCard: {
				break;
			}
			default: {
				SystemID = sysDeposit;
				int CashM = 0;
				if( Cash >= 1000 && Cash < 10000 )
					CashM = 5;
				else
				if( Cash >= 10000 && Cash < 50000 )
					CashM = 10;
				else
				if( Cash >= 50000 )
					CashM = 15;
				if( SystemID == sysRupay )
					CashM += 5;
				CashBonus = (int)(CashM*Cash/100.0);
			}
		}
		if( CashBonus > 0 ) {
			UserCashLog(
				true,
				SystemID,
				UserID,
				BalanceFree,
				CashBonus,
				Balance+CashBonus,
				OrderID);
		}
	}
	return CashBonus;
}


class TCasinoPay
{
public:
	enum EStatus {
		tsError,
		tsErrorOrderID,
		tsErrorComplite,
		tsErrorWait,
		tsOK
	};

	unsigned int fOrderID;
	TStr         fCode;
	EPayCommand  fCommand;

	int fCashUpdate;
//	int fCashBonus;

	TCasinoPay(
			const unsigned int OrderID,
			const EPayCommand  Command,
			const TStr& Code)
	:
		fOrderID(OrderID),
		fCommand(Command),
		fCode(Code)
	{}

	EStatus Run(TServer* server) {
		if( fOrderID == 0 )
			return tsErrorOrderID;
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql(
			"SELECT "
			"  log_pay_order.fid_status, "
			"  log_pay_order.fid_system, "
			"  log_pay_order.fid_user, "
			"  log_pay_order.fcash, "
			"  log_pay_order.finfo, "
			"  tbl_users.fcash_real, "
			"  tbl_users.fid_url, "
			"  const_paysystem.fhost, "
			"  const_paysystem.furl "
			"FROM log_pay_order, tbl_users, const_paysystem "
			"WHERE "
			"  log_pay_order.fid_system = const_paysystem.id and "
			"  log_pay_order.fid_user = tbl_users.id and "
			"  log_pay_order.id="+TStr(fOrderID));
		if( query.eof() )
			return tsErrorOrderID;

		EPaySystem   SystemID = (EPaySystem)TStr(query.fieldByName("fid_system")).ToInt();
		unsigned int UserID = TStr(query.fieldByName("fid_user")).ToInt();
		int          Cash = TStr(query.fieldByName("fcash")).ToInt();
		EPayStatus   oldStatus = (EPayStatus)TStr(query.fieldByName("fid_status")).ToInt();

		if( fCommand == cmWAIT ) {
			if( oldStatus == payWAIT )
				return tsErrorWait;
		} else
		if( fCommand == cmRF || fCommand == cmCB ) {
			if( !(oldStatus == payOK && Cash > 0) )
				return tsError;
		}
		if( oldStatus == payERROR )
			return tsError;
#ifndef TARGET_WIN32
		if( fCommand != cmRF && fCommand != cmCB ) {
			if( oldStatus == payOK )
				return tsError;
		}
#endif
/*
		else
		if(
			oldStatus != payPROCESSING &&
			oldStatus != payWAIT &&
			oldStatus != payADM &&
			fCommand == cmWAIT
		)
			return tsErrorComplite;
*/

		TStr PayUser;
		EPayStatus Status = oldStatus;
		{
			m_classes::TMySqlQuery q(rb.db());
			q.openSql("SELECT id,fkey FROM tbl_pay_user");
			while( !q.eof() )
			{
				if( strlen(q.fields(1)) > 0 && fCode.Pos(q.fields(1)) > -1 ) {
					PayUser = TStr(",fid_pay_user='")+TStr(q.fields(0))+"' ";
					break;
				}
				q.next();
			}
		}
		if( fCommand == cmOK && (Cash < 0 || fCashUpdate < 0) ) {
			fCommand = cmOUT;
			fCashUpdate = abs(fCashUpdate);
			if( fCashUpdate == 0 )
				fCashUpdate = abs(Cash);
		}
		if( fCommand == cmCANCEL && Cash > 0 ) {
			fCommand = cmERROR;
		}
		switch( fCommand )
		{
			case cmOK: {
				Status = payOK;
				if( SystemID == sysMnogo ) {
					query.execSql(
						"UPDATE log_pay_order SET "
						" fcash='"+TStr(fCashUpdate)+"', "
						" flast=now() "
						"WHERE ID="+TStr(fOrderID));
				} else
				if( Cash > 0 ) {
					if( strlen(query.fieldByName("fid_url")) > 3 )
						HTTP_Deposit(TStr(query.fieldByName("fid_url")), TStr(Cash));

					server->UserCashAdd(fOrderID,SystemID,UserID,Cash);
					SendMailDepositComplete(UserID,fOrderID,CashToStr(Cash));

					if( SystemID == sysCard ) {
						// CPAN
						TStr CPAN(fCode.CopyAfter("CPAN=").CopyBefore(";"));
						if( !CPAN.isEmpty() ) {
							query.openSql(
								"SELECT fNum FROM tbl_users_club "
								"WHERE fid_user="+TStr(UserID));
							if( query.eof() ) {
								query.execSql(
									"REPLACE tbl_users_club ("
									" fid_user,"
									" fdate,"
									" fnum "
									") VALUES ("
									+TStr(UserID)+","
									"now(),"
									+StrToSQL(CPAN)+
									")");
							} else {
								if( CPAN != query.fieldByName("fNum") )
									query.execSql(
										"UPDATE tbl_users_club SET "
										" fcomment=CONCAT(fcomment,'\nCPAN="+StrToSQL(CPAN)+"') "+
										"WHERE FID_User="+TStr(UserID));
							}
						}
						if( !fCode.isEmpty() )
							query.execSql(
								"UPDATE log_pay_order SET "
								" fcomment=CONCAT(fcomment,'"+StrToSQL(fCode)+"'), "
								" flast=now() "
								"WHERE ID="+TStr(fOrderID));
						// ACTION
						query.openSql(
							"SELECT Count(*) "
							"FROM log_pay_order "
							"WHERE fcash > 0 and fid_status=11 and fid_system=2 and fid_user="+TStr(UserID));
						if( query.fields(0)[0] == '0' ) {
							query.execSql(
								"INSERT INTO tbl_users_action ( "
								" fdate_create,"
								" fdate_plan,"
								" fid_user,"
								" fcomment "
								") VALUES ("
								" now(),"
								" now(),"
								+TStr(UserID)+","
								"'check transaction credit card CASH="+CashToStr(Cash)+" TR="+TStr(fOrderID)+"'"
								")");
						}
					}

				} else
					SendMailCashoutComplete(UserID,fOrderID,CashToStr(-Cash));

/*
				TUser* p = server->UserFind(UserID);
				if( p != NULL ) {
					p->ModeToREAL();
				} else {
					query.execSql(
					"UPDATE tbl_users SET "
					" fid_status = 1 "
					"WHERE id="+UserID);
				}
*/

				break;
			}
			case cmCB: case cmRF: {
				// ����� ������ ����������, �������� �� ��� OK_..
				// ������� ����� ���������� � ..
				// ���� CB (RF) �� ������������� ������ (� ��� ����� ���� ������ �� �������)
				//
				if( fCommand == cmRF )
					Status = (EPayStatus)32;
				else
					Status = payCB;
				query.execSql(
					"INSERT INTO log_pay_order ("
					"  fdate,"
					"  ftime,"
					"  fid_user,"
					"  fid_system,"
					"  fcash,"
					"  fid_status,"
					"  fid_order, "
					"  flast "
					") VALUES ("
					"  now(),"
					"  now(),"
					+TStr(UserID)+","
					+TStr(SystemID)+","
					+TStr(-Cash)+","
					+TStr(Status)+","
					+TStr(fOrderID)+","
					"now() "
					")");
				Status = oldStatus;
				break;
			}
			case cmWAIT: {
				Status = payWAIT;
				break;
			}
			case cmBLOCK: case cmCANCEL: {
				Status = payCANCEL;
				if( fCommand == cmBLOCK ) {
					TUser* User = server->UserFind(UserID);
					if( User != NULL ) {
						User->CloseBlock();
					}
					query.execSql(
						"UPDATE tbl_users SET "
						" factive = 0 "
						"WHERE ID="+TStr(UserID));
				}
				break;
			}
			case cmOUT: {
				if( server->UserCashAdd(fOrderID,SystemID,UserID,-fCashUpdate) ) {
//					if( fCashBonus > 0 )
//						server->UserUpdate(UserID,"fCash_Free",-fCashBonus);
					query.execSql(
						"UPDATE log_pay_order SET "
						" fcash=-'"+TStr(fCashUpdate)+"', "
						" flast=now() "
						"WHERE ID="+TStr(fOrderID));
					SendMailCashoutComplete(UserID,fOrderID,CashToStr(-fCashUpdate));
/*
					TPayTransaction TR;
					TR.fOrderID = fOrderID;
					if( fCashSend > 0 )
						TR.fCash = fCashSend;
					else
						TR.fCash = Cash;
					TR.fSystemID = SystemID;
					TR.fOrderValues = query.fieldByName("finfo");
					TR.fUserID = UserID;
					TR.fHost = query.fieldByName("fhost");
					TR.fURL = query.fieldByName("fURL");
					TPay* pay = new TPay( server, 1, "127.0.0.1", "80");
					pay->Create( TR );
*/
					if( SystemID == sysCard )
						Status = payExecuted;
					else
						Status = payOK;
				} else
					return tsError;
				break;
			}
			case cmERROR: {
				if( Cash > 0 ) {
					TUser* p = server->UserFind(UserID);
					if( p != NULL )
						p->CashError(SystemID,Cash);
					SendMailDepositError(UserID,fOrderID,CashToStr(Cash));
					// Comment about error
					if( !fCode.isEmpty() )
						query.execSql(
							"UPDATE log_pay_order SET "
							" fcomment=CONCAT(fcomment,'"+StrToSQL(fCode)+"'), "+
							" flast=now() "
							"WHERE ID="+TStr(fOrderID));
					//
				}
				Status = payERROR;
				break;
			}
			default:
				Status = payUNDEFINED;
		}

		if( Status != oldStatus ) {
			query.execSql(
				"UPDATE log_pay_order SET "
				" flast=now(), "
				" fdate=now(), "
				" ftime=now(), "
				" fid_status = "+TStr(Status)+" "+PayUser+" "
				"WHERE ID="+TStr(fOrderID));
		}

		return tsOK;
	}
};

TStr TServer::OnCasherPay(TXMLNode* info)
{
	TCasinoPay Result(
		TStr(info->VarValue("ID")).ToInt(),
		StrToPayCommand(info->VarValue("Status")),
		__unescape(info->VarValue("Code")));

	Result.fCashUpdate = TStr(info->VarValue("cashupdate")).ToIntDef(0);
//	Result.fCashBonus = TStr(info->VarValue("cashbonus")).ToIntDef(0);

	TCasinoPay::EStatus r = Result.Run(this);

	if( r == TCasinoPay::tsOK )
		return "ok";
	return "error";
}


struct TCasinoCasher: public TPayTransaction
{
	TServer* Server;
	TStr  keyTR;
	TStr fUserEmail;

	enum ECasherStatus {
		stNOT,
		stOK,
		stCOMPLITE,
		stERROR,

		stERROR_SYSTEM,
		stERROR_AMOUNT
	};

	TCasinoCasher(
			TServer* _Server,
			const unsigned int UserID,
			const TStr& IP,
			const TStr& EMail)
	{
		Server = _Server;
		fUserID = UserID;
		fUserIP = IP;
		fUserEmail = EMail;
	}

	ECasherStatus OnCasher(
		const TStr& System,
		const TStr& Amount,
		const TStr& Type,
		const TStr& Info,
		TUser* User)
	{
		if( fUserID == 0 )
			return stERROR;
		fCash = StrToCash(Amount);
		if( fCash < 0 )
			fCash = 0;
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		if( System == "promo" )
			fSystemID = sysPromo;
		else
		if( System == "mnogo" ) {
			fCash = 0;
			fSystemID = sysMnogo;
		} else {
			query.openSql(
				"SELECT ID,fHOST,fURL "
				"FROM const_paysystem "
				"WHERE fname='"+System+"'");
			if( query.eof() )
				return stERROR_SYSTEM;
			fSystemID = TStr(query.fields(0)).ToInt();
			if( fCash == 0 )
				return stERROR_AMOUNT;
		}

		EPayStatus OrderStatus;
		if( Type == "out" ) {
			// 2004-02-29 begin
//			if( fCash < 500 && fSystemID != sysMnogo )
//				return stERROR_AMOUNT;
			// 2004-02-29 end
			fCash = -fCash;
			OrderStatus = payADM;
		} else {
			OrderStatus = payPROCESSING;
		}
		switch( fSystemID )
		{
			case sysPromo: {
/*
				static bool f = true;
				if( f ) {
					f = false;
					fCash = 100;
					return stCOMPLITE;
				}
				f = true;
				fCash = 0;
				return stERROR;
*/
				TStr NUM(__unescape(Info.CopyAfter("num=").CopyBefore(";")).Trim());
				if( NUM.isEmpty() )
					NUM = Amount;
				query.openSql(
					"SELECT id,finfo,fser,factive,fdate_start,fdate_end "
					"FROM promo "
					"WHERE "
					" length(fser) > 0 and "
					" factive>0 and "
					" fdate_start <= Now() and "
					" fdate_end >= Now()");
				TStr PromoID;
				TXML xml;
				TStr Active;
				while( !query.eof() )
				{
					TXML x;
					x.FromStr(query.fieldByName("finfo"));
					TStr PROMO_NUM(x.VarValue("PROMO/NUM"));
					bool f = PROMO_NUM == NUM;
					if( !f )
						for( int i = 0; i < PROMO_NUM.Length(); i++ ) {
							if( PROMO_NUM[i] == '*' ) {
								f = true;
								break;
							}
							if( NUM.Length() <= i )
								break;
							if( NUM[i] != PROMO_NUM[i] )
								break;
						}
					if( f ) {
						m_classes::TMySqlQuery q(rb.db());
						q.openSql(
							"SELECT promo_users.fcash "
							"FROM promo_users, promo "
							"WHERE "
							" promo_users.fid_promo = promo.id and "
							" promo.fser='"+TStr(query.fieldByName("fser"))+"' and "
//							" fid_promo="+TStr(query.fieldByName("id"))+" and "
							" promo_users.fid_user="+fUserID);
						if( q.eof() ) {
							PromoID = query.fieldByName("id");
							Active = query.fieldByName("factive");
							xml.FromStr(x.ToStr().c_str());
							break;
						}
					}
					query.next();
				}
				if( PromoID.isEmpty() )
					return stERROR;
				fOrderID = 0;
				fCash = 0;
				int fCashBets = xml.VarValue("PROMO/BETS").ToIntDef(0);
				if( xml.VarValue("PROMO/TYPE") == "amount" ) {
					fCash = xml.VarValue("PROMO/AMOUNT_SUM").ToIntDef(0);
					fCashBets = xml.VarValue("PROMO/AMOUNT_BETS").ToIntDef(0);
				} else {
					int fCashMin = xml.VarValue("PROMO/MIN").ToIntDef(0);
					if( xml.VarValue("PROMO/TYPE") == "deposit" )
						fCashMin = xml.VarValue("PROMO/DEPOSIT_MIN").ToIntDef(0);
					else
					if( xml.VarValue("PROMO/TYPE") == "procent" )
						fCashMin = xml.VarValue("PROMO/PROCENT_MIN").ToIntDef(0);
					//
					TStr DateStart(xml.VarValue("PROMO/DATE_START"));
					if( DateStart.isEmpty() )
						DateStart = query.fieldByName("fdate_start");
					query.openSql(
						"SELECT SUM(fcash) "
						"FROM log_pay_order "
						"WHERE "
						"fid_user="+TStr(fUserID)+" and "
						"fcash > 0 and "
						"fid_status in (11) and "
						"fdate >= '"+DateStart+"'");
					if( query.eof() )
						return stERROR;
					if( TStr(query.fields(0)).ToInt() < fCashMin )
						return stERROR;
					//
					if( xml.VarValue("PROMO/TYPE") == "procent" ) {
						int fCashMax = xml.VarValue("PROMO/PROCENT_MAX").ToIntDef(0);
						fCash = TStr(query.fields(0)).ToInt();
						fCash = fCash * xml.VarValue("PROMO/PROCENT_SUM").ToIntDef(0) / 100;
						if( fCash > fCashMax )
							fCash = fCashMax;
						fCashBets = fCash * xml.VarValue("PROMO/PROCENT_BETS").ToIntDef(0) / 100;
					} else
					if( xml.VarValue("PROMO/TYPE") == "deposit" ) {
						fCashBets = xml.VarValue("PROMO/DEPOSIT_BETS").ToIntDef(0);
						fCash = xml.VarValue("PROMO/DEPOSIT_SUM").ToIntDef(0);
					}
				}
				if( fCash <= 0 )
					return stERROR;
				if( Active == "1" )
					query.execSql("UPDATE promo SET factive=0 WHERE id="+PromoID);
				query.execSql(
					"INSERT INTO promo_users (fid_promo,fid_user,fdate,ftime,fcash) "
					"VALUES ('"+PromoID+"','"+fUserID+"',Now(),Now(),"+TStr(fCash)+")");
				int CashFree = User->Cash(cashFREE);
				Server->UserCashUpdate(fUserID,fCash);
				Server->UserFreeAdd(fUserID,fCash,fCashBets,User,query);
				User->ModeToREAL();
				UserCashLog(
					true,
					sysPromo,
					fUserID,
					CashFree,
					fCash,
					User->Cash(),
					0);
				return stCOMPLITE;
			}
			case sysWire: {
				if( fCash > 0 ) {
					TStr HOLDER(Info.CopyAfter("holder=").CopyBefore(";"));
					TStr BANK(Info.CopyAfter("bank=").CopyBefore(";"));
					fOrderValues = "<HOLDER value=\""+__unescape(HOLDER).Trim()+"\" />";
					fOrderValues += "<BANK value=\""+__unescape(BANK).Trim()+"\" />";
				} else {
					TStr HOLDER(Info.CopyAfter("holder=").CopyBefore(";"));
					TStr BANK(Info.CopyAfter("bank=").CopyBefore(";"));
					TStr SWIFT(Info.CopyAfter("swift=").CopyBefore(";"));
					TStr PHONES(Info.CopyAfter("phones=").CopyBefore(";"));
					TStr PHONECODE(Info.CopyAfter("code_phones=").CopyBefore(";"));
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					TStr ADDRESS(Info.CopyAfter("address=").CopyBefore(";"));
					fOrderValues = "<HOLDER value="+__unescape(HOLDER).Trim()+" />";
					fOrderValues += "<BANK value="+__unescape(BANK).Trim()+" />";
					fOrderValues += "<SWIFT value="+__unescape(SWIFT).Trim()+" />";
					fOrderValues += "<PHONE value=\""+__unescape(PHONES).Trim()+"\" />";
					fOrderValues += "<PHONECODE value=\""+__unescape(PHONECODE).Trim()+"\" />";
					fOrderValues += "<NUM value="+__unescape(NUM).Trim()+" />";
					fOrderValues += "<ADDRESS value="+__unescape(ADDRESS).Trim()+" />";
				}
				break;
			}
			case sysEport: {
				TStr NUM1(Info.CopyAfter("num1=").CopyBefore(";"));
				TStr NUM2(Info.CopyAfter("num2=").CopyBefore(";"));
				fOrderValues = "<NUM1 value="+__unescape(NUM1).Trim()+" />";
				fOrderValues += "<NUM2 value="+__unescape(NUM2).Trim()+" />";
				break;
			}
			case sysWm: {
				TStr WMZ,WMR;
				TStr Type(Info.CopyAfter("type=").CopyBefore(";").Trim());

				// Type == "1" - ����� � ��������
				if( Type == "1" ) {
					WMR = Info.CopyAfter("num=").CopyBefore(";");
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMR'");
					if( query.eof() )
						return stERROR;
					fCurrency = 1;
					if( Type == "3" ) {
						fCurrencyCash = fCash;
						fCash = (int)(fCash / TStr(query.fields(0)).ToDouble());
					} else
						fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
				} else
				if( Type == "3" ) {
					WMR = Info.CopyAfter("num=").CopyBefore(";");
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMU'");
					if( query.eof() )
						return stERROR;
					fCurrency = 2;
					fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
				} else
				if( Type == "4" ) {
					WMR = Info.CopyAfter("num=").CopyBefore(";");
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWME'");
					if( query.eof() )
						return stERROR;
					fCurrency = 3;
					fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
				} else
					WMZ = Info.CopyAfter("num=").CopyBefore(";");
				TStr WMID(Info.CopyAfter("login=").CopyBefore(";"));
				if( !WMID.isEmpty() ) {
					fOrderValues += "<WMID value="+__unescape(WMID).Trim();
					if( fCurrency > 0 )
						fOrderValues += " Currency="+TStr(fCurrency)+" CurrencyCash="+TStr(fCurrencyCash);
					fOrderValues += " />";
				}
				if( !WMZ.isEmpty() )
					fOrderValues += "<WMZ value="+__unescape(WMZ).Trim()+" />";
				if( !WMR.isEmpty() )
					fOrderValues += "<WMR value="+__unescape(WMR).Trim()+" />";
				break;
			}
			case sysCard: {
				if( fCash > 0 ) {
					TStr CCTYPE(__unescape(Info.CopyAfter("type=").CopyBefore(";")).Trim());
					TStr CCNAME(__unescape(Info.CopyAfter("holder=").CopyBefore(";")).Trim());
					TStr _CCNUM(__unescape(Info.CopyAfter("num=").CopyBefore(";")).Trim());
					TStr CCNUM;
					for( int i = 0; i < _CCNUM.Length(); i++ )
						if( _CCNUM[i] >= '0' && _CCNUM[i] <= '9' )
							CCNUM += _CCNUM[i];
					TStr CCEXP(Info.CopyAfter("dateend=").CopyBefore(";"));
					TStr CVC2(__unescape(Info.CopyAfter("cvc2=").CopyBefore(";")).Trim());
					TStr BANK(__unescape(Info.CopyAfter("bank=").CopyBefore(";")).Trim());
					TStr CITY(__unescape(Info.CopyAfter("city=").CopyBefore(";")).Trim());
					TStr STATE(__unescape(Info.CopyAfter("state=").CopyBefore(";")).Trim());
					TStr ZIP(__unescape(Info.CopyAfter("zip=").CopyBefore(";")).Trim());
					TStr COUNTRY(__unescape(Info.CopyAfter("country=").CopyBefore(";")).Trim());
					TStr PHONE(__unescape(Info.CopyAfter("phones=").CopyBefore(";")).Trim());
					TStr PHONECODE(__unescape(Info.CopyAfter("code_phones=").CopyBefore(";")).Trim());
					TStr CLUB(__unescape(Info.CopyAfter("club_check=").CopyBefore(";")).Trim());
					if( CLUB != "2" )
						CLUB.Clear();
					else
						CLUB="yes";
					TStr ADDRESS(__unescape(Info.CopyAfter("address=").CopyBefore(";")).Trim());
					query.openSql(
						"SELECT fcode "
						"FROM const_country "
						"WHERE id='"+COUNTRY+"'");
					if( !query.eof() )
						COUNTRY = query.fields(0);
					else
						COUNTRY.Clear();
					fOrderValues =
						"<CLUB value="+CLUB+" />"
						"<IP value="+fUserIP+" />"
						"<CCTYPE value="+CCTYPE+" />"
						"<CCNUM value="+CCNUM+" />"
						"<CCNAME value=\""+CCNAME+"\" />"
						"<BANK value=\""+BANK+"\" />"
						"<CCEXP value="+CCEXP+" />"
						"<CVV2 value="+CVC2+" />"
						"<ZIP value="+ZIP+" />"
						"<COUNTRY value="+COUNTRY+" />"
						"<ADDRESS value=\""+ADDRESS+"\" />"
						"<CITY value=\""+CITY+"\" />"
						"<PHONE value=\""+PHONE+"\" />"
						"<PHONECODE value=\""+PHONECODE+"\" />"
						"<STATE value=\""+STATE+"\" />"
						"<EMAIL value=\""+fUserEmail+"\" />";
				}
				break;
			}
			case sysCreditpilot: {
				if( fCash > 0 ) {
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMR'");
					if( query.eof() )
						return stERROR;
					if( Info.CopyAfter("ser=").CopyBefore(";").Trim() != "no_ser" ) {
						fCurrencyCash = fCash;
						fCash = (int)(fCash / TStr(query.fields(0)).ToDouble());
					} else
						fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
					OrderStatus = payWAIT;
				} else {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
			case sysRapida: {
				if( fCash > 0 ) {
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMR'");
					if( query.eof() )
						return stERROR;
					fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
					OrderStatus = payWAIT;
				} else {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
			case sysFaktura: {
				if( fCash > 0 ) {
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMR'");
					if( query.eof() )
						return stERROR;
					fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
					OrderStatus = payWAIT;
				} else {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
			case sysYandex: {
				if( fCash < 0 ) {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				} else {
					query.openSql(
						"SELECT fValue  "
						"FROM setup "
						"WHERE fname='WMZtoWMR'");
					if( query.eof() )
						return stERROR;
					fCurrencyCash = (int)(fCash*TStr(query.fields(0)).ToDouble()+1.0);
					OrderStatus = payWAIT;
				}
				break;
			}
			case sysEgold: {
				if( fCash > 0 )
					OrderStatus = payWAIT;
				else
					fOrderValues = Info;
				break;
			}
			case sysMnogo: {
				if( fCash <= 0 ) {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
			case sysMoneyBookers: {
				if( fCash > 0 )
					OrderStatus = payWAIT;
				else {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
			case sysEvoCash: {
				TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
				fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				if( fCash > 0 )
					OrderStatus = payWAIT;
				break;
			}
			case sysEcoCard: {
				TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
				fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				if( fCash > 0 )
					OrderStatus = payWAIT;
				break;
			}
			case sysRupay: {
				if( fCash > 0 ) {
					OrderStatus = payWAIT;
				} else {
					TStr NUM(Info.CopyAfter("num=").CopyBefore(";"));
					fOrderValues = "<NUM value="+__unescape(NUM).Trim()+" />";
				}
				break;
			}
		}
		keyTR = CreateGUID(16);
//		if( fCash == 0 )
//			return stERROR_AMOUNT;
		query.execSql(
			"INSERT INTO log_pay_order ("
			"  fdate,"
			"  ftime,"
			"  fid_user,"
			"  fid_system,"
			"  fcash,"
			"  fInfo,"
			"  fid_status, "
			"  fkey, "
			"  flast "
			") VALUES ("
			"  now(),"
			"  now(),"
			+TStr(fUserID)+","
			+TStr(fSystemID)+","
			+TStr(fCash)+","
			"'"+StrToSQL(fOrderValues)+"',"
			+TStr(OrderStatus)+","
			"'"+keyTR+"',"
			"now()"
			")");
		fOrderID = query.last_id();

		if( fCash < 0 ) {
			SendMailCashout(fUserID,fOrderID,CashToStr(-fCash));
		}

		// UserInfo
		//  - RegIP
		//  - RegCountry
		//  - RegPhone
		//  - RegEMail

		return stOK;
	}
};

TStr TServer::OnCasher(
		TXMLNode* info,
		const unsigned int UserID,
		const TStr& IP,
		bool& f)
{
	f = false;
	if( UserID == 0 )
		return "";
	TUser* fUser = UserFind(UserID);
	if( fUser == NULL ) {
		TServerDB  rb;
		m_classes::TMySqlQuery query(rb.db());
		query.openSql("SELECT flogin,fpassword FROM tbl_users WHERE ID="+TStr(UserID));
		if( query.eof() )
			return "";
		fUser = UserAuth(query.fields(0),query.fields(1),NULL);
	}
	if( fUser == NULL || info == NULL )
		return "";

	HTTP_IP(IP);

	TCasinoCasher TR(this,fUser->ID(),IP,fUser->fEMail);

	TCasinoCasher::ECasherStatus fStatus = TR.OnCasher(
			info->VarValue("system"),
			info->VarValue("cash"),
			info->VarValue("type"),
			info->VarValue("info"),
			fUser);

	TStr sCurrency; 
	switch( fStatus )
	{
		case TCasinoCasher::stOK: {
			switch( TR.fSystemID )
			{
				case sysWm:
				case sysRapida:
				case sysCreditpilot:
				case sysYandex:
				case sysFaktura:
					if( TR.fCurrencyCash > 0 )
						sCurrency = " amount_r=\""+TStr(TR.fCurrencyCash)+"\" ";
					break;
			}
	/*
			TR.fUserInfo =
				"<user "+TR.fUserInfo+" "
				" country="+TStr(fUser->fCountryID)+
				" phone="+fUser->fPhone+
				" email="+fUser->fEMail+
				" />";
	*/
			
			if( TR.fCash > 0 ) {
				TStr cm;
				switch( TR.fSystemID )
				{
					case sysWm:
						cm = "wm";
						break;
					case sysCard:
						cm = "card";
						break;
					case sysEport:
						cm = "eport";
						break;
				}
				if( !cm.isEmpty() ) {
					TServerSSI rs;
					TPay* pay = new TPay(
						this,
						rs.ssi()->SSIResDef("pay_cgi_ip_"+cm,"127.0.0.1"),
						rs.ssi()->SSIResDef("pay_cgi_port_"+cm,"80"),
						rs.ssi()->SSIResDef("pay_cgi_path_"+cm,"/"),
						rs.ssi()->SSIResDef("pay_cgi_host_"+cm,""));
					pay->Create( TR );
				}
			} else
				TR.fCash = -TR.fCash;
			if( fUser->Mode() != 1 )
				fUser->ModeToREAL();
			f = true;
			break;
		}
		case TCasinoCasher::stCOMPLITE:
			if( fUser->Mode() != 1 )
				fUser->ModeToREAL();
			f = true;
			break;
		default:
			TR.fOrderID = 0;
			f = false;
			break;
	}

	return "<pay id=\""+TStr(TR.fOrderID)+"\" amount=\""+TStr(TR.fCash)+"\" "+sCurrency+" keytr=\""+TR.keyTR+"\" />";
}


TStr TServer::OnCasherSetup(const TStr& UserID)
{
	TStr s;
	TServerDB  rb;
	m_classes::TMySqlQuery query(rb.db());
	query.openSql("SELECT Count(*) FROM log_pay_1 WHERE FID_User="+TStr(UserID));
	bool fDeposit = (query.fields(0)[0] != '0');

	query.openSql(
		"SELECT "
		"  log_pay_order.finfo, "
		"  log_pay_order.fid_system, "
		"  log_pay_order.fcash, "
		"  log_pay_order.fid_status, "
		"  tbl_users.fcash_real "
		"FROM "
		"  (tbl_users "
		"  INNER JOIN log_pay_1 "
		"  ON tbl_users.ID = log_pay_1.fid_user) "
		"  INNER JOIN log_pay_order  "
		"  ON log_pay_1.FID_Order = log_pay_order.ID "
		"WHERE "
		"  log_pay_1.FID_User="+UserID+" "
		"ORDER BY log_pay_order.ID DESC");
	TStr fWmIn;
	TStr fWmOut;
	TStr fCardOut;
	TStr fEpOut("0");
	TStr fOrderOut("0");

	bool f = (UserID == "1" || UserID == "2" || UserID == "3" );

	if( !query.eof() && query.fieldByNameAsInt("fCash_real") > 0 ) {
		fOrderOut = "1";
	}
	while( !query.eof() )
	{
		TStr       Info = query.fields(0);
		int        Cash = TStr(query.fields(2)).ToInt();
		EPaySystem SystemID = (EPaySystem)TStr(query.fields(1)).ToInt();
		switch( SystemID )
		{
			case sysEport:
				fEpOut = "1";
				break;
			case sysWm:
				if( Cash > 0 && fWmIn.isEmpty() ) {
					fWmIn = TXML(Info.c_str()).VarValue("WMID/value");
				} else
				if( Cash < 0 && fWmOut.isEmpty() ) {
					fWmOut = TXML(Info.c_str()).VarValue("WMZ/value");
				}
				break;
			case sysCard:
				if( s.isEmpty() ) {
					TXML xml(Info.c_str());
					TStr COUNTRY(xml.VarValue("country/value"));
					m_classes::TMySqlQuery q(rb.db());
					q.openSql("SELECT ID FROM const_country WHERE fcode='"+COUNTRY+"'");
					if( !q.eof() )
						COUNTRY = q.fields(0);
					s +=
						TStr("<casher_card")+
						" city=\""+xml.VarValue("city/value")+"\""
						" state=\""+xml.VarValue("state/value")+"\""
						" zip=\""+xml.VarValue("zip/value")+"\""
						" country=\""+COUNTRY+"\""
						" address=\""+xml.VarValue("address/value")+"\""
						" phones=\""+xml.VarValue("phone/value")+"\""
						" code_phones=\""+xml.VarValue("phonecode/value")+"\""
						" />";
				}
				if( TStr(query.fields(3)).ToInt() == payOK ) {
					fCardOut = "1";
				}
				break;
		}
		query.next();
	}

	s += "<casher_mode type=\"ecocard_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"faktura_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"parimatch_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"sberbank_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"phone_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"promo_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"card_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"order_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"webmoney_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"eport_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"creditpilot_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"egold_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"moneybookers_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"Rupay_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"rapida_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"evocash_in\"  mode=\"1\" />";

	s += "<casher_mode type=\"yandex_in\"  mode=\"1\" />";
	s += "<casher_mode type=\"yandexmoney_in\"  mode=\"1\" />";

	if( f || fDeposit ) {
		s += "<casher_mode type=\"faktura_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"ecocard_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"evocash_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"moneybookers_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"egold_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"yandex_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"yandexmoney_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"order_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"creditpilot_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"rupay_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"rapida_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"mnogo_out\"  mode=\"1\" />";
		s += "<casher_mode type=\"order_out\"  mode=\""+fOrderOut+"\" />";
	} 

	if( fCardOut.isEmpty() ) {
		if( f )
			s += "<casher_mode type=\"card_out\" mode=\"1\" />";
		else
			s += "<casher_mode type=\"card_out\" mode=\"0\" />";
	} else {
		s += "<casher_mode type=\"card_out\" mode=\"1\" />";
	}

	if( f || fDeposit )
		s += "<casher_mode type=\"eport_out\" mode=\"1\" />";
	else
		s += "<casher_mode type=\"eport_out\" mode=\""+fEpOut+"\" />";

	if( f || !fWmOut.isEmpty() || !fWmIn.isEmpty() || fDeposit ) {
		s += "<casher_mode type=\"webmoney_out\" mode=\"1\" />";
		TStr w;
		if( !fWmOut.isEmpty() )
			w += " Num=\""+fWmOut+"\"";
		if( !fWmIn.isEmpty() )
			w += " WM=\""+fWmIn+"\"";
		s += "<casher_webmoney"+w+" />";
	} else
		s += "<casher_mode type=\"webmoney_out\" mode=\"0\" />";

	return s;
}


