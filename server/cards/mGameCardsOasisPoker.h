#ifndef mGameCardsOasisPokerH
#define mGameCardsOasisPokerH

#include "mGameCards.h"

#define cOasisPokerName "oasispoker"

const unsigned int cOasisPokerSize = 256;
const unsigned int cOasisPokerGame = 30;

class TOasisPokerRoom: public TCardsRoom
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
	unsigned int GameID() { return cOasisPokerGame; }
	TStr Log(IPlayer*);
	void OnTagPlayNext(IPlayer *Player);
	void OnTagPlay(IPlayer* Player);

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

	void OnBoxBonus(const unsigned int i, const bool f);

protected:

public:

	TOasisPokerRoom(const unsigned int Count)
	: TCardsRoom(Count,5,52) {}

	static void Init();
	static void Free();
};



class TOasisPokerProducer:public IPlayerProducer
{
protected:
	IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
		if( Private )
			return new TOasisPokerRoom(1);
		return new TOasisPokerRoom(5);
	}
public:
	TOasisPokerProducer(): IPlayerProducer(
		cOasisPokerGame,
		cOasisPokerName,
		cOasisPokerSize)
	{
		TOasisPokerRoom::Init();
	}
	~TOasisPokerProducer() {
		TOasisPokerRoom::Free();
	}
};

#endif
