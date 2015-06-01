#include "mGameCardsCaribbeanPoker.h"

#include "server/mUser.h"

TCaribbeanPokerProducer CaribbeanPoker;

TPokerCombination* TCaribbeanPokerRoom::fBonus;
TPokerCombination* TCaribbeanPokerRoom::fGame;


static TPokerCombination::TCombination CombinationGame[12] = {
	{  1, 100 },
	{  2, 50  },
	{  3, 20  },
	{  4, 7   },
	{  5, 5   },
	{  6, 4   },
	{  7, 3   },
	{  8, 2   },
	{  9, 1   },
	{ 10, 1   },
	{ 11, 1   },
	{ 12, 1   }
};

static TPokerCombination::TCombination CombinationBonus[7] = {
	{ 1, 5000 },
	{ 2, 1000 },
	{ 3, 250  },
	{ 4, 100  },
	{ 5, 50   },
	{ 6, 40   },
	{ 7, 20   }
};

void TCaribbeanPokerRoom::Init()
{
	fGame = new TPokerCombination();
	fBonus = new TPokerCombination();
	fGame->Init(CombinationGame,12);
	fBonus->Init(CombinationBonus,7);
}

void TCaribbeanPokerRoom::Free()
{
	delete fGame;
	delete fBonus;
}

// - LOG

TStr TCaribbeanPokerRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int i = 0; i < 5; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
//			for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
			for( unsigned int j = 0; j < 5; j++ )
				s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(Box(i)->Card(j))+"\" />";
			TCardsCombination BoxCombination(Box(i)->Cards());
			TStr Result(CardsCombinationAsStr(BoxCombination.Combination()));
			if( Box(i)->Bet(BOX_WIN) == 0 )
				Result = "lost";
			s += "<box id=\""+TStr(i+1)+"\" ante=\""+TStr(Box(i)->Bet(BOX_ANTE))+"\" result=\""+Result+"\" />";
		}
	if( !s.isEmpty() )
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
	return s;
}

// -- OnCommitCard

void TCaribbeanPokerRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{
		case MARKER_PUSH:
			break;
		case MARKER_WINGAME: case MARKER_WINBONUS:
			Box(p->fBox)->BetAdd(BOX_WIN,p->fInfo.ToIntDef(0));
			break;
		case MARKER_COMBINATION:
			ssi->SSIValue("BOXID",p->fBox+1);
			ssi->SSIValue("SCORE",p->fParam);
			PlayerMessage(ssi->SSIRes("cards_action_score"),NULL);
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

void TCaribbeanPokerRoom::OnTagPlayNext(IPlayer *Player)
{
	if( Player->fTag > -1 ) {
		ssi->SSIValue("BOXID",Player->fTag+1);
		Player->AddMsg(ssi->SSIRes("cards_action_close"));
	}
}

void TCaribbeanPokerRoom::OnBoxBonus(const unsigned int i, const bool)
{
	ssi->SSIValue("BOXID",i+1);
	if( Box(i)->Player() != NULL && Box(i)->Bet(BOX_BONUS) > 0 ) {
		if( Box(i)->Bet(BOX_WIN) > 0 ) {
			ssi->SSIBlock("&isCash",true);
			ssi->SSIValue("ACTION","bonuswin");
			ssi->SSIValue("CASH",Box(i)->Bet(BOX_WIN));
		} else {
			ssi->SSIBlock("&isCash",false);
			ssi->SSIValue("ACTION","bonuslose");
		}
//		if( f )
				PlayerMessage(ssi->SSIRes("cards_action"),NULL);
//		else
//			Box(i)->Player()->AddMsg(ssi->SSIRes("cards_action"));
	}
}

void TCaribbeanPokerRoom::OnTagPlay(IPlayer *Player)
{
	Player->AddMsg(CardsToSSI(ssi,"open",Box(Player->fTag)->Cards(),5,Player->fTag+1));
	Player->AddMsg(BoxCombination(ssi,Player->fTag+1,Box(Player->fTag)->Cards()));
	OnBoxBonus(Player->fTag,false);
	Player->AddMsg(ssi->SSIRes("cards_play"));
	if( Player->User()->Cash() >= Box(Player->fTag)->Bet(BOX_ANTE)*2 )
		Player->AddMsg(ssi->SSIRes("cards_button_raise"));
	Player->AddMsg(ssi->SSIRes("cards_button_surrender"));
}

// - OnGameStart

void TCaribbeanPokerRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_caribbeanpoker.txt");
	TagInit();
	bool isGame = false;
	for( unsigned int i = 0; i < 5; i++ )
		if( isBoxPlay(i) ) {
			isGame = true;
			break;
		}
	if( isGame ) {
		PlayerMessage(ssi->SSIRes("cards_delivery"),NULL);
		do {
			Marker();
			MarkerCard(-1,MARKER_OPEN);
			bool f = true;
			for( unsigned int i = 0; i < 5; i++ )
				if( isBoxPlay(i) ) {
					for( unsigned int j = 0; j < 5; j++ )
						MarkerCard(i,MARKER_CLOSE);
					TCardsCombination BoxCombination(Box(i)->Cards());
					if( BoxCombination.Combination() != CARD_NOT ) {
						unsigned int Cash = Box(i)->Bet(BOX_ANTE) * 4 * fGame->Cash(BoxCombination.Combination());
						if( !(f = CashTmpAdd(i,Cash)) )
							break;
						MarkerAction(i,MARKER_TMP,Cash);
					}
					unsigned int Cash =
						Box(i)->Bet(BOX_BONUS) *
						fBonus->Cash(BoxCombination.Combination());
					if( BoxCombination.Combination() == CARD_THREE_OF_A_KIND ) {
						if( BoxCombination.CardsSort(4) != 14 )
							Cash = 0;
						else
						if( !BoxCombination.Hold(BoxCombination.CardsSortIndex(4)) )
							Cash = 0;
					}
					if( Cash > 0 ) {
						if( !(f = CashTmpAdd(i,Cash)) )
							break;
						MarkerAction(i,MARKER_WINBONUS,Cash);
					}
				}
			if( f ) {
				MarkerCommit();
				break;
			}
			MarkerRollback();
		} while( true );
		TimerStart();
		TagPlayNext(5);
	} else
		GameEnd();
}

// - OnGame

void TCaribbeanPokerRoom::OnGame(
	const TStr& Action,
	const TStr& ID,
	IPlayer* Player)
{
	if( Action == "leave" )
		Player->fTag = -1;
	else
	if( Action == "Change" ) {
		int CashBet = Box(Player->fTag)->Bet(BOX_ANTE);
		if( Player->User()->CashBetCommit(cCaribbeanPokerGame,Table(),CashBet) ) {
			unsigned int CardIndex = ID.ToInt();
			Marker();
			MarkerCard(Player->fTag,MARKER_OPEN);
			MarkerCardChange(Player->fTag,5-CardIndex);
			Player->AddMsg(CardToSSI(ssi,"change",Box(Player->fTag)->Card(5-CardIndex),Player->fTag+1,CashBet,"",CardIndex));
			TCardsCombination BoxCombination(Box(Player->fTag)->Cards());
			ssi->SSIValue("BOXID",Player->fTag+1);
			ssi->SSIValue("SCORE",CardsCombinationAsStr(BoxCombination.Combination()));
			Player->AddMsg(ssi->SSIRes("cards_action_score"));
			MarkerCommit();
		}
	} else
	if( Action == "Raise" ) {
		int CashBet = Box(Player->fTag)->Bet(BOX_ANTE)*2;
		if( Player->User()->CashBetCommit(cCaribbeanPokerGame,Table(),CashBet) ) {
			Box(Player->fTag)->BetAdd(BOX_BET,CashBet);
			PlayerMessage(BoxAction(ssi,"raise",Player->fTag+1,CashBet),NULL);
			TagPlayNext(Player,5);
		}
	} else
	if( Action == "surrender" ) {
		OnBoxBonus(Player->fTag,true);
		PlayerMessage(BoxAction(ssi,"surrender",Player->fTag+1),NULL);
		TagPlayNext(Player,5);
	} else
	if( Action == "Timer" ) {
		bool f = false;
		for( unsigned int i = 0; i < PlayerCount(); i++ )
		{
			if( TCardsRoom::Player(i)->fTag > -1 )
			{
				PlayerMessage(BoxAction(ssi,"surrender",TCardsRoom::Player(i)->fTag+1),NULL);
				if( TagPlayNext(TCardsRoom::Player(i),5) )
					f = true;
			}
		}
		if( f )
			TimerUpdate();
	}

	if( Player->fTag > -1 && Timer() > 45 )
		TimerUpdate();

	if( TagGameEnd() ) {
		for( unsigned int i = 0; i < 5; i++ )
		{
			if( isBoxBet(i) ) {
				PlayerMessage(CardsToSSI(ssi,"open",Box(i)->Cards(),5,i+1),NULL);
				PlayerMessage(BoxCombination(ssi,i+1,Box(i)->Cards()),NULL);
			}
			OnBoxBonus(i,true);
		}
		do {
			Marker();
			bool f = true;
			unsigned int i;
			for( i = 0; i < 4; i++ )
				MarkerCard(-1,MARKER_OPEN);
			TCardsCombination DealerCombination(IBox(-1)->Cards());
			MarkerAction(-1,MARKER_COMBINATION)->fParam = CardsCombinationAsStr(DealerCombination.Combination());
			for( i = 0; i < 5; i++ )
			{
				if( isBoxBet(i) ) {
					unsigned int CashWin = 0;
					unsigned int Cash = 0;
					unsigned int Action = MARKER_LOSE;
					if( Box(i)->Bet(BOX_BET) > 0 ) {
						if( DealerCombination.Combination() == CARD_NOT ) {
							Cash = Box(i)->Bet(BOX_BET);
							CashWin = Box(i)->Bet(BOX_ANTE) * 2 + Cash;
							Action = MARKER_WIN;
						} else {
							int MaxWin = 0-Box(i)->Bet(BOX_ANTE)*2-Box(i)->Bet(BOX_BET);
							switch( Table() )
							{
								case 1: MaxWin += 10000; break;
								case 2: MaxWin += 500000; break;
								case 3: MaxWin += 1000000; break;
							}
							TCardsCombination BoxCombination(Box(i)->Cards());
							switch( DealerCombination.isWin(BoxCombination) )
							{
								case 0: {
										Action = MARKER_WIN;
										int c = Box(i)->Bet(BOX_BET) * fGame->Cash(BoxCombination.Combination());
										if( c > MaxWin )
											c = MaxWin;
										Cash = c + Box(i)->Bet(BOX_BET);
										CashWin = Box(i)->Bet(BOX_ANTE)*2 + Cash;
									break;
								}
								case 1:
										Action = MARKER_PUSH;
										Cash = Box(i)->Bet(BOX_BET);
										CashWin = Box(i)->Bet(BOX_ANTE) + Cash;
									break;
							}
						}
					}
					int CashWinTmp = CashWin - Box(i)->Bet(BOX_TMP);
					if( CashWinTmp > 0 ) {
						if( !(f = CashTmpAdd(i,CashWinTmp)) )
							break;
					}
					MarkerAction(i,Action,Cash);
					if( CashWin > 0 )
						MarkerAction(i,MARKER_WINGAME,CashWin);
				}
			}
			if( f ) {
				MarkerCommit();
				break;
			}
			MarkerRollback();
		} while( true );
		GameEnd();
	}
}

// - OnPlayerAdd

void TCaribbeanPokerRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);
	if( Mode() == 1 ) {
		Player->fTag = -1;
		Player->AddMsg(ssi->SSIRes("cards_delivery_show"));
		ssi->SSIValue("CARD",IBox(-1)->Card(0));
		ssi->SSIValue("BOXID",0);
		ssi->SSIValue("ACTION","open");
		Player->AddMsg(ssi->SSIRes("cards_card"));
	}
}
