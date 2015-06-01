#ifndef mGameCardsLetItRideH
#define mGameCardsLetItRideH

#include "mGameCards.h"

#define cLetItRideName "letitride"

const unsigned int cLetItRideSize = 128;
const unsigned int cLetItRideGame = 32;

class TLetItRideRoom: public TCardsRoom
{
private:

	static TPokerCombination *fGame;
	unsigned int fCardsDealerCount;

protected:

	void OnGameEnd()
	{
		for( unsigned int i = 0; i < 3; i++ )
			if( isBoxBet(i) )
				Box(i)->BetAdd(BOX_WIN,BoxCashWin(i));
		TCardsRoom::OnGameEnd();
	}

	void OnCommitCard( TStep* p );
	unsigned int GameID() { return cLetItRideGame; }
	TStr Log(IPlayer*);
	void OnTagPlay(IPlayer* Player);

	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);
	void OnPlayerAdd(IPlayer *);

	unsigned int BoxCashWin(const unsigned int BoxID);

public:

	TLetItRideRoom(const unsigned int Count)
	: TCardsRoom(Count,3,52) {}

	static void Init();
	static void Free();
};



class TLetItRideProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TLetItRideRoom(1);
		return new TLetItRideRoom(3);
	}
public:
	TLetItRideProducer(): IPlayerProducer(
		cLetItRideGame,
		cLetItRideName,
		cLetItRideSize)
	{
		TLetItRideRoom::Init();
	}
	~TLetItRideProducer() {
		TLetItRideRoom::Free();
	}

};

#endif
