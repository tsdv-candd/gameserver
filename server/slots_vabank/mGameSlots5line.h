#ifndef mGameSlots5lineH
#define mGameSlots5lineH

#include "server/mGame.h"

#define cSlotsName "slots"
#define cSlotsSize 128

const unsigned int cGameSlots = 2;
const unsigned int cGameSlotsChance = 102;

enum TStatus { tBet, tGame };

class TServerSSI;

class TSlotsSession: public ISession
{
private:
	TStatus      fStatus;
	unsigned int fCashWin;
	unsigned int fChance;
	void OnBet(TRequest *r);
	void OnGame(TRequest *r);
protected:
	virtual void OnStart(TUser* User, const int i,const bool f);
	virtual void OnLeave(TUser*);
	void         Run(TRequest *request);
public:
	struct TWin {
		unsigned int wheel1;
		unsigned int wheel2;
		unsigned int wheel3;
		unsigned int pay;
	};
	static const int    fWheelLine[5][3];
	static TWin         fWheelWin[36];
	static unsigned int fWheelID[3][20];

	TSlotsSession();
};

class TSlotsProducer:public ISessionProducer
{
protected:
	ISession* OnCreateSession() { return new TSlotsSession(); }
public:
	TSlotsProducer();
};



#endif
