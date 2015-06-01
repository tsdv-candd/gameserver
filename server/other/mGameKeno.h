#ifndef mGameKenoH
#define mGameKenoH

#include "server/mGame.h"

#define cKenoName "keno"

const unsigned int cGameKeno = 4;

class TKeno: public TGameRequest
{
private:
protected:
  void Run(TRequest *request);
public:
  static unsigned int fWin[10][10];
  TKeno();
};

#endif
