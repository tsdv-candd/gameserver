#ifndef mGameVideopokerTenorbetterH
#define mGameVideopokerTenorbetterH

#include "server/mGame.h"

#define cVideopokerName "videopoker"
#define cVideopokerSize 128

const unsigned int cGameVideopoker = 3;
const unsigned int cGameVideopokerChance = 103;

enum TStatus { tBet, tGame, tChance };

class TVideopokerSession: public ISession
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
	static unsigned int fWin[9];
	TVideopokerSession(): fCashWin(0),fCashBet(0),fStatus(tBet) {}
};

class TVideopokerProducer:public ISessionProducer
{
protected:
	ISession* OnCreateSession() { return new TVideopokerSession(); }
public:
	TVideopokerProducer();
};

#endif
