#include "mGameCards.h"

#include "server/mServer.h"

//#include "file/m_file.h"

const unsigned int NUM[5] = { 2, 1, 0, 3, 4 };



void TCardsRoom::OnBoxJoin( const unsigned int BoxID, IPlayer* Player )
{
	Player->User()->ToSSI(ssi);
	ssi->SSIValue("BOXID",BoxID+1);
	ssi->SSIValue("USERID",Player->ID());
	Player->AddMsg(ssi->SSIRes("cards_join"));
	PlayerMessage(ssi->SSIRes("cards_joinuser"),Player);
//  PlayerMessage(BoxBet(ssi,BoxID,true),NULL);
}


void TCardsRoom::OnBoxLeave( const unsigned int BoxID )
{
	Box(BoxID)->Clear();
	ssi->SSIValue("BOXID",BoxID+1);
	PlayerMessage(ssi->SSIRes("cards_leave"),NULL);
}


bool TCardsRoom::OnRequestAction( IPlayer* Player, TXMLNode *p )
{
	TStr Action = "timer";
	TStr ID;
	if( p != NULL ) {
		Action = p->VarValue("button");
		ID = p->VarValue("id");
		if( ID.isEmpty() )
			ID = p->VarValue("id1") + ":" + p->VarValue("id2");
	}
	switch( Mode() )
	{
		case 0: {
			if( Action == "deal" )
				Player->ActionUpdate(1);
			bool isGame = true;
			unsigned int i;
			for( i = 0; i < PlayerCount(); i++ )
				if( !IPlayerRoom::Player(i)->Action() && IPlayerRoom::Player(i)->isGame() ) {
					isGame = false;
					break;
				}
			isGame = isGame || isTimer(GetTimer());
			if( isGame ) {
				for( i = 0; i < BoxCount(); i++ )
					// !!!
//					if( Box(i)->Player() != NULL ) {
						PlayerMessage(BoxBet(i,true),NULL);
//					} 
				ModeNext();
				OnGameStartBefore();
				IBox(-1)->GameClear();
				for( i = 0; i < PlayerCount(); i++ )
				{
					IPlayerRoom::Player(i)->User()->CashBetCommit(GameID(),Table(),0);
				}
				OnGameStart();
			}
			break;
		}
		case 1: {
			OnGame(Action,ID,Player);
			break;
		}
	}
	return true;
}


bool TCardsRoom::OnRequestBet( IPlayer* Player, TXMLNode *p)
{
	int ID = p->VarValue("id").ToIntDef(0);
	int Cash = p->VarValue("cash").ToIntDef(0);
	ssi->SSIValue("BOXID",ID);
	TStr sBet(p->VarValue("bet"));
	ssi->SSIBlock("&isBet",BoxStr(0)[0] != '\0');
	if( BoxStr(0)[0] != '\0' ) {
		sBet = p->VarValue("bet");
		ssi->SSIValue("Bet",sBet);
	}
	if( ID > 0 && Player->isGame() && Mode() == 0 ) {
		if( Box(ID-1)->Player() != NULL && Box(ID-1)->Player() != Player )
			return true;
		int Code = Bet(ID-1,Player,Cash,sBet);
		if( Code == 2 ) {
			OnBoxJoin(ID-1,Player);
		}
		if( Code != 0 ) {
			ssi->SSIValue("Cash",Cash);
			PlayerMessage(ssi->SSIRes("cards_bet"),Player);
		}
//    if( Code == 2 )
//      PlayerMessage(BoxBet(ssi,ID-1,true),NULL);
		if( Code == 3 && Player->fTag != ID-1 && Box(ID-1)->Del() ) {
			// ћ€гкое удаление
			OnBoxLeave(ID-1);
		}
	}
/*
	else {
		ssi->SSIValue("Cash",-Cash);
		Player->AddMsg(ssi->SSIRes("cards_bet"));
	}
*/
	return true;
}


void TCardsRoom::OnRequest(TRequest *r, IPlayer* Player)
{
	if( Player->isGame() ) {
		switch( Mode() )
		{
			case 2:
				for( unsigned int i = 0; i < PlayerCount(); i++ )
					IPlayerRoom::Player(i)->ActionUpdate(0);
				ModeClear();
				break;
		}
	}
	TServerSSI rs;
	ssi = rs.ssi();
	TXMLNode* pAction = NULL;
	for( unsigned int i = 0; i < r->xml()->NodesCount(); i++ )
	{
		TXMLNode* p = r->xml()->GetNode(i);
		if( p->GetName() == "action" ) pAction = p; else
		if( p->GetName() == "bet" )    OnRequestBet(Player,p);
	}
	if( Player->isGame() ) {
		if( pAction != NULL || isTimer(GetTimer()) )
			OnRequestAction(Player,pAction);
		switch( Mode() )
		{
			case 0: case 1:
				if( !isTimerStart() )
					TimerStart();
				r->AddReturn(TimerToSSI(ssi));
				break;
		}
		switch( Mode() )
		{
			case 0:
				r->AddStatus("bet");
				break;
			case 1:
				r->AddStatus("game");
				break;
			default:
				r->AddStatus("ok");
		}
	}
	ssi = NULL;
}


void TCardsRoom::OnPlayerAdd(IPlayer *Player)
{
	TStr s;
	for( unsigned int i = 0; i < BoxCount(); i++ )
	{
		if( Box(i)->Player() != NULL ) {
			ssi->SSIValue("BOXID",i+1);
			ssi->SSIValue("USERID",Box(i)->Player()->ID());
			Box(i)->Player()->User()->ToSSI(ssi);
			s += ssi->SSIRes("cards_joinuser");
			s += BoxBet(i,true);
		}
	}

/*
	static int c = 0;
	switch( c++ )
	{
	case 0:
		s += "<chat> join - bet - leave</chat>";
		s += "<box id=\"1\" action=\"join\"  />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"ante\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bonus\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bet\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"tie\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"leave\" />";
		break;
	case 1:
		s += "<chat> join - bet - join</chat>";
		s += "<box id=\"1\" action=\"join\"  />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"ante\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bonus\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bet\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"tie\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"join\"  />";
		break;
	case 2:
		s += "<chat> join - bet - leave OTHER</chat>";
		s += "<box id=\"1\" action=\"join\"  user_id=\"8\" nick=\"nick\" icon=\"\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"ante\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bonus\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bet\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"tie\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"leave\" />";
		break;
	default:
		c = 0;
		s += "<chat> join - bet - join OTHER</chat>";
		s += "<box id=\"1\" action=\"join\"  user_id=\"8\" nick=\"nick\" icon=\"\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"ante\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bonus\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"bet\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"bet\"   bet=\"tie\"  cash=\"100\" />";
		s += "<box id=\"1\" action=\"join\"  user_id=\"8\" nick=\"nick\" icon=\"\" />";
		break;
	}
*/

	OnBoxJoin(Player->fTag = Add(Player),Player);
	Player->AddMsg(s);
	Player->ActionUpdate(0);
	Player->AddMsg(TimerToSSI(ssi));
	if( Mode() == 1 )
		Player->fTag = -1;
}


void TCardsRoom::OnPlayerDel(IPlayer *Player)
{
	if( Mode() == 1 )
		OnGame("leave","",Player);
	else
	if( Mode() == 0 )
		OnRequestAction(Player,NULL);
	unsigned int ID[10];
	unsigned int i = Del(Player,ID);
	while( i-- > 0 )
		OnBoxLeave(ID[i]);
}


void TCardsRoom::GameResult()
{
	PlayerMessage(ssi->SSIRes("cards_gameend"),NULL);
	ModeNext();
	TimerStop();
}


void TCardsRoom::GameEnd()
{
		if( Mode() == 1 )
			GameResult();

		OnGameEnd();

		unsigned int i;

		for( i = 0; i < PlayerCount(); i++ )
		{
			// !!! ERR
			// Player(i)->User()->CashBetCommit() == 0
			// !!!
			if( Player(i)->User()->CashBetCommit() > 0 ) {
				Player(i)->User()->CashResCommit(GameID());
				ssi->SSIValue("CASHWIN",Player(i)->User()->CashWinTotal());
				ssi->SSIValue("CASHBET",Player(i)->User()->CashBetCommit());
				Player(i)->AddMsg(ssi->SSIRes("cards_game"));
				Player(i)->User()->CashWinCommit(Log(Player(i)));
			} else
				Player(i)->User()->CashRollback();
			Player(i)->User()->CashCommit();
			ssi->SSIValue("CASH",Player(i)->User()->Cash());
			ssi->SSIValue("BONUS",Player(i)->User()->CashBonus());
			Player(i)->AddMsg(ssi->SSIRes("cards_usercash"));
		}
		if( isNewsGameButton() )
			PlayerMessage(ssi->SSIRes("cards_button_newgame"),NULL);
		else
			PlayerMessage(ssi->SSIRes("cards_button_newgame_alpha"),NULL);
//		if( !Private() )
			PlayerMessage(ssi->SSIRes("cards_newgame"),NULL);

		GameNext();

		for( i = 0; i < PlayerCount(); i++ ) {
//      Player(i)->User()->CashWinCommit("");
			Player(i)->fTag = -1;
//			Player(i)->User()->ToSSI(ssi);
//			Player(i)->AddMsg(ssi->SSIRes("server_user"));
//			Player(i)->User()->CashRollback();
		}

		unsigned int j;
		for( j = 0; j < 10; j++ ) {
			unsigned int i = j;
			if( i < 5 )
				i = NUM[j];
			Box(i)->GameClear();
			if( Box(i)->Player() != NULL ) {
				if( !Box(i)->Del() ) {
					Box(i)->Player()->fTag = i;
					Box(i)->GameClear();
				} else {
					ssi->SSIValue("BOXID",i+1);
					PlayerMessage(ssi->SSIRes("cards_leave"),NULL);
					Box(i)->Clear();
				}
			}
		}
}


TStr TCardsRoom::BoxBet( const unsigned int BoxID, const bool fCash)
{
	TStr s;
	ssi->SSIValue("BOXID",BoxID+1);
	ssi->SSIBlock("&isBet",BoxStr(0)[0] != '\0');
	for( unsigned int i = 0; i < 4; i++ )
	{
		if( BoxStr(i)[0] == '\0' && i > 0 )
			break;
		if( Box(BoxID)->Player() == NULL )
			ssi->SSIValue("CASH","0");
		else
			ssi->SSIValue("CASH",Box(BoxID)->Bet(i));
		ssi->SSIValue("BET",BoxStr(i));
		if( fCash )
			s += ssi->SSIRes("cards_betcash");
		else
			s += ssi->SSIRes("cards_bet");
	}
	return s;
}



void TCardsRoom::OnCommitCard(TStep* p)
{
	switch( p->fAction )
	{

		case MARKER_OPEN: case MARKER_CARD: {
			const char* Action = "open";
			if( p->fAction == MARKER_CARD )
				Action = "card";
			PlayerMessage(CardToSSI(ssi,Action,p->fCard,p->fBox+1),NULL);
			break;
		}

		case MARKER_LOSE:
			PlayerMessage(BoxAction(ssi,"lose",p->fBox+1),NULL);
			break;

		case MARKER_WIN:
			PlayerMessage(BoxAction(ssi,"win",p->fBox+1,p->fInfo.ToIntDef(0)),NULL);
			break;

		case MARKER_PUSH:
			PlayerMessage(BoxAction(ssi,"push",p->fBox+1,p->fInfo.ToIntDef(0)),NULL);
			break;

		case MARKER_TMP:
			Box(p->fBox)->BetAdd(BOX_TMP,p->fInfo.ToIntDef(0));
			break;

	}
}


bool TCardsRoom::isGame()
{
	for( unsigned int i = 0; i < BoxCount(); i++ )
		if( isBoxPlay(i) )
			return true;
	return false;
}


void TCardsRoom::TagInit()
{
	for( unsigned int i = 0; i < PlayerCount(); i++ )
		Player(i)->fTag = -1;
}

void TCardsRoom::TagPlay(unsigned int fBox)
{
	ssi->SSIValue("BOXID",fBox+1);
	Box(fBox)->Player()->fTag = fBox;
	Box(fBox)->Player()->AddMsg(ssi->SSIRes("cards_play"));
	OnTagPlay(Box(fBox)->Player());
}

bool TCardsRoom::TagPlayNext(const unsigned int BoxCount)
{
	bool f = false;
	TagInit();
	for( unsigned int i = 0; i < BoxCount; i++ )
		if( isBoxPlay(i) )
			if( Box(i)->Player()->fTag == -1 ) {
				TagPlay(i);
				f = true;
			}
	return f;
}

bool TCardsRoom::TagPlayNext(IPlayer *Player, const unsigned int BoxCount)
{
	unsigned int i = Player->fTag;
	while( ++i < BoxCount )
		if( isBoxPlay(i,Player) ) {
			OnTagPlayNext(Player);
			TagPlay(i);
			return true;
		}
	Player->fTag = -100;
	return false;
}

bool TCardsRoom::TagGameEnd()
{
	for( unsigned int i = 0; i < PlayerCount(); i++ )
		if( Player(i)->fTag > -1 )
			return false;
	return true;
}

void TCardsRoom::OnGameEnd()
{
	if( IBox(-1)->CardsCount() == 0 )
		return;
	Marker();
	for( unsigned int i = 0; i < 5; i++ )
	{
		if( isBoxBet(i) )
// TRUE 
			Box(i)->Player()->User()->CashResAdd(GameID(),Table(),Box(i)->Bet(BOX_WIN),true);
	}
	MarkerCommit();
}
