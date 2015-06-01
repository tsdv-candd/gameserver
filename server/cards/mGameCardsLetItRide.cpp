#include "mGameCardsLetItRide.h"

#include "server/mUser.h"
#include "server/mServer.h"
#include "utils/m_utils.h"

TLetItRideProducer LetItRide;

TPokerCombination* TLetItRideRoom::fGame;

static TPokerCombination::TCombination CombinationGame[9] = {
	{  1, 1000 },
	{  2, 200  },
	{  3, 50   },
	{  4, 11   },
	{  5, 8    },
	{  6, 5    },
	{  7, 3    },
	{  8, 2    },
	{  9, 1    }
};

void TLetItRideRoom::Init()
{
	fGame = new TPokerCombination();
	fGame->Init(CombinationGame,9);
}

void TLetItRideRoom::Free()
{
	delete fGame;
}

// - LOG

TStr TLetItRideRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int i = 0; i < 3; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
			for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
				s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(Box(i)->Card(j))+"\" />";
			TCardsCombination BoxCombination(Box(i)->Cards());
			TStr Result(CardsCombinationAsStr(BoxCombination.Combination()));
			s += "<box id=\""+TStr(i+1)+"\" ante=\""+TStr(Box(i)->Bet())+"\" result=\""+Result+"\" />";
		}
	if( !s.isEmpty() )
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
	return s;
}

// -- OnCommitCard

void TLetItRideRoom::OnCommitCard( TStep* p )
{
	unsigned int i;
	switch( p->fAction )
	{
		case MARKER_WIN: case MARKER_LOSE:
				for( i = 0; i < Box(p->fBox)->CardsCount(); i++ )
					PlayerMessage(CardToSSI(ssi,"open",Box(p->fBox)->Card(i),p->fBox+1),Box(p->fBox)->Player());
				TCardsRoom::OnCommitCard(p);
			break;
		case MARKER_OPEN:
			if( p->fBox > -1)
				Box(p->fBox)->Player()->AddMsg(CardToSSI(ssi,"open",p->fCard,p->fBox+1));
			else
				TCardsRoom::OnCommitCard(p);
			break;
		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnTagPlay

void TLetItRideRoom::OnTagPlay(IPlayer *Player)
{
	Player->AddMsg(ssi->SSIRes("cards_button_return"));
	Player->AddMsg(ssi->SSIRes("cards_button_raise"));
}

// - OnGameStart

void TLetItRideRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_letitride.txt");
	bool isGame = false;
	for( unsigned int i = 0; i < 3; i++ )
		if( isBoxPlay(i) ) {
			Box(i)->BetAdd(1,Box(i)->Bet()/3);
			isGame = true;
		}
	if( isGame ) {
		PlayerMessage(ssi->SSIRes("cards_delivery"),NULL);
		do {
			Marker();
			bool f = true;
			for( unsigned int i = 0; i < 3; i++ )
				if( isBoxPlay(i) ) {
					MarkerCard(i,MARKER_OPEN);
					MarkerCard(i,MARKER_OPEN);
					MarkerCard(i,MARKER_OPEN);
					int Cash = BoxCashWin(i)/*+Box(i)->Bet(1)*2*/;
					f = CashTmpAdd(i,Cash);
					if( !f )
						break;
					MarkerAction(i,MARKER_TMP,Cash);
				}
			if( f ) {
				MarkerCommit();
				break;
			}
			MarkerRollback();
		} while( true );
		TimerStart();
		TagPlayNext(3);
	} else
		GameEnd();
}

// - OnGame

void TLetItRideRoom::OnGame(const TStr& Action, const TStr&, IPlayer* Player)
{
	if( Action == "leave" ) 
		Player->fTag = -1;
	else
	if( Action == "Timer" ) {
		for( unsigned int i = 0; i < PlayerCount(); i++ )
		{
			IPlayer* Player = TCardsRoom::Player(i);
			if( Player->fTag > -1 ) {
				Box(Player->fTag)->BetAdd(-(int)Box(Player->fTag)->Bet(1));
				Box(Player->fTag)->BetAdd(BOX_WIN,Box(Player->fTag)->Bet(1));
				PlayerMessage(BoxAction(ssi,"return",Player->fTag+1),NULL);
				Player->fTag = -1;
			}
		}
	} else
	if( Player->fTag > -1 ) {
		if( Action != "Raise" ) {
			Box(Player->fTag)->BetAdd(-(int)Box(Player->fTag)->Bet(1));
			Box(Player->fTag)->BetAdd(BOX_WIN,Box(Player->fTag)->Bet(1));
		}
		PlayerMessage(BoxAction(ssi,Action,Player->fTag+1),NULL);
		Player->fTag = -1;
	}
	if( TagGameEnd() ) {
		bool isGameEnd = IBox(-1)->CardsCount() == 1;
		do {
			Marker();
			bool f = true;
			MarkerCard(-1,MARKER_OPEN);
			for( unsigned int i = 0; i < 3; i++ )
				if( isBoxPlay(i) ) {
					// !!! 320 & -
					// int Cash = BoxCashWin(i) - Box(i)->Bet(BOX_TMP);
					int Cash = BoxCashWin(i);
					if( Cash > 0 ) {
//						Cash -= Box(i)->Bet(BOX_TMP);
						print("cash=",Cash);
					}
					//

					if( Cash > 0 )
						if( !(f = CashTmpAdd(i,Cash)) )
							break;
					if( isGameEnd )
						if( BoxCashWin(i) > 0 )
							MarkerAction(i,MARKER_WIN,BoxCashWin(i));
						else
							MarkerAction(i,MARKER_LOSE);
				}
			if( f ) {
				MarkerCommit();
				break;
			} 
			MarkerRollback();
		} while( true );
		if( isGameEnd )
			GameEnd();
		else {
			TimerStart();
			TagPlayNext(3);
		}
	}
}

// - OnPlayerAdd

void TLetItRideRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);
	if( Mode() == 1 ) {
		Player->AddMsg(ssi->SSIRes("cards_delivery_show"));
		Player->fTag = -1;
		if( IBox(-1)->CardsCount() == 1 ) {
			ssi->SSIValue("CARD",IBox(-1)->Card(0));
			ssi->SSIValue("BOXID",0);
			ssi->SSIValue("ACTION","open");
			Player->AddMsg(ssi->SSIRes("cards_card"));
		}
		for( unsigned int i = 0; i < PlayerCount(); i++ )
			if( TCardsRoom::Player(i)->fTag > -1 )
				if( Box(TCardsRoom::Player(i)->fTag)->Bet(BOX_WIN) > 0 )
					Player->AddMsg(BoxAction(ssi,"return",TCardsRoom::Player(i)->fTag+1));
	}
}

unsigned int TLetItRideRoom::BoxCashWin(const unsigned int BoxID)
{
	unsigned int Cards[5];
	unsigned int iCards = 0;
	unsigned int i;
	for( i = 0; i < 3; i++ )
		Cards[iCards++] = Box(BoxID)->Card(i);
	for( i = 0; i < IBox(-1)->CardsCount(); i++ )
		Cards[iCards++] = IBox(-1)->Card(i);
	while( iCards < 5 )
		Cards[iCards++] = 0;
	TCardsCombination BoxCombination(Cards);
	int Cash = Box(BoxID)->Bet();
	int r = Cash * fGame->Cash(BoxCombination.Combination());
	if( r > 0 )
		 r += Cash;
	int MaxWin = 0;
	switch( Table() )
	{
		case 1: MaxWin += 10000; break;
		case 2: MaxWin += 500000; break;
		case 3: MaxWin += 1000000; break;
	}
	if( r > MaxWin )
		r = MaxWin;
	return r;
}
