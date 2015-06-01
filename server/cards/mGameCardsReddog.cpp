#include "mGameCardsReddog.h"

#include "server/mUser.h"
#include "server/mServer.h"

TRedDogProducer RedDog;

/*

 1 .. 13 .. xx
 1 .. 1  .. 1
 1 .. 1  .. 3
 1 .. 2
 1 .. 4  .. 3
 1 .. 4  .. 1,4,5

*/

// - LOG

TStr TRedDogRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
		s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
	s += "<stread id=\""+TStr(fSpread)+"\" />";
	for( unsigned int i = 0; i < 5; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
			TStr Result("lose");
			if( Box(i)->Bet(BOX_WIN) > 0 )
				Result = "win";
			s += "<box id=\""+TStr(i+1)+"\" ante=\""+TStr(Box(i)->Bet())+"\" result=\""+Result+"\" />";
		}
	return s;
}

// -- OnCommitCard

void TRedDogRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{

		case MARKER_WIN: case MARKER_PUSH:
			Box(p->fBox)->BetAdd(BOX_WIN,p->fInfo.ToIntDef(0));
			TCardsRoom::OnCommitCard(p);
			break;

		case MARKER_PLAY:
				if( Box(p->fBox)->Player()->fTag < 0 )
					TagPlay(p->fBox);
			break;

		case MARKER_SPREAD:
				ssi->SSIValue("BOXID",0);
				ssi->SSIValue("VALUE",p->fInfo.ToIntDef(0));
				fSpread = p->fInfo.ToIntDef(0);
				ssi->SSIValue("ACTION","spread");
				PlayerMessage(ssi->SSIRes("cards_action_value"),NULL);
			break;

		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnTagPlay

void TRedDogRoom::OnTagPlay(IPlayer* Player)
{
	Player->AddMsg(ssi->SSIRes("cards_button_raise"));
	Player->AddMsg(ssi->SSIRes("cards_button_stand"));
}

// - OnGameStart

void TRedDogRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_reddog.txt");
	TagInit();
	if( isGame() ) {
		do {
			Marker();
			bool f = true;
			MarkerCard(-1,MARKER_CARD);
			MarkerCard(-1,MARKER_CARD);
			unsigned int f1 = CardID(IBox(-1)->Card(0));
			unsigned int f2 = CardID(IBox(-1)->Card(1));
			int d = f1 - f2;
			if( d < 0 )
				d = -d;
			if( d == 0 || d == 1 ) {
				unsigned int k = 1;
				if( d == 0 ) {
					MarkerCard(-1,MARKER_CARD);
					if( CardID(IBox(-1)->Card(2)) == f2 ) {
						MarkerAction(-1,MARKER_SPREAD,14);
						k = 11+1;
					} else
						MarkerAction(-1,MARKER_SPREAD,13);
				} else
					MarkerAction(-1,MARKER_SPREAD,12);
				for( unsigned int i = 0; i < 5; i++ )
					if( Box(i)->Player() != NULL && Box(i)->Bet(BOX_BET) > 0 ) {
						if( !(f = CashTmpAdd(i,Box(i)->Bet(BOX_BET)*k)) )
							break;
						if( k > 1 )
							MarkerAction(i,MARKER_WIN,Box(i)->Bet(BOX_BET)*k);
						else
							MarkerAction(i,MARKER_PUSH,Box(i)->Bet(BOX_BET));
					}
			} else {
				MarkerAction(-1,MARKER_SPREAD,d-1);
				for( unsigned int i = 0; i < 5; i++ )
					if( Box(i)->Player() != NULL && Box(i)->Bet(BOX_BET) > 0 )
						MarkerAction(i,MARKER_PLAY);
			}
			if( f ) {
				MarkerCommit();
				break;
			}
			MarkerRollback();
		} while( true );
		if( TagGameEnd() )
			GameEnd();
		else
			TimerStart();
	} else
		GameEnd();
}

// - OnGame

void TRedDogRoom::OnGame(
	const TStr& Action,
	const TStr&,
	IPlayer* Player)
{
	if( Action == "leave" ) {
		while( Player->fTag > -1 )
			TagPlayNext(Player,5);
	} else 
	if( Action == "Timer" ) {
		bool f = false;
		for( unsigned int i = 0; i < PlayerCount(); i++ )
			if( TCardsRoom::Player(i)->fTag > -1 ) {
				PlayerMessage(BoxAction(ssi,"stand",TCardsRoom::Player(i)->fTag+1),NULL);
				if( TagPlayNext(TCardsRoom::Player(i),5) )
					f = true;
			}
		if( f )
			TimerUpdate();
	} else
	if( Player->fTag > -1 ) {
		if( Action == "raise" ) {
			int CashBet = Box(Player->fTag)->Bet(BOX_BET);
			if( Player->User()->CashBetCommit(cRedDogGame,Table(),CashBet) ) {
				PlayerMessage(BoxAction(ssi,"raise",Player->fTag+1,CashBet),NULL);
				Box(Player->fTag)->BetAdd(BOX_RAISE,CashBet);
			}
		} else
			PlayerMessage(BoxAction(ssi,"stand",Player->fTag+1),NULL);
		TagPlayNext(Player,5);
	}

	if( Player->fTag > -1 && Timer() > 45 )
		TimerUpdate();

	if( TagGameEnd() ) {
		do {
			Marker();
			bool f = true;
			MarkerCard(-1,MARKER_CARD);
			unsigned int f1 = CardID(IBox(-1)->Card(0));
			unsigned int f2 = CardID(IBox(-1)->Card(1));
			unsigned int f3 = CardID(IBox(-1)->Card(2));
			int k = f1 - f2;
			if( k < 0 )
				k = -k;
			k -= 1;
			if( f1 < f3 && f3 < f2 || f2 < f3 && f3 < f1 ) {
				switch( k )
				{
					case 1: k = 5; break;
					case 2: k = 4; break;
					case 3: k = 2; break;
					default: k = 1;
				}
				k++;
			} else
				k = 0;
			for( unsigned int i = 0; i < 5; i++ )
				if( isBoxPlay(i) ) {
					unsigned int CashWin = (Box(i)->Bet(BOX_RAISE)+Box(i)->Bet(BOX_BET))*k; 
					f = CashTmpAdd(i,CashWin);
					if( !f )
						break;
					if( CashWin > 0 )
						MarkerAction(i,MARKER_WIN,CashWin);
					else
						MarkerAction(i,MARKER_LOSE);
				}
			if( f ) {
				MarkerCommit();
				GameEnd();
				break;
			}
			MarkerRollback();
		} while( true );
	}
}

// - OnPlayerAdd

void TRedDogRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);
	if( Mode() == 1 ) {
		Player->AddMsg(ssi->SSIRes("cards_delivery_show"));
		ssi->SSIValue("BOXID",0);
		ssi->SSIValue("ACTION","card");
		ssi->SSIValue("CARD",IBox(-1)->Card(0));
		Player->AddMsg(ssi->SSIRes("cards_card"));
		ssi->SSIValue("CARD",IBox(-1)->Card(1));
		Player->AddMsg(ssi->SSIRes("cards_card"));
		Player->fTag = -1;
	}
}


