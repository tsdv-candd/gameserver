#include "utils/m_utils.h"

/*
replace into money_games (fid_money,fid_mode,fid_game,fbet) values (1,1,120,0);
replace into money_games (fid_money,fid_mode,fid_game,fbet) values (1,2,120,0);
replace into money_games (fid_money,fid_mode,fid_game,fbet) values (2,1,120,100);
replace into money_games (fid_money,fid_mode,fid_game,fbet) values (2,2,120,100);
*/

#include "mGame.h"
#include "mUser.h"
#include "mServer.h"

m_object::TVector<IProducer>* IProducer::fProducerList;
//TLock IProducer::l;


unsigned int IProducer::GameList(
	unsigned int* mas,
	unsigned int  size)
{
	unsigned int i = fProducerList->Count();
	if( i < size )
		size = i;
	else
		i = size - 1;
	while( i-- > 0 )
		mas[i] = fProducerList->Get(i)->fID;
	return size;
}



// Request


TUser* TRequest::User()
{
	return Session()->User();
}


void TRequest::UserToSSI(m_ssi::TSSIControl *ssi)
{
	if( User() == NULL ) {
		ssi->SSIValue("TYPE","");
		return;
	}
	User()->ToSSI(ssi);
}



// Game


TStr IConnect::Name()
{
	return Producer()->Name();
}


unsigned int IConnect::GameID()
{
	return fProducer->GameID();
}

TUser* IConnect::User()
{
	if( Session()->User() == NULL )
		throw TExcept("IConnect::User()");
	return Session()->User();
}

/*
unsigned int IConnect::ID()
{
	return Session()->ID();
}
*/


void IConnect::Run(TRequest *r)
{
	if( r->User() == NULL )
		return;
	// start
	if( r->cm() == "start" ) {
		bool fPrivate = false;
		int  fTable = 1;
		TXMLNode *game = r->xml()->FindNode("game");
		TServerSSI rs;
		if( game != NULL && r->User()->Status() != tGIFT ) {
			fPrivate = game->VarValue("privacy") == "private";
			fTable = game->VarValue("table").ToIntDef(1);
		}
		rs.ssi()->SSIBlock("&isGIFT",r->User()->Status() == tGIFT,false);
		rs.ssi()->SSIBlock("&notGIFT",r->User()->Status() != tGIFT,false);
		OnStart(r->User(),fTable,fPrivate);
		r->UserToSSI(rs.ssi());
		TStr s(rs.ssi()->SSIRes("server_user"));
		rs.ssi()->SSIValue("STREAM_TABLE",rs.ssi()->SSIResDef(TStr(Producer()->Name())+"_table_"+fTable,""));
		rs.ssi()->SSIValue("TABLE",fTable);
		if( fPrivate )
			rs.ssi()->SSIValue("PRIVACY","private");
		else
			rs.ssi()->SSIValue("PRIVACY","public");
		s += rs.ssi()->SSIResDef("server_gamestart","");
		s = rs.ssi()->SSIResDef(TStr(Producer()->Name())+"_start","") + s;
		r->User()->ToSSI(rs.ssi());
		r->AddReturn(rs.ssi()->SSIRes("server_user"));
		r->Add(s,"ok");
	} else
	// leave
	if( r->cm() == "leave" || r->cm() == "leavemoney" ) {
		if( isLeave(r->User()) ) {
			TServerSSI rs;
			OnLeave(r->User());
			r->AddStatus("ok");
		} else
			r->AddStatus("err");
	}
}


IProducer::IProducer(const unsigned int ID, const char *Name)
{
//	TLockSection _l(&l);
	if( fProducerList == NULL )
		fProducerList = new m_object::TVector<IProducer>(100,false);
	fID = ID;
	fName = new char[strlen(Name)+1];
	strcpy(fName,Name);
	fProducerList->Append(this);
}


IProducer::~IProducer()
{
//	TLockSection _l(&l);
	if( fProducerList != NULL ) {
		delete fProducerList;
		fProducerList = NULL;
	}
	delete[] fName;
}


IConnect* IProducer::Create(const char *Name, TSession *Session)
{
	if( fProducerList == NULL )
		return NULL;
	for( unsigned int i = 0; i < fProducerList->Count(); i++ )
	{
		if( strrcmp(fProducerList->Get(i)->Name(),Name) == 0 ) {
			IConnect *p = fProducerList->Get(i)->OnCreate();
			if( p != NULL ) {
				p->fProducer = fProducerList->Get(i);
				p->fSession = Session;
			}
			return p;
		}
	}
	return NULL;
}


void IProducer::Free(IConnect *p)
{
	if( p != NULL )
		p->fProducer->OnFree(p);
}



// Session


IConnect* ISessionProducer::OnCreate()
{
	TLockSection _l(&SessionPool);
	ISession* session = NULL;
	session = SessionPool.First();
	while( session != NULL )
	{
		if( session->isCreate() )
			break;
		session = SessionPool.Next(session);
	}
	if( session == NULL ) {
		session = OnCreateSession();
		session->fRef++;
		SessionPool.Insert(session,false);
	}
	return session;
}


void ISessionProducer::OnFree(IConnect *game)
{
	TLockSection _l(&SessionPool);
	ISession *session = SessionPool.Get(game);
	session->fRef--;
	if( session->isFree() ) {
		OnFreeSession(session);
		SessionPool.Delete(session,false);
	}
}



// Player


IPlayerProducer* IPlayer::Producer()
{
	return (IPlayerProducer*)ISession::Producer();
}


void IPlayer::OnStart(TUser*, const int fTable, const bool fPrivate)
{
	Producer()->FreeRoom(this);
	fRoom = NULL;
	Producer()->CreateRoom(this,fTable,fPrivate);
}


void IPlayer::OnLeave(TUser*)
{
	Producer()->FreeRoom(this);
	fRoom = NULL;
}


void IPlayer::AddMsg(const TStr& Message)
{
	if( fRoom->fGameNum == fGameNum )
		fMessage += Message;
	else
		fMessageNew += Message;
}


int IPlayer::Action()
{
	if( fRoom->fGameNum == fGameNum )
		return fAction;
	return 0;
}


void IPlayer::ActionUpdate(const int Action)
{
	if( fRoom->fGameNum == fGameNum )
		fAction = Action;
	else
		fAction = 0;
}


bool IPlayer::isGame()
{
	return fRoom->fGameNum == fGameNum;
}


bool IPlayer::isMultiuser()
{
	return !fRoom->Private();
}


bool IPlayer::isLeave(TUser* p)
{
	if( fRoom != NULL )
		return fRoom->isLeave(p);
	return true;
}

void IPlayer::Run(TRequest *r)
{
	//
	ISession::Run(r);
	if( fRoom != NULL ) {
//		TLockSection _l(fRoom);
		if( r->cmGameNum() == fRoom->GameNum() || r->cmGameNum() == -1 )
			fRoom->OnRequest(r,this);
		// chat
		for( unsigned int i = 0; i < r->xml()->NodesCount(); i++ )
			if( r->xml()->GetNode(i)->GetName() == "chat" )
				fRoom->Chat(this,r->xml()->GetNode(i)->GetText());
		//
		r->AddReturn(fMessage);
		if( fRoom->fGameNum == fGameNum ) {
//      if( !r->isStatus() )
//        r->AddStatus(fRoom->ModeAsStr());
			fMessage.Clear();
		} else {
			fAction = 0;
			r->AddStatus("ok");
			fGameNum = fRoom->fGameNum;
			fMessage = fMessageNew;
			fMessageNew.Clear();
		}
		r->AddGameNum(fRoom->GameNum());
	}
}


void IPlayerRoom::OnRequest(TRequest*, IPlayer* Player)
{
	if( isTimerStart() ) {
		TServerSSI rs;
		Player->AddMsg(TimerToSSI(rs.ssi()));
	}
}


void IPlayerProducer::CreateRoom(
	IPlayer *player,
	const unsigned int Table,
	const bool Private)
{
	TLockSection lRoomList(&fRoomList);
	IPlayerRoom *room = NULL;
	if( !Private ) {
		room = fRoomList.First();
		while( room != NULL )
		{
			if( room->isSelect(Table) )
				break;
			room = fRoomList.Next(room);
		}
	}
	if( room  == NULL ) {
		fRoomList.Insert(room = OnCreateRoom(Table,Private),false);
		room->fTable = Table;
		room->fPrivate = Private;
	}
//	TLockSection lRoom(room);
	player->fRoom = room;
	player->fGameNum = room->fGameNum;
	room->PlayerAdd(player);
}


void IPlayerProducer::FreeRoom(IPlayer *player)
{
	if( player->fRoom != NULL ) {
		TLockSection lRoomList(&fRoomList);
		player->fRoom->PlayerDel(player);
		if( player->fRoom->isFree() )
			fRoomList.Delete(player->fRoom,false);
	}
}


bool IPlayerRoom::isTimerStart()
{
	return fTimer > 0;
}


bool IPlayerRoom::isTimer(const unsigned int Time)
{
//	print("Time=",Time,"  fTimer=",Time,"  time(NULL)=",time(NULL),"  time(NULL)-fTimer=",time(NULL)-fTimer);
	if( !fPrivate && isTimerStart() && (time(NULL) - fTimer) > (int)Time )
		return true;
	return false;
}


void IPlayerRoom::TimerUpdate(const unsigned int Time)
{
	fTimer = time(NULL) - Time;
}


void IPlayerRoom::TimerStart(const unsigned int Time)
{
	fTimer = time(NULL)-Time;
}


void IPlayerRoom::TimerStop()
{
	fTimer = 0;
}


TStr IPlayerRoom::TimerToSSI(m_ssi::TSSIControl *ssi, const unsigned int Time)
{
	if( !fPrivate && isTimerStart() ) {
		ssi->SSIValue("TIMER",(int)(100-(time(NULL) - fTimer) * 100.0 / Time));
		return ssi->SSIRes("server_gametimer");
	}
	return "";
}


void IPlayerRoom::Chat(IPlayer* player, const TStr& str)
{
	if( !str.isEmpty() ) {
		fChat1 = fChat2;
		fUserID1 = fUserID2;
		fChat2 = str;
		fUserID2 = player->ID();
		TServerSSI rs;
		rs.ssi()->SSIValue("TEXT",fChat2);
		rs.ssi()->SSIValue("USERID",fUserID2);
		PlayerMessage(rs.ssi()->SSIRes("server_chat"),player);
	}
}


void IPlayerRoom::PlayerMessage(const TStr& Message, IPlayer* ignore)
{
	for( unsigned int i = 0; i < fPlayerList.Count(); i++ )
		if( fPlayerList.Get(i) != ignore )
			fPlayerList.Get(i)->AddMsg(Message);
}


unsigned int IPlayerRoom::FindIndex(IPlayer *player)
{
	for( unsigned int i = 0; i < fPlayerList.Count(); i++ )
		if( fPlayerList.Get(i) == player )
			return i;
	throw TExcept("IPlayerRoom::Find");
}


void IPlayerRoom::PlayerAdd(IPlayer *player)
{
	TServerSSI rs;
	ssi = rs.ssi();
//	TLockSection _l(this);
	OnPlayerAdd(player);
/*
	if( !fChat1.isEmpty() ) {
		rs.ssi()->SSIValue("TEXT",fChat1);
		rs.ssi()->SSIValue("USERID",fUserID1);
		player->AddMsg(rs.ssi()->SSIRes("server_chat"));
	}
	if( !fChat2.isEmpty() ) {
		rs.ssi()->SSIValue("TEXT",fChat2);
		rs.ssi()->SSIValue("USERID",fUserID2);
		player->AddMsg(rs.ssi()->SSIRes("server_chat"));
	}
*/
	fPlayerList.Add(player);
	ssi = NULL;
}


void IPlayerRoom::PlayerDel(IPlayer *player)
{
	TServerSSI rs;
	ssi = rs.ssi();
//	TLockSection _l(this);
	OnPlayerDel(player);
	if( player->User() != NULL )
		player->User()->CashRollback();
	fPlayerList.Del(FindIndex(player));
	if( fUserID2 == player->ID() )
		fChat2.Clear();
	if( fUserID1 == player->ID() )
		fChat1.Clear();
//  !!! Clear Game pointer 
//	if( player->User() != NULL )
//		player->User()->GameClear();
	ssi = NULL;
}


