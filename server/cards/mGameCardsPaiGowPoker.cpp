#include "mGameCardsPaiGowPoker.h"

#include "server/mUser.h"
#include "server/mServer.h"
#include "server/mCommon.h"

TPaiGowPokerProducer TPaiGowPoker;

// -

bool _BoxHouseCombination(
	unsigned int* Cards,
	FCardsCombination& Combination,
	unsigned int& CombinationHigh,
	const int i = -1)
{
	bool f = true;
	TCardsCombination cc(Cards);
	Combination = cc.Combination();
	if( i > -1 && cc.Hold(i) /*&& CardID(Cards[i]) != 14*/ ) {
		if(
			Combination != CARD_FIVE_OF_A_KIND &&
			Combination != CARD_STRAIGHT_FLUSH &&
			Combination != CARD_ROYAL_FLUSH &&
			Combination != CARD_STRAIGHT &&
			Combination != CARD_FLUSH
		) {
			f = CardID(Cards[i]) == 14;
			if( !f ) {
				Cards[i] = 0;
				do {
					Cards[i] += 13;
					TCardsCombination ll(Cards);
					cc = ll;
					Combination = cc.Combination();
				} while(
					!(Combination != CARD_FIVE_OF_A_KIND &&
					Combination != CARD_STRAIGHT_FLUSH &&
					Combination != CARD_ROYAL_FLUSH &&
					Combination != CARD_STRAIGHT &&
					Combination != CARD_FLUSH) );
			}
//			f = false;
		}
	}
	CombinationHigh = 0;
	for( int j = 0; j < 5; j++ )
		if( Combination == CARD_NOT || cc.Hold(j) )
			if( cc.Cards(j) > CombinationHigh /*&& i != j*/ )
				CombinationHigh = cc.Cards(j);
	return f;
}

int _BoxHouseCombinationJoker(
	unsigned int*      Cards,
	FCardsCombination& Combination,
	unsigned int&      CombinationHigh,
	const bool         SrcChange)
{
	for( int k = 0; k < 5; k++ )
		if( Cards[k] == 53 ) {
			unsigned int JokerCode = 0;
			Combination = CARD_NOT;
			FCardsCombination ccCombination;
			unsigned int      ccCombinationHigh;
//			for( unsigned int i = 13; i < 53; i+=13 )
			for( unsigned int i = 1; i < 53; i++ )
			{
				Cards[k] = i;
				if( _BoxHouseCombination(Cards,ccCombination,ccCombinationHigh,k) ) {
					if(
						ccCombination < Combination ||
						ccCombination == Combination && ccCombinationHigh >= CombinationHigh ||
						i == 1 )
					{
						Combination = ccCombination;
						CombinationHigh = ccCombinationHigh;
						JokerCode = i;
					}
				}
			}
			if( SrcChange )
				Cards[k] = JokerCode;
			else
				Cards[k] = 53;
			return k;
		}
	_BoxHouseCombination(Cards,Combination,CombinationHigh);
	return -1;
}

bool _BoxHouse(
		const unsigned int* Src,
		const unsigned int  i,
		const unsigned int  j,
		const bool          SrcChange)
{
	unsigned int Cards[5];
	unsigned int ii = 0;
	for( unsigned int k = 0; k < 7; k++ )
		if( k != i && k != j )
			Cards[ii++] = Src[k];
	bool Pair = false;
	unsigned int Rang = 0;
	unsigned int ci = CardID(Src[i]);
	unsigned int cj = CardID(Src[j]);
	if( ci == 0 )
		ci = 14;
	if( cj == 0 )
		cj = 14;
//	if( ci == cj || ci == 0 && cj == 14 || cj == 0 && ci == 14 )
	if( ci == cj )
		Pair = true;
	if( ci > cj )
		Rang = ci;
	else
		Rang = cj;
	FCardsCombination Combination;
	unsigned int CombinationHigh;
	_BoxHouseCombinationJoker(Cards,Combination,CombinationHigh,SrcChange);

	bool ccPair = Combination == CARD_PAIR || Combination == CARD_PAIR_TEN;
	TStr ss(TStr(i+1)+"-"+TStr(j+1)+"  ");
	ss +=
		CardsCombinationToStr(Combination)+
		":"+
		CardToStr(CombinationHigh)+
		"  "+
		CardToStr(ci)+
		"-"+
		CardToStr(cj)+" : ";
	if( Pair )
		ss += " pair";
	else
		ss += "     ";
	if( !(
		((Combination == CARD_NOT || Combination == CARD_ACE_KING) && CombinationHigh < Rang) ||
		((Combination == CARD_NOT || Combination == CARD_ACE_KING) && Pair) ||
		(ccPair && Pair && CombinationHigh < Rang)
	) ) {
//		print(ss+"  yes");
		return false;
	}
//	print(ss+"  no");
	return true;
}

class TLine
{
public:
	int               jk;
	unsigned int      fid1,fid2;
	TCardsCombination fHigh;
	TCardsCombination fLow;

	TLine(const unsigned int* Cards, const unsigned int id1, const unsigned int id2) {
		unsigned int _Cards[5];
		unsigned int ii = 0;
		for( unsigned int i = 0; i < 7; i++ )
			if( i != id1 && i != id2 )
				_Cards[ii++] = Cards[i];

		FCardsCombination cc;
		jk = _BoxHouseCombinationJoker(_Cards,cc,ii,true);

		fHigh.Init(_Cards);
		_Cards[0] = Cards[fid1 = id1];
		_Cards[1] = Cards[fid2 = id2];

		if( _Cards[1] == 53 )
			_Cards[1] = 13;
		else
		if( _Cards[0] == 53 )
			_Cards[0] = 13;

		_Cards[2] = _Cards[3] = _Cards[4] = 0;
		fLow.Init(_Cards);
	}
	bool isHigh(TLine *l) {
		if( fHigh.isWin(l->fHigh,false) > 0 && fLow.isWin(l->fLow) > 0 )
			return true;
		return false;
	}
	unsigned int isWin(TLine &l) {
		unsigned int id1 = fHigh.isWin(l.fHigh);
		unsigned int id2 = fLow.isWin(l.fLow);
		if( id1 + id2 > 3 )
			return 2;
		if( id1 == 0 && id2 < 2 || id1 < 2 && id2 == 0 )
			return 0;
		return 1;
	}
};

void HouseWay(const unsigned int* Cards, unsigned int& id1, unsigned int& id2)
{
	TLine* p = NULL;
	m_object::TVector<TLine> line(49,true);
	m_object::TVector<TLine> line_s(49,true);
	unsigned int i;
	unsigned int j;
	for( i = 0; i < 7; i++ )
		for( j = 0; j < 7; j++ )
			if( j > i ) {
				unsigned int _Cards[7];
				for( unsigned int k = 0; k < 7; k++ )
					_Cards[k] = Cards[k];
				if( !_BoxHouse(_Cards,i,j,true) ) {
					line.Add(p = new TLine(Cards,i,j));
//					line_s.Add(new TLine(Cards,i,j));
				}
			}
	for( i = 0; i < line.Count(); i++ )
		print(
			line.Get(i)->fid1,":",
			line.Get(i)->fid2,":",
			CardsCombinationToStr(line.Get(i)->fHigh.Combination()));
	p = NULL;
	for( j = 0; j < line.Count(); j++ )
	{
		bool f = p == NULL;
		if( p != NULL ) {
			f =
				line.Get(j)->fHigh.isWin(p->fHigh,false) == 2 ||
				line.Get(j)->fHigh.isWin(p->fHigh,false) == 1 &&
				line.Get(j)->fLow.isWin(p->fLow,true) > 1;
/*
			print(
				CardsCombinationToStr(p->fHigh.Combination()),":",CardsCombinationToStr(p->fLow.Combination()),
				CardsCombinationToStr(line.Get(j)->fHigh.Combination()),":",CardsCombinationToStr(line.Get(j)->fLow.Combination())
				);
*/
		}
		if( f ) {
			TLine* p = line.Get(j);
			bool fLowPair =
				p->fLow.Combination() == CARD_PAIR ||
				p->fLow.Combination() == CARD_PAIR_TEN;
			switch( p->fHigh.Combination() )
			{
				case CARD_TWO_PAIR: {
					int r1 = p->fHigh.CardsSort(1);
					int r2 = p->fHigh.CardsSort(3);
					if( !fLowPair ) {
						// Нет дополнительно Туза:
						//   если обе пары меньше Семерок, то они составляют старшую руку,
						//   хотя бы одна из пар Семерки или выше - младшую пару на младшую руку;
						// Есть еще Туз:
						//   если обе пары меньше Валетов, то они составляют старшую руку,
						//   хотя бы одна из пар Валеты или выше - младшую пару на младшую руку.
						bool fAce =
							p->fLow.Cards(0) == 14 ||
							p->fLow.Cards(1) == 14;
						if( !fAce && (r1 > 6 || r2 > 6) )
							f = false;
						else
						if( fAce && (r1 > 10 || r2 > 10 ) )
							f = false;
						else
						if( r1 == 14 || r2 == 14 )
							f = false;
					} else {
						int r = p->fLow.CardsSort(4);
						// Три пары.
						//   Старшая пара - на младшую руку
						if( r < r1 || r < r2 )
							f = false;
					}
					break;
				}
				case CARD_THREE_OF_A_KIND:
					// Тройка.
					// Всегда играет на старшей руке, за исключением трех Тузов.
					// В этом случае пара Тузов на старшей, а третий Туз со следующей по старшинству картой - на младшую руку.
					if( fLowPair ) {
						// Две тройки поиск
						for( int ii = 0; ii < 5; ii++ )
							if( p->fHigh.CardsSort(ii) == p->fLow.CardsSort(4) ) {
								if( p->fLow.CardsSort(4) < p->fHigh.CardsSort(3) )
									f = false;
								break;
							}
					} else
					if( p->fHigh.CardsSort(4) == 14 )
						f = false;
					else
					if(
						p->fHigh.CardsSort(3) == p->fLow.CardsSort(4) ||
						p->fHigh.CardsSort(3) == p->fLow.CardsSort(3)
					)
						f = false;
					break;
				case CARD_FULL_HOUSE:
					f = false;
					break;
				case CARD_FOUR_OF_A_KIND:
					// Каре (Four-of-a-kind).
					// меньше Семерок - каре играет в старшей руке;
					// от Семерок до Десяток - каре в старшей руке, если на младшую можно положить Туза с Королем, Дамой или Валетом, или же любую пару. В остальных случаях - разбивать на две пары;
					// старше Десяток - разбивать на две пары, если нет дополнительной пары от Семерок или выше.
					if( !fLowPair ) {
						if(
							p->fHigh.CardsSort(4) > 6 &&
							p->fHigh.CardsSort(4) < 11
						) {
							if( p->fLow.CardsSort(3) < 10 )
								f = false;
						} else
							f = false;
					}
					break;
				case CARD_FLUSH:
				case CARD_STRAIGHT:
				case CARD_STRAIGHT_FLUSH:
				case CARD_ROYAL_FLUSH:
					// Стрэйт, флэш, стрэйт-флэш, ройял-флэш.
					// Играть как старшую руку, за исключением случаев:
					//   имеется две пары от Десяток и выше - играть как две пары;
					//   имеется пара Тузов и любая другая пара - играть как две пары;
					//   имеется две пары меньше Десяток и дополнительный Туз - играть как две пары;
					//   имеется тройка - играть как тройку;
					//   имеется три пары (возможно только при Джокере и Тузе, например, A Дж 5 5 4 4 2) - играть как три пары;
					//   имеется фулл хаус (возможно только при Джокере и Тузе, например, A Дж 5 5 5 3 2) - играть как фулл хаус;
					//   Если возможны одновременно несколько из этих комбинаций, то на старшей руке оставляется та, которая обеспечивает наиболее сильную младшую руку. Например, ЄA K Q J 10 §9 Ё8 играть стрэйт (Q J 10 9 8) на старшей руке, а AK - на младшей.
					//   То же самое при комбинациях длиной 6 или 7 карт.
					// Искать пары и тройки
					int ip = 0;
					for( unsigned int i = 0; i < 7; i++ )
					{
						int ii = 0;
						for( unsigned int j = i+1; j < 7; j++ )
							if( CardID(Cards[i]) == CardID(Cards[j]) )
								ii++;
						if( ii > 1 ) {
							f = false;
							break;
						}
						if( ii == 1 )
							ip++;
						if( ip >= 2 ) {
							f = false;
							break;
						}
					}
					break;
			}
		}
		if( f ) {
			p = line.Get(j);
/*
			print(
				">>> ",
				CardsCombinationToStr(p->fHigh.Combination()),
				":",
				CardsCombinationToStr(p->fLow.Combination()));
*/
		}
	}
	if( p == NULL )
		p = line.Get(0);
	id1 = p->fid1;
	id2 = p->fid2;
/*
	for( unsigned int i = 0; i < line.Count(); i++ )
		for( unsigned int j = i+1; j < line.Count(); j++ )
			if( !line.Get(i)->isHigh(line.Get(j)) )
				line.Swap(i,j);
	id1 = line.Get(0)->fid1;
	id2 = line.Get(0)->fid2;
*/
}

// - LOG

TStr TPaiGowPokerRoom::Log(IPlayer* p)
{
	TStr s;
	for( unsigned int i = 0; i < 3; i++ )
		if( Box(i)->Player() == p && isBoxBet(i) ) {
			for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ )
				if( j != ID1[i] && j != ID2[i] )
					s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(Box(i)->Card(j))+"\" />";
			s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(IBox(i)->Card(ID1[i]))+"\" />";
			s += "<card id=\""+TStr(i+1)+"\" type=\""+TStr(IBox(i)->Card(ID2[i]))+"\" />";
			break;
		}
	if( !s.isEmpty() ) {
		for( unsigned int j = 0; j < IBox(-1)->CardsCount(); j++ )
			if( j != _ID1 && j != _ID2 )
				s += "<card type=\""+TStr(IBox(-1)->Card(j))+"\" />";
		s += "<card type=\""+TStr(IBox(-1)->Card(_ID1))+"\" />";
		s += "<card type=\""+TStr(IBox(-1)->Card(_ID2))+"\" />";
	}
	return s;
}

// -- OnCommitCard

void TPaiGowPokerRoom::OnCommitCard( TStep* p )
{
	switch( p->fAction )
	{
		case MARKER_WIN: case MARKER_LOSE: case MARKER_PUSH:
				Box(p->fBox)->BetAdd(BOX_WIN,p->fInfo.ToIntDef(0));
				TCardsRoom::OnCommitCard(p);
			break;
		case MARKER_HOUSE: {
			if( p->fBox == -1 ) {
				for( unsigned int i = 0; i < 7; i++ )
					if( i != _ID1 && i != _ID2 )
						PlayerMessage(CardToSSI(ssi,"open",IBox(-1)->Card(i),0),NULL);
					PlayerMessage(CardToSSI(ssi,"open",IBox(-1)->Card(_ID1),0),NULL);
					PlayerMessage(CardToSSI(ssi,"open",IBox(-1)->Card(_ID2),0),NULL);
			} else
				Box(p->fBox)->Player()->AddMsg(BoxHouse(p->fBox));
			break;
		}
		case MARKER_OPEN:
			if( p->fBox > -1)
				Box(p->fBox)->Player()->AddMsg(CardToSSI(ssi,"open",p->fCard,p->fBox+1));
/*
			else
				TCardsRoom::OnCommitCard(p);
*/
			break;
		default:
			TCardsRoom::OnCommitCard(p);
	}
}

// - OnTagPlay

void TPaiGowPokerRoom::OnTagPlay(IPlayer *Player)
{
	Player->AddMsg(ssi->SSIRes("cards_button_done"));
	Player->AddMsg(ssi->SSIRes("cards_button_house"));
}

// - OnGameStart

void TPaiGowPokerRoom::OnGameStart()
{
	CardsClear();
	CardsLoad("cards_paigow.txt");

	bool isGame = false;
	for( unsigned int i = 0; i < 3; i++ ) {
		if( Box(i)->Player() != NULL && Box(i)->Bet() > 0 ) {
			isGame = true;
			break;
		}
	}
	if( isGame ) {
		TimerStart();
		PlayerMessage(ssi->SSIRes("cards_delivery"),NULL);
		do {
			bool f = true;
/*
			for( unsigned int i = 0; i < 7; i++ )
				MarkerCard(-1,CLOSE);
			HouseWay(IBox(-1)->Cards(),_ID1,_ID2);
			TLine DealerCombination(IBox(-1)->Cards(),_ID1,_ID2);
*/
			for( unsigned int i = 0; i < 3; i++ )
				if( isBoxPlay(i) ) {
					for( unsigned int j = 0; j < 7; j++ )
						MarkerCard(i,MARKER_OPEN);
					HouseWay(IBox(i)->Cards(),ID1[i],ID2[i]);
/*
					TLine BoxCombination(IBox(i)->Cards(),ID1[i],ID2[i]);
					if( BoxCombination.isWin(DealerCombination) ) {
						if( !(f = CashTmpAdd(i,Box(i)->Bet()*1.95)) )
							break;
						MarkerAction(i,MARKER_TMP,Box(i)->Bet()*1.95));
					}
*/
					MarkerAction(i,MARKER_HOUSE);
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

void TPaiGowPokerRoom::OnGame(
	const TStr& Action,
	const TStr& ID,
	IPlayer* Player)
{
	if( Action == "leave" )
		Player->fTag = -1;
	else
	if( Action == "Timer" ) {
		for( unsigned int i = 0; i < PlayerCount(); i++ )
			if( TCardsRoom::Player(i)->fTag > -1 )
				TCardsRoom::Player(i)->fTag = -2;
//			TCardsRoom::Player(i)->fTag = -1;
	} else
	if( Player->fTag > -1 ) {
		TStr sID1 = ID.CopyBefore(":");
		TStr sID2 = ID.CopyAfter(":");
		if( !sID1.isEmpty() && !sID2.isEmpty() ) {
			ID1[Player->fTag] = sID1.ToInt()-1;
			ID2[Player->fTag] = sID2.ToInt()-1;
		}
		TagPlayNext(Player,3);
	}
	if( TagGameEnd() ) {
		unsigned int i;
		for( i = 0; i < 3; i++ )
			if( isBoxPlay(i) ) {
				for( unsigned int j = 0; j < Box(i)->CardsCount(); j++ ) 
					PlayerMessage(CardToSSI(ssi,"open",Box(i)->Card(j),i+1),Box(i)->Player());
				IPlayer* Player = Box(i)->Player();
				if( Player->fTag == -2 ) {
					TLine BoxCombination(IBox(i)->Cards(),ID1[i],ID2[i]);
					Player = NULL;
				}
				ssi->SSIValue("ACTION","houseup");
				ssi->SSIValue("BOXID",i+1);
				ssi->SSIValue("CARDID1",ID1[i]+1);
				ssi->SSIValue("CARDID2",ID2[i]+1);
				PlayerMessage(ssi->SSIRes("cards_action_cards"),Player);
			}
		do {
			Marker();
			for( i = 0; i < 7; i++ )
				MarkerCard(-1,MARKER_OPEN);
			HouseWay(IBox(-1)->Cards(),_ID1,_ID2);
			MarkerAction(-1,MARKER_HOUSE);
			TLine DealerCombination(IBox(-1)->Cards(),_ID1,_ID2);
			bool f = true;
			for( i = 0; i < 3; i++ )
				if( isBoxPlay(i) ) {
					TLine BoxCombination(IBox(i)->Cards(),ID1[i],ID2[i]);
					switch( BoxCombination.isWin(DealerCombination) )
					{
						case 2: {
								unsigned int Cash = Box(i)->Bet()*195/100;
								if( Cash-Box(i)->Bet(BOX_TMP) > 0 )
									f = CashTmpAdd(i,Cash-Box(i)->Bet(BOX_TMP));
								MarkerAction(i,MARKER_WIN,Cash);
							}
							break;
						case 1: {
								unsigned int Cash = Box(i)->Bet();
								if( Cash-Box(i)->Bet(BOX_TMP) > 0 )
									f = CashTmpAdd(i,Cash-Box(i)->Bet(BOX_TMP));
								MarkerAction(i,MARKER_PUSH,Cash);
							}
							break;
						case 0:
							MarkerAction(i,MARKER_LOSE);
							break;
					}
					if( !f )
						break;
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

void TPaiGowPokerRoom::OnPlayerAdd(IPlayer *Player)
{
	TCardsRoom::OnPlayerAdd(Player);
	if( Mode() == 1 ) {
		Player->fTag = -1;
		Player->AddMsg(ssi->SSIRes("cards_delivery_show"));
	}
}

// -

TStr TPaiGowPokerRoom::BoxHouse(const int BoxID)
{
	TStr s;
	const unsigned int *Src = Box(BoxID)->Cards();
	ssi->SSIValue("BOXID",BoxID+1);
	ssi->SSIValue("ACTION","houseno");
	for( unsigned int i = 0; i < 7; i++ )
		for( unsigned int j = 0; j < 7; j++ )
			if( j > i ) {
				if( _BoxHouse(Src,i,j,false) ) {
					ssi->SSIValue("CARDID1",i+1);
					ssi->SSIValue("CARDID2",j+1);
					s += ssi->SSIRes("cards_action_cards");
				}
			}
	ssi->SSIValue("ACTION","house");
	unsigned int id1,id2;
	HouseWay(Src,id1,id2);
	ssi->SSIValue("CARDID1",id1+1);
	ssi->SSIValue("CARDID2",id2+1);
	s += ssi->SSIRes("cards_action_cards");
	return s;
}

