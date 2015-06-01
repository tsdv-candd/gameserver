#ifndef mGameCardsPaiGowPokerH
#define mGameCardsPaiGowPokerH

#include "mGameCards.h"

#define cPaiGowPokerName "paigowpoker"

const unsigned int cPaiGowPokerSize = 256;
const unsigned int cPaiGowPokerGame = 33;




class TPaiGowPokerRoom: public TCardsRoom
{
enum EAction { MARKER_CLOSE = MARKER_NOT + 1, MARKER_HOUSE };
private:
	void OnCommitCard( TStep* p );
	unsigned int GameID() { return cPaiGowPokerGame; }
	TStr Log(IPlayer*);
	void OnTagPlay(IPlayer* Player);
	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);
	void OnPlayerAdd(IPlayer *);

	unsigned int _ID1,_ID2;
	unsigned int ID1[3];
	unsigned int ID2[3];
	TStr BoxHouse(const int BoxID);
protected:
public:
	TPaiGowPokerRoom(const unsigned int Count)
	: TCardsRoom(Count,3,53) {}
};



class TPaiGowPokerProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TPaiGowPokerRoom(1);
		return new TPaiGowPokerRoom(5);
	}
public:
	TPaiGowPokerProducer(): IPlayerProducer(
		cPaiGowPokerGame,
		cPaiGowPokerName,
		cPaiGowPokerSize)
	{}
};

#endif
