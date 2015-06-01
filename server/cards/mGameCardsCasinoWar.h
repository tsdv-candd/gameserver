#ifndef mGameCardsCasinoWarH
#define mGameCardsCasinoWarH

#include "mGameCards.h"

#define cCasinoWarName "casinowar"

const unsigned int cCasinoWarSize = 256;
const unsigned int cCasinoWarGame = 42;

class TCasinoWarRoom: public TCardsRoom
{
enum EAction { MARKER_PLAY = MARKER_NOT + 1 };
enum EBOX { BOX_BET = 0, BOX_TIE = 1, BOX_WIN = 3 };
protected:

	void OnCommitCard( TStep* p );
	unsigned int GameID() { return cCasinoWarGame; }
	TStr Log(IPlayer*);

	void BoxGame();

	void OnTagPlay(IPlayer* Player);

	virtual const char* BoxStr(const unsigned int Index) {
		switch( Index )
		{
			case BOX_TIE: return "tie";
			case BOX_BET: return "bet";
		}
		return "";
	}

	void OnPlayerAdd(IPlayer *);

public:

	TCasinoWarRoom(const unsigned int Count)
	: TCardsRoom(Count,5,52) {}

	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);

};



class TCasinoWarProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TCasinoWarRoom(1);
		return new TCasinoWarRoom(5);
	}
public:
	TCasinoWarProducer(): IPlayerProducer(
		cCasinoWarGame,
		cCasinoWarName,
		cCasinoWarSize)
  {}
};

#endif
