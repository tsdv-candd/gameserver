#ifndef mGameRouletteH
#define mGameRouletteH

#include "server/mGame.h"

#define cRouletteName "roulette"

const unsigned int cRouletteGame = 1;
const unsigned int cRouletteSize = 256;

const unsigned int cHistory = 10;
const unsigned int sCount = 158;

class TBet {
private:
  IPlayer *fPlayer;
  char    fCode[4];
  int     fCash;
public:
  int     fWin;

  TBet(IPlayer *Player,const TStr& Code,const unsigned int Cash)
  :fPlayer(Player), fCash(Cash) {
    strncpy(fCode,Code.c_str(),3);
    fCode[3] = '\0';
  }
  IPlayer* Player() { return fPlayer; }
  char*    Code() { return fCode; }
  int      Cash() { return fCash; }
  void     CashAdd(int Cash) { fCash += Cash; if( fCash < 0 ) fCash = 0; }
};

class TRouletteRoom: public IPlayerRoom
{
private:

  static char*          fBetCode[sCount];
  static unsigned int   fBetCashWin[sCount][37];
  static char           fBetCashCode[8][3];
  static int            fBetCashMin[8][3];
  static int            fBetCashMax[8][3];

  bool BetLimitCheck(char Code, int Cash);
  void BetLimitCheck();
  
protected:

  m_object::TVector<TBet> fBetList;
  unsigned int            fHistory[cHistory];

  int FindBetIndex(const TStr& Code, IPlayer *);
  
  void OnPlayerAdd(IPlayer *);
  void OnPlayerDel(IPlayer *);

public:
  TRouletteRoom(const unsigned int Count);

  TStr         ModeAsStr();  
  void         OnRequest(TRequest *r, IPlayer* Player);
  void         OnGame();
  void         OnBet(TRequest *r, IPlayer* Player);
  
  static void  Init();
};

class TRouletteProducer:public IPlayerProducer
{
protected:
  IPlayerRoom* OnCreateRoom(const unsigned int,const bool Private) {
    if( Private )
      return new TRouletteRoom(1);
    return new TRouletteRoom(5);
  }
public:
  TRouletteProducer(): IPlayerProducer(
    cRouletteGame,
    cRouletteName,
    cRouletteSize)
  {
    TRouletteRoom::Init();
  }
};

#endif
