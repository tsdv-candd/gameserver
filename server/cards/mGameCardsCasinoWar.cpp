#include "mGameCardsCasinoWar.h"

#include "server/mUser.h"
#include "server/mServer.h"

TCasinoWarProducer CasinoWar;

// - LOG

TStr TCasinoWarRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int i = 0; i < 5; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
			for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
				s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(Box(i)->Card(j))+"\" />";
		}
	if( !s.isEmpty() )
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
	return s;
}

// -- OnCommitCard

void TCasinoWarRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{

		case MARKER_WIN:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("CASH",p->fInfo.ToIntDef(0));
				ssi->SSIValue("BET",BoxStr(p->fBet));
				PlayerMessage(ssi->SSIRes("cards_action_win_bet"),NULL);
				Box(p->fBox)->BetAdd(BOX_WIN,p->fInfo.ToIntDef(0));
				if( p->fBet == BOX_BET )
					Box(p->fBox)->PlayClear();
			break;

		case MARKER_LOSE:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("BET",BoxStr(p->fBet));
				PlayerMessage(ssi->SSIRes("cards_action_lose_bet"),NULL);
				if( p->fBet == BOX_BET )
					Box(p->fBox)->PlayClear();
			break;

		case MARKER_PLAY:
				if( Box(p->fBox)->Play() && Box(p->fBox)->Player()->fTag == -1 )
					TagPlay(p->fBox);
			break;

		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnTagPlay

void TCasinoWarRoom::OnTagPlay(IPlayer* Player)
{
	Player->AddMsg(ssi->SSIRes("cards_button_raise"));
	Player->AddMsg(ssi->SSIRes("cards_button_surrender"));
}

// - OnGameStart

void TCasinoWarRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_casinowar.txt");
	TagInit();
	bool isGame = false;
	for( unsigned int i = 0; i < 5; i++ )
		if( isBoxPlay(i) ) {
			Box(i)->Player()->User()->CashWinAdd(cCasinoWarGame,Table(),Box(i)->Bet(BOX_BET)/2);
			isGame = true;
		}
	if( isGame )
		BoxGame();
	else
		GameEnd();
}

// - OnGame

void TCasinoWarRoom::OnGame(
	const TStr& Action,
	const TStr&,
	IPlayer* Player)
{
	if( Action == "leave" ) {
		while( Mode() == 1 && Player->fTag > -1 )
			TagPlayNext(Player,5);
	} else
	if( Action == "timer" ) {
		bool f = false;
		for( unsigned int i = 0; i < PlayerCount(); i++ )
		{
			if( TCardsRoom::Player(i)->fTag > -1 ) {
				int CashBet = Box(TCardsRoom::Player(i)->fTag)->Bet(BOX_BET);
				PlayerMessage(BoxAction(ssi,"surrender",TCardsRoom::Player(i)->fTag+1,CashBet/2),NULL);
				Box(TCardsRoom::Player(i)->fTag)->BetAdd(BOX_WIN,CashBet/2);
				if( TagPlayNext(TCardsRoom::Player(i),5) )
					f = true;
			}
		}
		if( f )
			TimerUpdate();
	} else
	if( Player->fTag > -1 ) {
		int CashBet = Box(Player->fTag)->Bet(BOX_BET);
		if( Action == "raise" ) {
			if( Player->User()->CashBetCommit(cCasinoWarGame,Table(),CashBet) ) {
				Box(Player->fTag)->BetAdd(BOX_BET,CashBet);
				PlayerMessage(BoxAction(ssi,"raise",Player->fTag+1,CashBet),NULL);
			}
		} else {
			Box(Player->fTag)->PlayClear();
			PlayerMessage(BoxAction(ssi,"surrender",Player->fTag+1,CashBet/2),NULL);
			Box(Player->fTag)->BetAdd(BOX_WIN,CashBet/2);
		}
		TagPlayNext(Player,5);
	}

	if( Player->fTag > -1 && Timer() > 45 )
		TimerUpdate();

	if( TagGameEnd() )
		BoxGame();
}

// - OnPlayerAdd

void TCasinoWarRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);
	if( Mode() == 1 ) {
		Player->fTag = -1;
		Player->AddMsg(ssi->SSIRes("cards_delivery_show"));
		ssi->SSIValue("CARD",IBox(-1)->CardLast());
		ssi->SSIValue("BOXID",0);
		ssi->SSIValue("ACTION","card");
		Player->AddMsg(ssi->SSIRes("cards_card"));
		for( unsigned int i = 0; i < BoxCount(); i++ )
			if( isBoxPlay(i) ) {
				ssi->SSIValue("BOXID",i+1);
				ssi->SSIValue("CARD",Box(i)->CardLast());
				Player->AddMsg(ssi->SSIRes("cards_card"));
			}
	}
}

// -

void TCasinoWarRoom::BoxGame()
{
	do {
		Marker();
		bool f = true;
		if( IBox(-1)->CardsCount() == 1 ) {
			MarkerStep(-1,0,MARKER_CARD);
			MarkerStep(-1,0,MARKER_CARD);
			MarkerStep(-1,0,MARKER_CARD);
		}
		MarkerCard(-1,MARKER_CARD);
		for( unsigned int i = 0; i < 5; i++ )
		{
			if( isBoxPlay(i) ) {
				MarkerCard(i,MARKER_CARD);

				unsigned int BoxCardID = CardID(Box(i)->CardLast());
				unsigned int DealerCardID = CardID(IBox(-1)->CardLast());
				if( BoxCardID > DealerCardID || BoxCardID == DealerCardID && IBox(-1)->CardsCount() > 1 ) {
					// jj = 1.5
					unsigned int jj = 3;
					// jj = 1
					if( IBox(-1)->CardsCount() > 1 )
						jj = 2;
					// *jj
					if( !(f = CashTmpAdd(i,Box(i)->Bet(BOX_BET)*jj/2)) )
						break;
					// jj+0.5
					MarkerAction(i,MARKER_WIN,Box(i)->Bet(BOX_BET)*(jj+1)/2)->fBet = BOX_BET;
				} else
				if( CardID(Box(i)->CardLast()) < CardID(IBox(-1)->CardLast()) )
					MarkerAction(i,MARKER_LOSE)->fBet = BOX_BET;
				if( Box(i)->Bet(BOX_TIE) > 0 && IBox(-1)->CardsCount() == 1 ) {
					if( BoxCardID == DealerCardID ) {
						if( !(f = CashTmpAdd(i,Box(i)->Bet(BOX_TIE)*19/2)) )
							break;
						MarkerAction(i,MARKER_WIN,Box(i)->Bet(BOX_TIE)*10)->fBet = BOX_TIE;
					} else
						MarkerAction(i,MARKER_LOSE)->fBet = BOX_TIE;
				}

				if( IBox(-1)->CardsCount() == 1 && BoxCardID == DealerCardID )
					MarkerAction(i,MARKER_PLAY);
			}
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
}




