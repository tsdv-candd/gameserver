#ifndef mGameH
#define mGameH

#include "m_string.h"
#include "thread/m_thread.h"
#include "utils/m_object.h"
#include "xml/m_xmlparser.h"
#include "ssi/mf_ssi.h"
#include "mCommon.h"
#include <time.h>

// -- TRequest

class TSession;
class TUser;

class TRequest
{
friend class TServer;
private:
	TSession    *fSession;
	TXMLNode    *fNode;
	TStr        fcm;
	int         fcmGameNum;
	TStr        fStatus;
	TStr        fReturn;
	int         fGameNum;
public:
	TRequest(TSession *Session, TXMLNode *Node, const TStr &cm, const int cmGameNum)
	:fSession(Session), fNode(Node), fcm(cm), fcmGameNum(cmGameNum), fGameNum(-1) {}

	TSession*     Session() { return fSession; }
	TUser*        User();
	TXMLNode*     xml() { return fNode; }
	const TStr&   cm() { return fcm; }
	int           cmGameNum() { return fcmGameNum; }
	const TStr&   Return() { return fReturn; }
	const TStr&   Status() { if(fStatus.isEmpty()) return fcm; return fStatus; }
	int           GameNum() { return fGameNum; }

	bool          isStatus() { return !fStatus.isEmpty(); }

	void          Add(const TStr& sr, const TStr& st) { AddReturn(sr); AddStatus(st); }
	void          AddCm(const TStr& s) { fcm = s; }
	void          AddReturn(const TStr& s) { fReturn += s; }
	void          AddStatus(const TStr& s) { fStatus = s; }
	void          AddGameNum(const int s) { fGameNum = s; }

	void          UserToSSI(m_ssi::TSSIControl *ssi);
};



// -- Game

class IConnect: virtual public m_object::TObject
{
friend class IProducer;
private:
	unsigned int LogID;
	//
	IProducer* fProducer;
	TSession*  fSession;
protected:
	virtual bool isLeave(TUser*) { return true; }
	virtual void OnStart(TUser*,const int,const bool) {}
	virtual void OnLeave(TUser*) {}
public:
	IConnect(): fProducer(NULL),fSession(NULL) {}

	virtual bool isMultiuser() { return false; }

	TStr Name();

	IProducer* Producer() { return fProducer; }
	TSession*  Session()  { return fSession; }

#ifdef TARGET_VC
	unsigned int ID() { return TObject::ID(); }
#else
	unsigned int ID() { return m_object::TObject::ID(); }
#endif

	unsigned int GameID();

	TUser* User();

	virtual void Run(TRequest *request);
};

class IProducer
{
private:
//	static       TLock l;
	static       m_object::TVector<IProducer> *fProducerList;
	char*        fName;
	unsigned int fID;
protected:
	virtual IConnect* OnCreate() = NULL;
	virtual void      OnFree(IConnect *) {}
public:
	IProducer(const unsigned int ID, const char *Name);
	virtual ~IProducer();

	const char*       Name() { return fName; }
	static IConnect*  Create(const char *, TSession *);
	static void       Free(IConnect *);

	unsigned int      GameID() { return fID; }

	static unsigned int IProducer::GameList( unsigned int* mas, unsigned int size);
};

class TGameRequest: public IProducer, public IConnect
{
protected:
	TGameRequest(const unsigned int ID, const char* name): IProducer(ID,name) {}
	IConnect* OnCreate() { return this; }
};



// -- Session

class ISession: public IConnect
{
friend class ISessionProducer;
private:
	unsigned int fRef;
protected:
	unsigned int Ref() { return fRef; }

	virtual bool isCreate() { return Ref() == 0; } // false
	virtual bool isFree()   { return Ref() == 0; } // true
public:
	ISession(): fRef(0) {}
};

class ISessionProducer: public IProducer
{
private:
	m_object::TObjectPool<ISession> SessionPool;
protected:
	IConnect* OnCreate();
	void      OnFree(IConnect *p);

	virtual ISession* OnCreateSession() = NULL;
	virtual void      OnFreeSession(ISession*) {}
public:
	ISessionProducer(const unsigned int ID, const char *name, const unsigned int size)
	: IProducer(ID,name), SessionPool(size,true)
	{}
};



// -- Player

class IPlayerRoom;
class IPlayerProducer;

class IPlayer: public ISession
{
friend class IPlayerProducer;
private:
	IPlayerRoom* fRoom;
	TStr         fMessage;
	TStr         fMessageNew;
	int          fGameNum;
	int          fAction;
	//
	void Run(TRequest *request);
protected:
	bool isMultiuser();
	bool isLeave(TUser*);
	void OnStart(TUser*,const int,const bool);
	void OnLeave(TUser*);
	IPlayerProducer* Producer();
public:
	int  fTag;

	int  Action();
	void ActionUpdate(const int Action);

	bool isGame();

	IPlayer(): fRoom(NULL), fAction(0), fGameNum(0) {}

	void AddMsg(const TStr& Message);
};

class IPlayerRoom: /*public TLock,*/ public m_object::TObject
{
friend class IPlayerProducer;
friend class IPlayer;
private:
	m_object::TVector<IPlayer> fPlayerList;
	//
	unsigned int fTable;
	bool         fPrivate;
	TStr         fChat1;
	unsigned int fUserID1;
	TStr         fChat2;
	unsigned int fUserID2;
	int          fGameNum;
	time_t       fTimer;
	unsigned int fMode;
	//
	unsigned int FindIndex(IPlayer *);
	//
	void PlayerAdd(IPlayer *);
	void PlayerDel(IPlayer *);
	void Chat(IPlayer*, const TStr& str);
	void Init(const unsigned int Table) { fTable = Table; }
	bool isFree() { return PlayerCount() == 0; }
protected:

	virtual bool isLeave(TUser* p) { return true; }

	m_ssi::TSSIControl *ssi;

	void          GameNext() { fGameNum++; }
	unsigned int  Mode() { return fMode; }
	void          ModeClear() { fMode = 0; }
	void          ModeNext() { fMode++; }
	bool          isTimerStart();
	bool          isTimer(const unsigned int Time = 60);
	unsigned int  Timer() { int c = (int)(time(NULL) - fTimer); if( c < 0 ) c = 0; return c; }
	void          TimerStart(const unsigned int Time = 0);
	void          TimerUpdate(const unsigned int Time = 50);
	TStr          TimerToSSI(m_ssi::TSSIControl *ssi, const unsigned int Time = 60);
	void          TimerStop();

	bool          Private() { return fPrivate; }

	unsigned int  Table() { return fTable; }
	virtual bool  isSelect(const unsigned int Table) { return fTable == Table && fPlayerList.Count() < fPlayerList.Size(); }
	virtual void  OnPlayerAdd(IPlayer *) {}
	virtual void  OnPlayerDel(IPlayer *) {}

	IPlayer*      Player(const unsigned int i) { return fPlayerList.Get(i); }
	unsigned int  PlayerCount() { return fPlayerList.Count(); }
	void          PlayerMessage(const TStr& Message, IPlayer* ignore);
public:
	int          GameNum() { return fGameNum; }
	TStr virtual ModeAsStr() { return fMode; }

	IPlayerRoom(const unsigned int Size)
	: fPlayerList(Size,false), fTable(0), fGameNum(0), fTimer(0), fMode(0) {}

	void virtual OnRequest(TRequest *r, IPlayer* Player);
};

class IPlayerProducer: public ISessionProducer
{
private:
	m_object::TObjectPool<IPlayerRoom> fRoomList;
protected:
	ISession* OnCreateSession() { return OnCreatePlayer(); }
	void      OnFreeSession(ISession* session) { FreeRoom((IPlayer*)session); }
protected:
	virtual IPlayerRoom* OnCreateRoom(const unsigned int,const bool) = NULL;
	virtual IPlayer*     OnCreatePlayer() { return new IPlayer(); };
public:
	IPlayerProducer(const unsigned int ID, const char *name, const unsigned int size)
	: ISessionProducer(ID,name,size), fRoomList(size,true) {}

	void   CreateRoom(IPlayer*,const unsigned int,const bool);
	void   FreeRoom(IPlayer *player);
};

#endif
