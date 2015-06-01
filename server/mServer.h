#ifndef mServerH
#define mServerH

#include "utils/m_object.h"
#include "utils/m_datetime.h"
#include "socket/m_socket.h"
#include "xml/m_xmlparser.h"

#include "common/mPay.h"

#include "m_storage.h"
#include "mCash.h"
#include <time.h>

// 

class IServer
{
friend class TServer;
private:
	static TServer* fServer;
protected:
	TServer* Server() { return fServer; }
public:
	virtual ~IServer() {}
};

// TSession

class TUser;
class IConnect;

class TSession: public IServer, public m_object::TObject
{
friend class TServer;
friend class TUser;
public:
	enum EClose { clNOT, clCLOSE, clBLOCK, clPASSIVE };
private:
	int   idcount;
	int   id[50];
	TStr  date[50];
	TStr  mas[50];
	bool  adm[50];
	int   imas;

	bool       fChatOpen;
	long       fChat;
	long       fCreate;
	long       fLast;
	int        fCount, fTime, fRead, fSend;

	int        fNum;
	TStr       fRefID;
	TStr       fGUID;
	TUser*     fUser;
	IConnect*  fGame;
	EClose     fClose;
	int        fLang;

	TStr       fMessage;

	void AddMsg(const TStr& s, const bool a) {
			for( unsigned int i = 19; i > 0; i-- ) {
				id[i] = id[i-1];
				date[i] = date[i-1];
				mas[i] = mas[i-1];
				adm[i] = adm[i-1];
			}
			id[0] = idcount++;
			mas[0] = s;
			date[0] = DateTimeToStr(Now(),"%Y-%m-%d %H:%M:%S");
			adm[0] = a;
			imas++;
	}

//	TStr OnCasher(TXMLNode* p, bool& f);
	TStr OnChat(TXMLNode* p);
	TStr OnHistoryGame(TStr ID);
	TStr OnHistoryGameID(TStr ID);
	TStr OnHistoryCash();
	TStr OnHistoryCashOrder();

	void  Close(EClose);
	const TStr& GUID();
public:
	TStr       fIP_Connect;
	TStr       fIP_HTTP;
	const TStr& IP() {
		if( !fIP_Connect.isEmpty() )
			return fIP_Connect;
		return fIP_HTTP;
	}
public:
	TSession():
		fLast(0), fChat(0), fLang(1), idcount(0), fChatOpen(false), 
		imas(0),
		fNum(0),
		fUser(NULL), fGame(NULL), fClose(clNOT) 
	{
		fCreate = fLast = (long)time(NULL);
		fTime = fCount = fRead = fSend = 0;
	}
	virtual ~TSession() {}

	TUser* User()     { return fUser; }
};

// TConnect

class TConnect: public TThread, public IServer, public m_object::TObject
{
friend class TServer;
private:
	TStr       s;
	TStr       fIP;
	long       fCreate;
	long       fLast;
	int        fCount, fRead, fSend;
	TSession*  fSession;
	TSocket    fSocket;
	bool       fSessionClose;
protected:
public:
	static unsigned int TimeOut;

	TConnect(const TSocket &c)
	: fSocket(c), fSession(NULL), fSessionClose(false)
	{
		fCreate = fLast = (long)time(NULL);
		fCount = fRead = fSend = 0;
	}

	~TConnect() {
	}

	void Run();

	void Close();
};

// TServer

class TServerRequest
{
friend class TServer; 
private:
	TStr      fRead;
	TStr      fSend;
	TConnect* fConnect;
	TSession* fSession;
	TXML      fXml;
	bool      fClose;
public:
	TServerRequest(const TStr& Read, TSession* Session = NULL, TConnect* Connect = NULL):
		fRead(Read), fSession(Session), fConnect(Connect), fClose(false)
	{}

	void        Init() { fXml.FromStr(fRead.c_str()); }
	const TStr& Send() { return fSend; }

	TSession*   Session() { return fSession; }

	bool isClose() { return fClose; }
};

class TServer: public TThread
{
friend class TServerSSI;
friend class TServerDB;
friend class TCasinoPay;
private:
	TLock l;

	unsigned int fSessionTimer;
	unsigned int fSleepMin;
	unsigned int fSleepMax;
	long fCreate;
	int  fCount;
	int  fReadLen;
	int  fSendLen;
	int  fTime;
	bool fExit, fOpen;

	TSSIStorage    *ssiStorage;
	TMySqlStorage  *dbStorage;

	TObjectPool<TConnect> *fConnectPool;
	TObjectPool<TSession> *fSessionPool;
	TObjectPool<TUser>    *fUserPool;

	TCasinoControl        fCasinoControl;

	TConnect* fConnectAdmChat;

	TStr RunCasino(TXMLNode *r);
	TStr RunAdm(TXMLNode *r);
	TStr RunAdm_Update(TXMLNode *r);
	void UserUpdate(const unsigned int ID, const TStr& Name, const TStr& Value);

	TUser*    UserFind(const unsigned int ID);
	TUser*    UserCreate(TSession* Session);
	TUser*    UserAuth(const TStr&, const TStr&, TSession* Session);
//	void      UserFree(TUser*);

	void      SessionFree(TSession* session);
	TSession* SessionFind(TSession* Session);
	TSession* SessionFind(const TStr& GUID);
	TStr      RunClient(TSession* Session, TXMLNode* root);

	void      Timer();

	void      Stop();

	unsigned int UserBonusAdd(
		const unsigned int OrderID,
		EPaySystem         SystemID,
		const unsigned int UserID,
		int                BalanceFree,
		int                Balance,
		int                Cash);

	TStr      OnCasherSetup(const TStr& UserID);
	TStr      OnCasherPay(TXMLNode* p);

	TStr      OnCasher(TXMLNode* p, const unsigned int UserID, const TStr& IP, bool& f);

public:
	bool UserCashUpdate(
			const unsigned int ID,
			const TStr&        Name,
			const int          Amount,
			const EPaySystem   System,
			const TStr&        Name2 = "",
			const TStr&        Value2 = "");
	void UserFreeAdd(
		const unsigned int UserID,
		int                CashFree,
		int                CashFreeTotal,
		TUser*             User,
		m_classes::TMySqlQuery &query);
	bool SessionGUIDCheck(const TStr& GUID);
	bool UserCashUpdate(
		const unsigned int UserID,
		int                Cash);
	bool UserCashAdd(
		const unsigned int OrderID,
		EPaySystem         SystemID,
		const unsigned int UserID,
		int                Cash);

	TServer();
	~TServer();

	bool isOpen() { return fOpen; }

	void Create(const bool f) { fOpen = true; TThread::Create(f); }

	void Run();

	TConnect* AcceptConnect(TSocket& c, const TStr& IP);
	void      FreeConnect(TConnect*);

	void      Request(TServerRequest &r);
	void      Request(const TStr &r);
};

class TServerSSI: public IServer
{
private:
	m_ssi::TSSIControl *fSSI;
public:
	TServerSSI() { fSSI = Server()->ssiStorage->Create(); }
	~TServerSSI() { Server()->ssiStorage->Free(fSSI); }

	m_ssi::TSSIControl *ssi() { return fSSI; }
};

class TServerDB: public IServer
{
private:
	m_classes::TMySqlConnect* fDB;
public:
	TServerDB() { fDB = Server()->dbStorage->Create(); }
	~TServerDB() { Server()->dbStorage->Free(fDB); }

	m_classes::TMySqlConnect* db() { return fDB; }
};

#endif
