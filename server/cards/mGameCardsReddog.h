#ifndef mGameCardsReddogH
#define mGameCardsReddogH

#include "mGameCards.h"

#define cRedDogName "reddog"

const unsigned int cRedDogSize = 256;
const unsigned int cRedDogGame = 41;

class TRedDogRoom: public TCardsRoom
{
enum EAction {
	MARKER_SPREAD = MARKER_NOT + 1,
	MARKER_PLAY
};
enum EBOX {
	BOX_BET = 0,
	BOX_RAISE = 1
};
protected:

	unsigned int fSpread;

	virtual const char* BoxStr(const unsigned int Index) {
		switch( Index )
		{
			case BOX_RAISE: return "raise";
			case BOX_BET: return "bet";
		}
		return "";
	}
	void OnCommitCard( TStep* p );
	TStr Log(IPlayer*);
	unsigned int GameID() { return cRedDogGame; }

	void OnPlayerAdd(IPlayer *);

	void GameWin(const bool fPush);
	void OnTagPlay(IPlayer *Player);

public:

	TRedDogRoom(const unsigned int Count)
	: TCardsRoom(Count,5,52) {}

	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);

};



class TRedDogProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TRedDogRoom(1);
		return new TRedDogRoom(5);
	}
public:
	TRedDogProducer(): IPlayerProducer(
		cRedDogGame,
		cRedDogName,
		cRedDogSize)
	{}
};

#endif
