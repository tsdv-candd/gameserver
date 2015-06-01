#ifndef mGameVideopokerJokerpokerH
#define mGameVideopokerJokerpokerH

#include "server/mGame.h"

#define cJokerpokerName "jokerpoker"
#define cJokerpokerSize 128

const unsigned int cGameJokerpoker = 6;
const unsigned int cGameJokerpokerChance = 106;

enum TStatus { tBet, tGame, tChance };

class TJokerpokerSession: public ISession
{
private:
	unsigned int BetTable;

	TStatus      fStatus;
	unsigned int Coin;
	int          fCashBet;
	int          fCashWin;
	unsigned int fCards[5];
	unsigned int fCardsChance[26];
	unsigned int fCardsChanceCount;
	//
	void OnBet(TRequest *r);
	void OnGame(TRequest *r);
	void OnCash(TRequest *r);
	void OnChance(TRequest *r);
protected:
	virtual void OnStart(TUser*,const int i,const bool f);
	virtual void OnLeave(TUser* User);
	void         Run(TRequest *request);
public:
	static unsigned int fWin[11];
	TJokerpokerSession(): fCashWin(0),fCashBet(0),fStatus(tBet) {}
};

class TJokerpokerProducer:public ISessionProducer
{
protected:
	ISession* OnCreateSession() { return new TJokerpokerSession(); }
public:
	TJokerpokerProducer();
};

#endif
