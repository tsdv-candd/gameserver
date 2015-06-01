#ifndef mGameCardsBlackjackH
#define mGameCardsBlackjackH

#include "mGameCards.h"
#include "utils/m_utils.h"

#define cBlackjackName "blackjack"

const unsigned int cBlackjackSize = 256;
const unsigned int cBlackjackGame = 20;

class TBlackjackRoom: public TCardsRoom
{

enum EAction {
	MARKER_BLACKJACK = MARKER_NOT+1,
	MARKER_BUST,
	MARKER_SCORE,
	MARKER_DOUBLE,
	MARKER_HIT,
	MARKET_INSURANCE_WIN,
	MARKET_INSURANCE_LOSE
};

enum EBox { BOX_INSURANCE = 1, BOX_DOUBLE = 3 };

private:

	static int Mas[10];

	bool         fTrash;
	int          fBox;

protected:

	TStr Log(IPlayer*);

	virtual bool isNewsGameButton() {
		if( !fTrash )
			return true;
		return false;
	}

	virtual bool isBoxBet() { return false; }

	bool BoxWin(const unsigned int BoxID, const bool f);

	void BoxPlay( bool code = true );
	void BoxButton();

	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);
	void OnGameEnd();

	void OnCommitCard(TStep* p);

	bool isInsurance(const unsigned int i) {
		return
			Box(i)->CardsCount() == 2 &&
			CardID(IBox(-1)->CardFirst()) >= 10;
	}
	bool isSplit(const unsigned int i) {
		return
			i < 5 &&
			Box(i)->CardsCount() == 2 &&
			CardPrice(Box(i)->Card(0)) == CardPrice(Box(i)->Card(1));
	}
	bool isBlackjack(const int i) {
		if( i < 0 )
			return
				IBox(i)->Score() == 21 &&
				IBox(i)->CardsCount() == 2;
		return
			i < 5 &&
			Box(i)->Score() == 21 &&
			Box(i)->CardsCount() == 2 &&
			Box(i+5)->Player() == NULL;
	}

	void OnPlayerAdd(IPlayer *);

	unsigned int virtual GetTimer() { return 60; }

public:
	TBlackjackRoom(const unsigned int Count)
	: TCardsRoom(Count,5,52*6), fTrash(true) {}

	unsigned int GameID() { return cBlackjackGame; }
};



class TBlackjackProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TBlackjackRoom(1);
		return new TBlackjackRoom(5);
	}
public:
	TBlackjackProducer(): IPlayerProducer(
		cBlackjackGame,
		cBlackjackName,
		cBlackjackSize)
	{}
};

#endif
