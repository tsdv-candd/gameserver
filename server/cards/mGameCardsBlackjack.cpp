#include "mGameCardsBlackjack.h"
#include "server/mUser.h"

#include "file/m_file.h"


TBlackjackProducer Blackjack;

int TBlackjackRoom::Mas[10] = { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9 };

// LOG

TStr TBlackjackRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int i = 0; i < 10; i++ )
		if( Box(i)->Player() == p && Box(i)->Bet() > 0 ) {
			for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
				s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(Box(i)->Card(j))+"\" />";
			TStr Result(CardsScore(IBox(i)->Cards(),IBox(i)->CardsCount()));
			if( isBlackjack(i) )
				Result = "BJ";
			s += "<box id=\""+TStr(i+1)+"\" bet=\""+TStr(Box(i)->Bet())+"\" result=\""+Result+"\" />";
		}
	if( !s.isEmpty() ) {
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
		TStr Result(CardsScore(IBox(-1)->Cards(),IBox(-1)->CardsCount()));
		if( isBlackjack(-1) )
			Result = "BJ";
		s += "<box result=\""+Result+"\" />";
	}
	return s;
}

// - OnGameEnd

void TBlackjackRoom::OnGameEnd()
{
	if( IBox(-1)->CardsCount() == 0 )
		return;
	fBox = -1;
	do {
		bool f = true;
		Marker();
		MarkerCard(-1,MARKER_OPEN);
		MarkerAction(-1,MARKER_SCORE,IBox(-1)->ScoreAsStr());
		while( IBox(-1)->Score() < 17 )
		{
			MarkerCard(-1,MARKER_CARD);
			MarkerAction(-1,MARKER_SCORE,IBox(-1)->ScoreAsStr());
		}
		unsigned int fScore = IBox(-1)->Score();
		if( fScore > 21 )
			MarkerAction(-1,MARKER_BUST);
		else
		if( fScore == 21 && IBox(-1)->CardsCount() == 2 )
			MarkerAction(-1,MARKER_BLACKJACK);

		for( unsigned int k = 0; k < 10; k++ )
		{
			unsigned int i = Mas[k];
			if( Box(i)->Player() != NULL && Box(i)->Bet() > 0 ) {
				unsigned int CashWin = 0;
				unsigned int Result = MARKER_LOSE;
				bool DealerBJ = isBlackjack(-1); 
				bool BoxBJ = isBlackjack(i);
				if( BoxBJ ) {
					if( DealerBJ ) {
						if( Box(i)->Bet(BOX_INSURANCE) > 0 ) {
							CashWin += Box(i)->Bet()*2;
							MarkerAction(i,MARKET_INSURANCE_WIN,Box(i)->Bet(BOX_INSURANCE));
							Result = MARKER_WIN;
						} else {
							CashWin = Box(i)->Bet();
							Result = MARKER_PUSH;
						}
					} else {
						if( Box(i)->Bet(BOX_INSURANCE) > 0 ) {
							CashWin += Box(i)->Bet()*2;
							MarkerAction(i,MARKET_INSURANCE_LOSE);
						} else {
							CashWin += Box(i)->Bet()*5/2;
						}
						Result = MARKER_WIN;
					}
				} else {
					if( Box(i)->Bet(BOX_INSURANCE) > 0 ) {
						if( DealerBJ ) {
							CashWin +=
								Box(i)->Bet() -
								Box(i)->Bet(BOX_DOUBLE) +
								Box(i)->Bet(BOX_INSURANCE);
							Result = MARKER_WIN;
							MarkerAction(i,MARKET_INSURANCE_WIN,Box(i)->Bet(BOX_INSURANCE));
						} else {
							MarkerAction(i,MARKET_INSURANCE_LOSE);
						}
					}
					if(
						fScore > 21 && Box(i)->Score() <= 21 ||
						Box(i)->Score() <= 21 && fScore < Box(i)->Score()
					) {
						CashWin += Box(i)->Bet() * 2;
						Result = MARKER_WIN;
					} else
					if( fScore <= 21 && fScore == Box(i)->Score() && !DealerBJ ) {
						CashWin += Box(i)->Bet();
						Result = MARKER_PUSH;
					}
				}
				if( !(f = Box(i)->Player()->User()->CashResAdd(cBlackjackGame,Table(),CashWin,iRollback >= 100)) )
					break;
				MarkerAction(i,Result,CashWin);
			}
		}
		if( f ) {
			MarkerCommit();
			if( CardsCount() > 200 ) {
				fTrash = true;
				PlayerMessage("<box action=\"trash\" />",NULL);
				CardsClear();
			}
			break;
		}
		MarkerRollback();
	} while( true );
}

// - OnCommitCard

void TBlackjackRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{

		case MARKER_BLACKJACK:
			PlayerMessage(BoxAction(ssi,"blackjack",p->fBox+1,p->fInfo.ToIntDef(0)),NULL);
			if( p->fBox > -1 )
				Box(p->fBox)->PlayClear();
			break;

		case MARKER_BUST:
			PlayerMessage(BoxAction(ssi,"bust",p->fBox+1,p->fInfo.ToIntDef(0)),NULL);
			if( p->fBox > -1 )
				Box(p->fBox)->PlayClear();
			break;

		case MARKER_SCORE: {
/*
			if( isBlackjack(p->fBox) )
				PlayerMessage(BoxScore(ssi,p->fBox+1,"blackjack"),NULL);
			else {
				TStr ss(p->fInfo);
				PlayerMessage(BoxScore(ssi,p->fBox+1,ss),NULL);
			}
*/
			TStr s(p->fInfo);
			if( p->fInfo == "blackjack" && (p->fBox > 5 || isBoxPlay(p->fBox+5)) )
				s = "21";
			PlayerMessage(BoxScore(ssi,p->fBox+1,s),NULL);
			break;
		}

		case MARKER_DOUBLE:
			PlayerMessage(CardToSSI(ssi,"double",p->fCard,p->fBox+1,p->fInfo.ToIntDef(0),"",0),NULL);
			break;

		case MARKER_HIT:
			PlayerMessage(CardToSSI(ssi,"hit",p->fCard,p->fBox+1),NULL);
			break;

		case MARKET_INSURANCE_WIN:
			PlayerMessage("\n<box id=\""+TStr(p->fBox+1)+"\" action=\"insurance_win\" cash=\""+p->fInfo+"\" />",NULL);
			break;

		case MARKET_INSURANCE_LOSE:
			PlayerMessage("\n<box id=\""+TStr(p->fBox+1)+"\" action=\"insurance_lose\" />",NULL);
			break;

		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnGameStart

void TBlackjackRoom::OnGameStart()
{
	fBox = -1;
	if( fTrash ) {
		CardsClear();
		fTrash = false;
	}
	CardsLoad("cards_blackjack.txt");
	bool isGame = false;

	for( unsigned int i = 0; i < 5; i++ )
		if( Box(i)->Player() != NULL && Box(i)->Bet() > 0 ) {
			isGame = true;
			break;
		}
	fBox = -1;

	if( isGame ) {
		do {
			Marker();
			bool f = true;
			MarkerStep(-1,0,MARKER_CARD);
//			MarkerAction(-1,MARKER_SCORE,IBox(-1)->Score());
			MarkerCard(-1,MARKER_CARD);
			MarkerAction(-1,MARKER_SCORE,IBox(-1)->ScoreAsStr());
			for( unsigned int i = 0; i < 5; i++ ) {
				if( Box(i)->Player() != NULL && Box(i)->Bet() > 0 ) {
					Box(i)->Player()->fTag = 60;
					MarkerCard(i,MARKER_CARD);
					MarkerAction(i,MARKER_SCORE,IBox(i)->ScoreAsStr());
					MarkerCard(i,MARKER_CARD);
					MarkerAction(i,MARKER_SCORE,IBox(i)->ScoreAsStr());
					if( Box(i)->Score() == 21 ) {
						if( !(f = CashTmpAdd(i,Box(i)->Bet()*5/2)) )
							break;
						if( CardID(IBox(-1)->CardLast()) < 10 )
							MarkerAction(i,MARKER_BLACKJACK,Box(i)->Bet()*3/2);
					}
				}
			}
//!!!			if( CardID(IBox(-1)->CardLast()) == 14 )
			if( CardID(IBox(-1)->CardLast()) >= 10 )
				for( unsigned int i = 0; i < 5; i++ ) {
					if( Box(i)->Player() != NULL && Box(i)->Bet() > 0 )
						if( !(f = CashTmpAdd(i,(int)(Box(i)->Bet()*1.5))) )
							break;
				}
			if( f ) {
				MarkerCommit();
				BoxPlay();
				break;
			}
			MarkerRollback();
		} while( true );
	}
	if( fBox == -1 )
		GameEnd();
}

// - OnGame

void TBlackjackRoom::OnGame(
	const TStr&  Action,
	const TStr&,
	IPlayer*     Player)
{
	if( Action == "leave" ) {
		while( fBox > -1 && Box(fBox)->Player() == Player )
			BoxPlay();
	} else
	if( Action == "Stand" || Action == "timer" ) {
		PlayerMessage(BoxAction(ssi,"stand",fBox+1),NULL);
//		if( Action == "timer" )
//			TimerUpdate(45);
		BoxPlay();
	} else {
		int CashBet = 0;
		if( Action == "split" ) {
			if( Box(fBox)->Bet(BOX_INSURANCE) == 0 ) {
				Box(fBox)->Player()->User()->CashBetCommit(cBlackjackGame,Table(),CashBet = Box(fBox)->Bet());
				Box(fBox+5)->Create(Player);
				Box(fBox+5)->BetAdd(CashBet);
				PlayerMessage(BoxAction(ssi,"split",fBox+1,CashBet),NULL);
			}
		} else
		if( Action == "double" ) {
			Box(fBox)->Player()->User()->CashBetCommit(cBlackjackGame,Table(),CashBet = Box(fBox)->Bet());
			Box(fBox)->BetAdd(BOX_DOUBLE,CashBet);
			// !!!
			Box(fBox)->BetAdd(CashBet);
		} else
		if( Action == "Insurance" ) {
			int CashBet = Box(fBox)->Bet() / 2;
			Box(fBox)->BetAdd(BOX_INSURANCE,CashBet);
			if( !isBlackjack(fBox) ) {
				if( CashBet == 0 )
					CashBet = 1;
				if( Box(fBox)->Player()->User()->CashBetCommit(cBlackjackGame,Table(),CashBet) )
					PlayerMessage(BoxAction(ssi,"insurance",fBox+1,CashBet),NULL);
			} else {
				PlayerMessage(BoxAction(ssi,"bjinsurance",fBox+1),NULL);
				BoxPlay();
			}
		}
		int iCount = 0;
		do {
			Marker();
			bool f = true;
			if( Action == "Hit" ) {
				MarkerCard(fBox,MARKER_HIT);
				MarkerAction(fBox,MARKER_SCORE,IBox(fBox)->ScoreAsStr());
			} else
			if( Action == "Double" ) {
				MarkerCard(fBox,MARKER_DOUBLE,CashBet);
				MarkerAction(fBox,MARKER_SCORE,IBox(fBox)->ScoreAsStr());
			} else
			if( Action == "Split" ) {
				MarkerCardMove(fBox+5,fBox);
				MarkerCard(fBox,MARKER_CARD);
				MarkerAction(fBox,MARKER_SCORE,IBox(fBox)->ScoreAsStr());
				MarkerCard(fBox+5,MARKER_CARD);
				MarkerAction(fBox+5,MARKER_SCORE,IBox(fBox+5)->ScoreAsStr());
				f = f && BoxWin(fBox+5,iCount > 100);
			}
			f = f && BoxWin(fBox,iCount > 100);
			if( f ) {
				if( IBox(fBox)->Score() > 21 )
					MarkerAction(fBox,MARKER_BUST);
				MarkerCommit();
				break;
			}
			MarkerRollback();
			iCount++;
		} while( true );
	}
	if( Action == "double" || Action == "Insurance" && IBox(fBox)->Score() == 21 )
		BoxPlay();
	else
	if( Action == "split" ) {
		if(
			CardID(IBox(fBox)->CardFirst()) == 14 &&
			CardID(IBox(fBox+5)->CardFirst())
		) {
			BoxPlay(false);
			BoxPlay();
		} else
			BoxButton();
	}
	if( fBox > -1 )
		while(
			IBox(fBox)->Score() > 21 ||
	//!!!		IBox(fBox)->Score() == 21 && CardID(IBox(-1)->CardLast()) != 14
			IBox(fBox)->Score() == 21 && IBox(fBox)->CardsCount() > 2 ||
			IBox(fBox)->Score() == 21 && IBox(fBox)->CardsCount() == 2 && CardID(IBox(-1)->CardLast()) < 10
		)
			BoxPlay();
	if( fBox == -1 )
		GameEnd();
}

// - OnPlayerAdd

void TBlackjackRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);

//	fTrash = false;
//	CardsClear(312);

	Player->AddMsg("<action name=\"cards\" left=\""+TStr(52*6-CardsCount())+"\" />");
//	Player->AddMsg("<action name=\"cards\" left=\"100\" />");
	if( Mode() == 1 ) {
		ssi->SSIValue("ACTION","show");
		unsigned int i;
		for( i = 0; i < 10; i++ )
			if( Box(i)->Player() != NULL ) {
				// split
				if( i >= 5 ) {
					ssi->SSIValue("BOXID",i+1);
					Box(i)->Player()->User()->ToSSI(ssi);
					Player->AddMsg(ssi->SSIRes("cards_joinuser"));
					ssi->SSIValue("CASH",Box(i)->Bet());
					Player->AddMsg(ssi->SSIRes("cards_bet"));
				}
				// cards
				for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
				{
					ssi->SSIValue("BOXID",i+1);
					ssi->SSIValue("CARD",Box(i)->Card(j));
					bool f = (j == Box(i)->CardsCount()-1);
					ssi->SSIBlock("&isScore",f);
					if( f )
						ssi->SSIValue("SCORE",Box(i)->Score());
					Player->AddMsg(ssi->SSIRes("cards_card"));
				}
			}
		// Dealer
		ssi->SSIValue("BOXID",0);
		ssi->SSIValue("CARD",0);
		Player->AddMsg(ssi->SSIRes("cards_card"));
		for( i = 0; i < IBox(-1)->CardsCount(); i++ )
			ssi->SSIValue("CARD",IBox(-1)->Card(i));
		Player->AddMsg(ssi->SSIRes("cards_card"));
		// Play
		ssi->SSIValue("BOXID",fBox+1);
		Player->AddMsg(ssi->SSIRes("cards_play"));
	}
}

// -

void TBlackjackRoom::BoxPlay( bool code )
{
	bool Find = fBox >= 0;
	for( unsigned int k = 0; k < 10; k++ )
	{
		int i = Mas[k];
		if( Find ) {
			if( i == fBox ) {
				Find = false;
				Box(fBox)->Player()->fTag = 60-Timer();
				if( Box(fBox)->Player()->fTag < 15 )
					Box(fBox)->Player()->fTag = 15;
				fBox = -1;
			}
		} else {
			if( Box(i)->Bet() > 0 && Box(i)->Play() ) {
				fBox = i;
				IPlayer* Player = Box(fBox)->Player();
				TimerStart(60-Player->fTag);
				if( code )
					BoxButton();
				break;
			}
		}
	}
}

void TBlackjackRoom::BoxButton()
{
	ssi->SSIValue("BOXID",fBox+1);
	IPlayer* Player = Box(fBox)->Player();
	TUser*   User = Player->User();
	PlayerMessage(ssi->SSIRes("cards_play"),NULL);
	Player->AddMsg(ssi->SSIRes("cards_button_stand"));
	if( IBox(fBox)->Score() < 21 ) {
		Player->AddMsg(ssi->SSIRes("cards_button_hit"));
		if( User->Cash() >= Box(fBox)->Bet() )
			Player->AddMsg(ssi->SSIRes("cards_button_double"));
		if( isSplit(fBox) ) {
			if( User->Cash() >= Box(fBox)->Bet() )
				Player->AddMsg(ssi->SSIRes("cards_button_split"));
		}
	}
	if( isInsurance(fBox) ) {
		if( User->Cash() >= Box(fBox)->Bet() )
			Player->AddMsg(ssi->SSIRes("cards_button_insurance"));
	}
}

bool TBlackjackRoom::BoxWin(const unsigned int i, const bool)
{
	if( IBox(i)->Score() == 21 ) {
		if( IBox(i)->CardsCount() == 2 && i < 5 )
			return true;
		return CashTmpAdd(i,Box(i)->Bet()*2);
	}
	return true;
}


