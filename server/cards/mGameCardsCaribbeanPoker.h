#ifndef mGameCardsCaribbeanPokerH
#define mGameCardsCaribbeanPokerH

#include "mGameCards.h"

#define cCaribbeanPokerName "caribbeanpoker"

const unsigned int cCaribbeanPokerSize = 256;
const unsigned int cCaribbeanPokerGame = 31;

class TCaribbeanPokerRoom: public TCardsRoom
{
enum EAction {
	MARKER_CLOSE = MARKER_NOT + 1,
	MARKER_COMBINATION,
	MARKER_WINGAME,
	MARKER_WINBONUS };
enum EBOX {
	BOX_ANTE = 0,
	BOX_BONUS = 1,
	BOX_BET = 2 };
private:

	static TPokerCombination *fGame;
	static TPokerCombination *fBonus;

	void OnCommitCard( TStep* p );
	unsigned int GameID() { return cCaribbeanPokerGame; }
	TStr Log(IPlayer*);
	void OnTagPlayNext(IPlayer *Player);
	void OnTagPlay(IPlayer* Player);
	void OnBoxBonus(const unsigned int i, const bool f); 

	void OnGameStart();
	void OnGame(const TStr&, const TStr&, IPlayer*);
	void OnPlayerAdd(IPlayer *);

	virtual const char* BoxStr(const unsigned int Index) {
		switch( Index )
		{
			case BOX_ANTE: return "ante";
			case BOX_BONUS: return "bonus";
			case BOX_BET: return "raise";
		}
		return "";
	}

public:

	TCaribbeanPokerRoom(const unsigned int Count)
	: TCardsRoom(Count,5,52) {}

	static void Init();
	static void Free();
};



class TCaribbeanPokerProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TCaribbeanPokerRoom(1);
		return new TCaribbeanPokerRoom(5);
	}
public:
	TCaribbeanPokerProducer(): IPlayerProducer(
		cCaribbeanPokerGame,
		cCaribbeanPokerName,
		cCaribbeanPokerSize)
	{
		TCaribbeanPokerRoom::Init();
	}
	~TCaribbeanPokerProducer() {
		TCaribbeanPokerRoom::Free();
	}
};

#endif
