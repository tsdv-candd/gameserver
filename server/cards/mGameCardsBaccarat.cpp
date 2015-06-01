#include "mGameCardsBaccarat.h"

#include "server/mUser.h"
#include "server/mServer.h"

TBaccaratProducer Baccarat;

// - LOG

TStr TBaccaratRoom::Log(IPlayer* p)
{
	TStr s;
	unsigned int BetPlayer = 0,BetBanker = 0,BetTie = 0;
	for( unsigned int i = 0; i < 5; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
			BetPlayer += Box(i)->Bet(BOX_PLAYER);
			BetBanker += Box(i)->Bet(BOX_BANKER);
			BetTie += Box(i)->Bet(BOX_TIE);
			if( s.isEmpty() ) 
				for( unsigned int j = 0; j < Box(6)->CardsCount(); j++ )
					s += "<card id=\"player\" type=\""+TStr(Box(6)->Card(j))+"\" />";
		}
	if( !s.isEmpty() ) {
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			s += "<card id=\"banker\" type=\""+TStr(IBox(-1)->Card(j))+"\" />";
		if( BetPlayer > 0 )
			s += TStr("<bet type=\"player\" cash=\"")+BetPlayer+"\" />";
		if( BetBanker > 0 )
			s += TStr("<bet type=\"banker\" cash=\"")+BetBanker+"\" />";
		if( BetTie > 0 )
			s += TStr("<bet type=\"tie\" cash=\"")+BetTie+"\" />";
	}
	return s;
}

// - OnGameEnd

void TBaccaratRoom::OnGameEnd()
{
	Marker();
	for( unsigned int i = 0; i < PlayerCount(); i++ )
		Player(i)->User()->CashResAdd(cBaccaratGame,Table(),Player(i)->User()->CashWinTotal(),iRollback >= 100);
	MarkerCommit();
}

// -- OnCommitCard

void TBaccaratRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{

		case MARKER_WIN:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("CASH",p->fInfo);
				ssi->SSIValue("BET",BoxStr(p->fBet));
				PlayerMessage(ssi->SSIRes("cards_action_win_bet"),NULL);
			break;

		case MARKER_PUSH:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("CASH",p->fInfo);
				ssi->SSIValue("BET",BoxStr(p->fBet));
				PlayerMessage(ssi->SSIRes("cards_action_push_bet"),NULL);
			break;

		case MARKER_LOSE:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("BET",BoxStr(p->fBet));
				PlayerMessage(ssi->SSIRes("cards_action_lose_bet"),NULL);
			break;

		case MARKER_SCORE:
				ssi->SSIValue("BOXID",p->fBox+1);
				ssi->SSIValue("SCORE",p->fInfo);
				PlayerMessage(ssi->SSIRes("cards_action_score"),NULL);
			break;

		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnGameStart

void TBaccaratRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_baccarat.txt");
	if( isGame() ) {
		do {
			bool f = true;
			Marker();

			IBox(6)->GameClear();
			MarkerCard(6,MARKER_CARD);
			MarkerAction(6,MARKER_SCORE,BoxScore(6));
			MarkerCard(6,MARKER_CARD);
			MarkerAction(6,MARKER_SCORE,BoxScore(6));

			unsigned int fScoreBox = BoxScore(6);
			if( fScoreBox <  6 ) {
				MarkerCard(6,MARKER_CARD);
				MarkerAction(6,MARKER_SCORE,BoxScore(6));
				fScoreBox = BoxScore(6);
			}
		
			unsigned int fScoreLast = CardPrice(IBox(6)->CardLast());
			if( fScoreLast == 11 )
				fScoreLast = 1;

			MarkerCard(-1,MARKER_CARD);
			MarkerAction(-1,MARKER_SCORE,BoxScore(-1));
			MarkerCard(-1,MARKER_CARD);
			MarkerAction(-1,MARKER_SCORE,BoxScore(-1));

			unsigned int fScoreDealer = BoxScore();
			if(
				fScoreDealer <= 2 ||
				fScoreDealer == 3 && fScoreLast != 8 ||
				fScoreDealer == 4 && fScoreLast >= 2 && fScoreLast <= 7 ||
				fScoreDealer == 5 && fScoreLast >= 4 && fScoreLast <= 7 ||
				fScoreDealer == 6 && fScoreLast >= 6 && fScoreLast <= 7 )
			{
				MarkerCard(-1,MARKER_CARD);
				MarkerAction(-1,MARKER_SCORE,BoxScore(-1));
				fScoreDealer = BoxScore(-1);
			}

			for( unsigned int i = 0; i < 5; i++ )
			{
				if( isBoxBet(i) ) {
					if( Box(i)->Bet(BOX_TIE) > 0 ) {
						if( fScoreDealer == fScoreBox ) {
							unsigned int Cash = Box(i)->Bet(BOX_TIE)*895/100;
							if( !(f = CashTmpAdd(i,Cash)) )
								break;
							MarkerAction(i,MARKER_WIN,Cash)->fBet = BOX_TIE;
						} else
							MarkerAction(i,MARKER_LOSE,"")->fBet = BOX_TIE;
					}
					if( Box(i)->Bet(BOX_BANKER) > 0 ) {
						unsigned int Cash = Box(i)->Bet(BOX_BANKER)*95/100;
						if( fScoreDealer > fScoreBox ) {
							Cash += Box(i)->Bet(BOX_BANKER);
							if( !(f = CashTmpAdd(i,Cash)) )
								break;
							MarkerAction(i,MARKER_WIN,Cash)->fBet = BOX_BANKER;
						} else
						if( fScoreDealer == fScoreBox ) {
							if( !(f = CashTmpAdd(i,Box(i)->Bet(BOX_BANKER))) )
								break;
							MarkerAction(i,MARKER_WIN,Box(i)->Bet(BOX_BANKER))->fBet = BOX_BANKER;
						} else
							MarkerAction(i,MARKER_LOSE,"")->fBet = BOX_BANKER;
					}
					if( Box(i)->Bet(BOX_PLAYER) > 0 ) {
						unsigned int Cash = Box(i)->Bet(BOX_PLAYER);
						if( fScoreDealer < fScoreBox ) {
							Cash += Box(i)->Bet(BOX_PLAYER);
							if( !(f = CashTmpAdd(i,Cash)) )
								break;
							MarkerAction(i,MARKER_WIN,Cash)->fBet = BOX_PLAYER;
						} else
						if( fScoreDealer == fScoreBox ) {
							if( !(f = CashTmpAdd(i,Cash)) )
								break;
							MarkerAction(i,MARKER_WIN,Cash)->fBet = BOX_PLAYER;
						} else
							MarkerAction(i,MARKER_LOSE)->fBet = BOX_PLAYER;
					}
				}
			}
			if( f ) {
				MarkerCommit();
				break;
			}
			MarkerRollback();
		} while( true );
	}
	GameEnd();
}

// -

unsigned int TBaccaratRoom::BoxScore(const int BoxID)
{
	unsigned int Score = 0;
	unsigned int i = IBox(BoxID)->CardsCount();
	while( i-- > 0 )
	{
		Score += CardPrice(IBox(BoxID)->Card(i));
		if( Score > 9 )
			Score -= 10;
	}
	if( Score > 9 )
		Score -= 10;
	return Score;
}


