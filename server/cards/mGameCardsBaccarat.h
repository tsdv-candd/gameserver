#ifndef mGameCardsBaccaratH
#define mGameCardsBaccaratH

#include "mGameCards.h"

#define cBaccaratName "baccarat"

const unsigned int cBaccaratSize = 256;
const unsigned int cBaccaratGame = 40;

class TBaccaratRoom: public TCardsRoom
{
enum EAction { MARKER_SCORE = MARKER_NOT+1 };
enum EBOX { BOX_TIE = 0, BOX_PLAYER = 1, BOX_BANKER = 2 };
private:

	unsigned int BoxScore(const int BoxID = -1);
	TStr         BoxScore(m_ssi::TSSIControl *ssi, const int BoxID = -1);

protected:

	unsigned int GameID() { return cBaccaratGame; }
	TStr Log(IPlayer*);

	void OnGameEnd();    

	void OnCommitCard( TStep* p );

	virtual const char* BoxStr(const unsigned int Index) {
		switch( Index )
		{
			case BOX_TIE: return "tie";
			case BOX_PLAYER: return "player";
			case BOX_BANKER: return "banker";
		}
		return "";
	}

public:

	TBaccaratRoom(const unsigned int Count)
	: TCardsRoom(Count,10,52) {}

	void OnGameStart();

};



class TBaccaratProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TBaccaratRoom(1);
		return new TBaccaratRoom(5);
	}
public:
	TBaccaratProducer(): IPlayerProducer(
		cBaccaratGame,
		cBaccaratName,
		cBaccaratSize)
	{}
};

#endif
