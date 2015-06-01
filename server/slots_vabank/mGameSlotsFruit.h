#ifndef mGameSlotsFruitH
#define mGameSlotsFruitH

#include "server/mGame.h"

#define cSlots2Name "fruit"
#define cSlots2Size 128

const unsigned int cGameSlots2 = 5;

class TServerSSI;

class TSlots2Session: public ISession
{
private:
	void OnBet(TRequest *r);
protected:
	void Run(TRequest *request);
public:
	struct TWin {
		unsigned int wheel1;
		unsigned int wheel2;
		unsigned int wheel3;
		unsigned int pay;
	};
	static const int    fWheelLine[3][3];
	static TWin         fWheelWin[39];
	static unsigned int fWheelID[3][20];
};

class TSlots2Producer:public ISessionProducer
{
protected:
	ISession* OnCreateSession() { return new TSlots2Session(); }
public:
	TSlots2Producer();
};



#endif
