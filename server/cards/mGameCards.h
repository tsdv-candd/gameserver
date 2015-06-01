#ifndef mGameCardsH
#define mGameCardsH

#include "server/mGame.h"
#include "server/mUser.h"

#include "utils/m_object.h"

#include <stdio.h>

class TPokerCombination
{
public:
	struct TCombination {
		unsigned int fID;
		unsigned int fCash;
	};
protected:
	m_object::TVector<TCombination> *fData;
public:
	TPokerCombination() {
		fData = new m_object::TVector<TCombination>(100,true);
	}
	virtual ~TPokerCombination() {
		delete fData;
	}
	unsigned int Cash( const unsigned int ID ) {
		for( unsigned int i = 0; i < fData->Count(); i++ )
			if( fData->Get(i)->fID == ID )
				return fData->Get(i)->fCash;
		return 0;
	}
	void Init(TCombination* m, const int Count) {
		fData->Clear();
		for( int i = 0; i < Count; i++ )
			fData->Add(new TCombination(m[i]));
	}
};



class TIBox
{
friend class TCardsRoom;
friend class TBoxTable;
private:

	unsigned int fCards[20];
	unsigned int fCardsCount;

private:

	void         SetCard(const unsigned int CardID) {
		fCards[fCardsCount++] = CardID;
	}
	void         SetCard(const unsigned int Index, const unsigned int CardID) { fCards[Index] = CardID; }
	unsigned int DelCard() { fCardsCount--; return fCards[fCardsCount]; }

public:

	TIBox(): fCardsCount(0) {}
	virtual ~TIBox() {}

	virtual void GameClear() {
		for(unsigned int i = 0; i < 20; i++ )
			fCards[i] = 0;
		fCardsCount = 0;
	}

	unsigned int  CardsCount() const { return fCardsCount; }
	unsigned int  Card(const unsigned int i) const { return fCards[i]; }
	unsigned int  CardLast() const {
		if( fCardsCount == 0 )
			return 0;
		return fCards[fCardsCount-1];
	}
	unsigned int  CardFirst() const {
		if( fCardsCount == 0 )
			return 0;
		return fCards[0];
	}

	const unsigned int* Cards() { return fCards; }

	TStr ScoreAsStr() const {
		TStr s;
		for( unsigned int i = 0; i < fCardsCount; i++ )
			if( CardPrice(fCards[i]) == 11 ) {
				int r1, r2;
				CardsScore(fCards,fCardsCount,r1,r2);
				if( r1 <= 21 && r2 <= 21 )
					s = TStr(r1) + "/" + TStr(r2);
				break;
			}
		if( s.isEmpty() )
			s = CardsScore(fCards,fCardsCount);
		if( fCardsCount == 2 && s == "11/21" )
			s = "blackjack";
		return s;
	}
	unsigned int Score() const {
		return CardsScore(fCards,fCardsCount);
	}
//	TStr ScoreAsStr() const { return CardsScoreAsStr(fCards,fCardsCount); }

};


class TBox: public TIBox
{
private:

	IPlayer *fPlayer;

	int fCash[6];

	bool fPlay;
	bool fDel;

public:

	TBox():fPlayer(NULL) { }

	void Create( IPlayer *Player, const bool Del = true ) {
		fPlayer = Player;
		fDel = Del;
		GameClear();
	}

	void Clear() { fPlayer = NULL; }

	IPlayer* Player() const { return fPlayer; }

	void GameClear() {
		for( unsigned int i = 0; i < 6; i++ )
			fCash[i] = 0;
		fPlay = true;
		TIBox::GameClear();
	}

	void PlayClear() { fPlay = false; }
	bool Play() { return fPlay && Player() != NULL; }
	bool Del() { return fDel; }

	void BetAdd(const int Cash) {
		BetAdd(0,Cash);
	}
	void BetAdd(const unsigned int Index, const int Cash) {
		fCash[Index] += Cash;
		if( fCash[Index] < 0 )
			fCash[Index] = 0;
	}
	unsigned int Bet(const unsigned int Index = 0) const {
		return fCash[Index];
	}
};

extern const unsigned int NUM[5];

class TBoxTable
{
private:

	TIBox fDealerBox;
	TBox *fBox[10];
	unsigned int fBoxCount;

protected:

	virtual const char* BoxStr(const unsigned int Index) { return ""; }

	bool isBoxSingle() {
	for( unsigned int i = 0; i < 4; i++ )
		if( BoxStr(i)[0] != '\0' )
			return false;
	return true;
	}

	bool isBoxBet(const unsigned int i) {
		if( Box(i)->Player() == NULL )
			return false;
		for( unsigned int j = 0; j < 4; j++ )
			if( Box(i)->Bet(j) > 0 )
				return true;
		return false;
	}

	virtual unsigned int BoxCount() { return fBoxCount; }

	virtual unsigned int TableID() = NULL;

public:

	TBoxTable(const unsigned int BoxCount): fBoxCount(BoxCount) {
		for( unsigned int i = 0; i < 10; i++ )
			fBox[i] = new TBox();
	}

	virtual ~TBoxTable() {
		for( unsigned int i = 0; i < 10; i++ )
			delete fBox[i];
	}

	int Add(IPlayer *Player) {
		for( unsigned int j = 0; j < fBoxCount; j++ )
		{
			unsigned int i = NUM[j];
			if( fBox[i]->Player() == NULL ) {
				fBox[i]->Create(Player,false);
				return i;
			}
		}
		return -1;
	}

	unsigned int Del(IPlayer *Player, unsigned int ID[5]) {
		unsigned int count = 0;
		for( unsigned int i = 0; i < 10; i++ )
			if( fBox[i]->Player() == Player ) {
				fBox[i]->Clear();
				ID[count++] = i;
			}
		return count;
	}

	int Bet(
			const unsigned int i,
			IPlayer *Player,
			const int Cash,
			const TStr& Bet)
	{
		int r = 1;
		if( fBox[i]->Player() == NULL ) {
			fBox[i]->Create(Player,true);
			r = 2;
		}
		if( fBox[i]->Player() == Player ) {
			if( Player->User()->CashBetAdd(TableID(),Cash) ) {
				unsigned int j = 0;
				while( j < 4 )
				{
					if( BoxStr(j)[0] == '\0' )
						break;
					if( Bet == BoxStr(j) )
						break;
					j++;
				}
				if( j == 4 )
					j = 0;
				fBox[i]->BetAdd(j,Cash);
				if( fBox[i]->Bet(0) == 0 && fBox[i]->Bet(1) == 0 && fBox[i]->Bet(2) == 0 && fBox[i]->Bet(3) == 0 )
					r = 3;
			}
			return r;
		}
		return 0;
	}

	void Clear(const unsigned int i) {
		fBox[i]->Clear();
	}

	TBox* Box(const unsigned int Index) {
		return fBox[Index];
	}

	TIBox* IBox(int Index) {
		if( Index < 0 )
			return &fDealerBox;
		return fBox[Index];
	} 

	int BoxFind(IPlayer *Player, int i = -1) {
		for( i++; i < 5; i++ )
			if( Box(i)->Player() == Player )
				return i;
		return -1;
	}

};

const unsigned int BOX_WIN = 3;
const unsigned int BOX_TMP = 4;

class TCardsRoom: public IPlayerRoom, public TBoxTable
{
public:

	virtual bool isLeave(TUser* p) {
		if( Mode() == 1 )
			for( unsigned int i = 0; i < BoxCount(); i++ )
				if( Box(i)->Player() != NULL && Box(i)->Player()->User() == p && isBoxBet(i) )
					return false;
		return true;
	}

public:

enum EAction {
	MARKER_CARD = 1,
	MARKER_OPEN,
	MARKER_LOSE,
	MARKER_WIN,
	MARKER_PUSH,
//	MARKER_BOX_CARD_CHANGE,
	MARKER_BOX_CARD_MOVE,
	MARKER_TMP,
	MARKER_NOT
};

protected:

	class TStep;

private:

	unsigned int* fCards;
	unsigned int  fCardsSize;
	unsigned int  fCardsCount;

	unsigned int  iCards,iCardsCount;

	TStep*        fMarker;
	unsigned int  fMarkerCount;

protected:

	unsigned int  iRollback;

	virtual bool isNewsGameButton() { return true; }

	bool isGame();

	virtual TStr Log(IPlayer*) = NULL;

	virtual unsigned int GameID() = NULL;

	class TStep {
	public:
		unsigned int fCard;
		int          fBox;
		TStep*       fNext;

		unsigned int fAction;
		TStr         fInfo;

		unsigned int fBet;
		TStr         fParam;

		TStep(): fInfo(), fAction(MARKER_NOT), fBox(-100), fCard(0), fNext(NULL) {}

		void Add(TStep *p) {
			if( fNext != NULL )
		fNext->Add(p);
			else
				fNext = p;
		}
	};

	virtual void OnMarker() {
		for( unsigned int i = 0; i < PlayerCount(); i++ )
		{
			Player(i)->User()->CashTmpRollback();
			Player(i)->User()->CashResRollback();
		}
	}

	virtual void OnCommitBegin() {}

	virtual void OnCommitEnd() {
		for( unsigned int i = 0; i < PlayerCount(); i++ )
			{
				Player(i)->User()->CashTmpCommit();
/*
				if( Mode() == 2 ) {
					Player(i)->User()->CashResCommit(GameID());
					ssi->SSIValue("CASHWIN",Player(i)->User()->CashWinTotal());
					ssi->SSIValue("CASHBET",Player(i)->User()->CashBetCommit());
					Player(i)->AddMsg(ssi->SSIRes("cards_game"));
					Player(i)->User()->CashWinCommit(Log(Player(i)));
				}
*/
			}
	}

	virtual void OnCommitCard(TStep* p);

	void Marker() {
		if( fMarker != NULL )
			MarkerRollback();
		OnMarker();
		fMarkerCount = fCardsCount;
	}

	void MarkerCommit() {
		iRollback = 0;
		OnCommitBegin();
		while( fMarker != NULL )
		{
			OnCommitCard(fMarker);
			TStep* Next = fMarker->fNext;
			delete fMarker;
			fMarker = Next;
		}
		fMarkerCount = 0;
		OnCommitEnd();
	}

	void MarkerRollback() {
		iRollback++;
		while( fMarker != NULL )
		{
			TStep* Next = fMarker->fNext;
			switch( fMarker->fAction )
			{
				case MARKER_BOX_CARD_MOVE: {
					int Box = fMarker->fInfo.ToIntDef(0);
//					IBox(Box)->SetCard(IBox(fMarker->fBox)->CardLast());
					IBox(fMarker->fBox)->DelCard();
				} break;
/*
				case MARKER_BOX_CARD_CHANGE: {
					IBox(fMarker->fBox)->SetCard(fMarker->fBet,fMarker->fCash);
				} break;
*/
				default:
					if( fMarker->fCard > 0 )
						IBox(fMarker->fBox)->DelCard();
			}
			delete fMarker;
			fMarker = Next;
		}
		fCardsCount = fMarkerCount;
		fMarkerCount = 0;
		for( unsigned int i = 0; i < PlayerCount(); i++ )
		{
			Player(i)->User()->CashTmpRollback();
			Player(i)->User()->CashResRollback();
		}
	}

	TStep* MarkerCardChange(const int Box, const unsigned int Index) {
		int Card = MarkerCardGen();
		TStep* p = MarkerStep(Box,Card,MARKER_BOX_CARD_MOVE,IBox(Box)->Card(Index));
		p->fBet = Index;
		IBox(Box)->SetCard(Index,Card);
		return p;
	}

	TStep* MarkerCardMove(const int Box1, const int Box2, const unsigned int Cash = 0) {
		IBox(Box1)->SetCard(IBox(Box2)->CardLast());
		IBox(Box2)->DelCard();
		return MarkerStep(Box1,0,MARKER_BOX_CARD_MOVE,Box2);
	}

	TStep* MarkerStep(
			const int Box,
			const unsigned int Card,
			const int Action,
			const TStr& Info = "")
	{
		TStep* p = new TStep();
		if( fMarker == NULL )
			fMarker = p;
		else
			fMarker->Add(p);
		p->fNext = NULL;
		p->fCard = Card;
		p->fBox = Box;
		p->fAction = Action;
		p->fInfo = Info;
		if( Card > 0 )
			IBox(Box)->SetCard(Card);
		return p;
	}

	TStep* MarkerAction(
			const int Box,
			const int Action,
			const TStr& Info = "")
	{
		return MarkerStep(Box,0,Action,Info);
	}

	unsigned int MarkerCardGen() {
		unsigned int Card;
		if( iCards < iCardsCount )
			Card = fCards[iCards++];
		else {
			//
//			for( unsigned int i = 0; i < fCardsCount; i++ )
//				printf("%d ",fCards[i]);
			//
			getRnd(fCards+fCardsCount,1,1,fCardsSize,fCards,fCardsCount);
			Card = fCards[fCardsCount];
//			printf("%d\n",Card);
		}
		fCardsCount++;
		if( fCardsSize > 53 && Card > 52 )
			while( Card > 52 )
				Card -= 52;
		return Card;
	}

	TStep* MarkerCard(const int Box, const int Action, const unsigned int Cash = 0) {
		return MarkerStep(Box,MarkerCardGen(),Action,Cash);
	}

	void         CardsLoad(const char* Name) {
		iCards = 0;
		iCardsCount = 0;
#ifdef TARGET_WIN32
//		iCardsCount = MasLoad(fCards,Name);
#endif
	}
	void         CardsClear(const int f = 0) { fCardsCount = f; }
	unsigned int CardsCount() { return fCardsCount; }

	unsigned int virtual GetTimer() { return 60; }

	void virtual OnGameStartBefore() {}
	void virtual OnGameStart() = NULL;
	void virtual OnGame(const TStr&, const TStr&, IPlayer*) { }
	void virtual OnGameEnd();

	void         GameResult();
	void         GameEnd();

	virtual bool isBoxPlay(const unsigned int BoxID) {
		return Box(BoxID)->Play() && isBoxBet(BoxID);
	}
	virtual bool isBoxPlay(const unsigned int BoxID, IPlayer* Player) {
		return Box(BoxID)->Player() == Player && isBoxPlay(BoxID);
	}

	TStr BoxBet(const unsigned int i, const bool fCash);

	void OnBoxJoin(const unsigned int BoxID, IPlayer* Player);
	void OnBoxLeave(const unsigned int BoxID);

	void OnRequest(TRequest *r, IPlayer* Player);
	bool OnRequestAction(IPlayer* Player, TXMLNode *p);
	bool OnRequestBet(IPlayer* Player, TXMLNode *p);

	bool isSelect(const unsigned int Table) {
		bool f = false;
//		if( Mode() == 0 ) {
			f = IPlayerRoom::isSelect(Table);
			if( f ) {
				f = false;
				for( unsigned int i = 0; i < BoxCount(); i++ )
				if( Box(i)->Player() == NULL ) {
					f = true;
					break;
				}
			}
//		}
		return f;
	}

	void OnPlayerAdd(IPlayer *);
	void OnPlayerDel(IPlayer *);

	unsigned int TableID() { return Table(); }

public:

	TCardsRoom(const unsigned int Count, const unsigned int BoxCount, const unsigned int CardsCount)
	: IPlayerRoom(Count), TBoxTable(BoxCount), fCardsSize(CardsCount), iRollback(0)
	{
		fCards = new unsigned int [CardsCount];
		iCards = iCardsCount = fMarkerCount = 0;
		fMarker = NULL;
		CardsClear();
	}

	virtual ~TCardsRoom() {
		delete [] fCards;
	}

	TStr ModeAsStr()
	{
		if( Mode() == 0 )
			return "bet";
		if( Mode() == 1 )
			return "game";
		return "ok";
	}

	void virtual OnTagPlay(IPlayer *Player) {}
	void virtual OnTagPlayNext(IPlayer *Player) {}

	bool CashTmpAdd(const unsigned int i, const unsigned int Cash) {
		return Box(i)->Player()->User()->CashTmpAdd(GameID(),Table(),Cash,iRollback > 100);
	}
//	int  CashWin(const unsigned int i) { return Box(i)->Player()->User()->CashWinTotal(); }
	void TagInit();
	void TagPlay(unsigned int fBox);
	bool TagPlayNext(const unsigned int BoxCount);
	bool TagPlayNext(IPlayer *Player, const unsigned int BoxCount);
	bool TagGameEnd();

};


#endif

