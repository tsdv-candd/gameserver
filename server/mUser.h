#ifndef mUserH
#define mUserH

#include "utils/m_object.h"
#include "utils/m_utils.h"
#include "ssi/mf_ssi.h"
#include "common/mPay.h"
#include "mCash.h"
#include "m_string.h"


enum TUserStatus {
	tGIFT,
	tFUN,
	tREAL
};


enum EUserCash {
	cashUNDEFINED,
	cashFUN,
	cashREAL,
	cashBONUS,
	cashFREE,
	cashFREEBETS
};


class TCasinoControl;

class TSession;

const unsigned int cJackpotVideopoker = 1;
const unsigned int cJackpotSlots = 2;
const unsigned int cJackpotSlots2 = 3;

class TUser: public m_object::TObject/*, public TLock*/
{
friend class TServer;
friend class TSession;
public:
	enum EJackpot { fJackpotSize = 100, fGameSize = 200 };
private:
	struct TJackpot {
		struct TGame {
			unsigned int ID;
			double       Bet;
		};
		unsigned int ID;
		double       Cash;
		TStr Info();
		void Info(const TStr& );
		TGame        Game[fGameSize];
		unsigned int GameCount;
	};
	static TJackpot     fJackpot[fJackpotSize];
	static unsigned int fJackpotCount;
	static void Load(m_classes::TMySqlConnect* db);
	static void Save(m_classes::TMySqlConnect* db);
	static void JackpotBet(const unsigned int GameID, const double& Cash);
	static int  JackpotGet(const unsigned int ID);

	static TCasinoControl *p_CasinoControl;

	static unsigned int GuestID;
	//
	bool fClose;

	unsigned int fID;
	unsigned int fLogID;

	TStr         fLogin;
	TStr         fPassword;
	TStr         fNick;
	TStr         fEMail;

	unsigned int fCashFun;
	unsigned int fCashReal;
	double       fCashRealBonus;
	double       fCashFree;
	double       fCashFreeBets;
	double       fCashBonus;

	TUserStatus  fStatus;
	TStr         fMessage;

	TSession     *fSession;

	TStr         fSex;
	TStr         fBonus;
	TStr         fPhone;
	TStr         fIcon;
	TStr         fDate;
	TStr         fGame;
	unsigned int fCountryID;
	unsigned int fLangID;

	int          fCashBetID;
	int          fCashWinID;
	int          fCashTmpID;

	TTable*      fCashBet;
	TTable*      fCashRes;
	unsigned int fCashBetCommit;
	unsigned int fCashWinCommit;

	unsigned int  fCashRealMin;
	unsigned int  fCashRealMinDn;
	unsigned int  fCashRealMax;
	unsigned int  fCashRealMaxUp;

	void OnStatusEnter();
	void OnStatusLeave();

	bool CashRealMin(TTable* Table, const bool f);
	bool CashRealMax(TTable* Table, const bool f);
public:

	void GameClear();

	int  Cash(const EUserCash UserCash) {
		switch( UserCash )
		{
			case cashFUN: return fCashFun;
			case cashREAL: return fCashReal;
			case cashBONUS: return (int)fCashBonus;
			case cashFREE: return (int)fCashFree;
			case cashFREEBETS: return (int)fCashFreeBets;
		}
		return 0;
	}
	bool CashUpdate(const EUserCash UserCash, const int Amount) {
		if( (Cash(UserCash)+Amount) < 0 )
			return false;
		switch( UserCash )
		{
			case cashFUN:      fCashFun += Amount; break;
			case cashREAL:     fCashReal += Amount; break;
			case cashBONUS:    fCashBonus += Amount; break;
			case cashFREE:     fCashFree += Amount; break;
			case cashFREEBETS: fCashFreeBets += Amount; break;
		}
		return true;
	} 

	void     OnCashAdd();

	unsigned int CashAdd(
			const int Cash,
			const TUserStatus);
	unsigned int CashAdd(
			const EPaySystem SystemID,
			const int Cash,
			const int CashBonus);
	void CashError(
		const EPaySystem SystemID,
		const int Cash);

	TUser()
	: fSession(NULL), fStatus(tGIFT), fID(0), fNick("Guest_____"),

		fCashRealMin(0), fCashRealMinDn(0),
		fCashRealMax(0), fCashRealMaxUp(0),

		fLogID(0),

		fCashFun(0),
		fCashReal(0), fCashRealBonus(0), fCashBonus(0), fCashFree(0), fCashFreeBets(0),

		fClose(false),

		fCashBet(NULL), // Накопительная ставка
		fCashRes(NULL),  // Накопительный выигрыш
		fCashBetCommit(0),  // Всего ставок сделано
		fCashWinCommit(0),  // Всего выйграл
		fCashBetID(-100), // КОД ставки
		fCashWinID(-100), // Транзакция выигрыша
		fCashTmpID(-100)  // Транзакция времеенная
	{
//		TLockSection _l(&GuestLock);
		fNick = TStr("Guest ")+GuestID++;
	}
	virtual ~TUser();

	TStr         Message() { TStr s(fMessage); fMessage.Clear(); return s; };

	unsigned int ID() { return fID; }

	TStr         Nick() { return fNick; }
	TStr         EMail() { return fEMail; }
	TStr         Icon() { return fIcon; }
	TUserStatus  Status() { return fStatus; }

	unsigned int Mode() {
		if( fStatus == tFUN )
			return 2;
		return 1;
	}

	void         ModeToREAL();

	void         ToSSI(m_ssi::TSSIControl *ssi);

	void         Save(); 
	void         Close();
	void         CloseBlock();
	void         Auth(TSession* Session);
	void         Auth(m_classes::TMySqlQuery&, TSession* Session);
	TStr         Edit(const bool,const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, const TStr&, TStr, const TStr&);

	TStr         ToXML();

	unsigned int Cash(TUserStatus Status);
	unsigned int Cash();
	unsigned int CashBonus() { return (unsigned int)fCashBonus; }

//	unsigned int CashJackpot(const unsigned int GameID);

	unsigned int CashBetCommit() { return fCashBetCommit; }
	unsigned int CashBetTotal() {
		if( fCashBet == NULL )
			return 0;
		return (unsigned int)(fCashBet->Cash());
	}
	TTable* CashBet() { return fCashBet; }
	bool CashBetAdd(
			const unsigned int TableID,
			const int Cash) { TTable T(TableID,Cash); return CashBetAdd(&T); }
	bool CashBetAdd(TTable* Table);
	void CashBetRollback();
	bool CashBetCommit(
			const unsigned int GameID,
			const unsigned int TableID,
			const int Cash) { TTable T(TableID,Cash); return CashBetCommit(GameID,&T); }
	bool CashBetCommit(
			const unsigned int GameID,
			TTable* Table = NULL);

	unsigned int CashWinTotal() { return p_CasinoControl->TRCash(this->fCashWinID); }
	TTable* CashWin() { return p_CasinoControl->TRTable(this->fCashWinID); }
	bool CashWinUpdate(
			const unsigned int GameID,
			const unsigned int TableID,
			const unsigned int Cash,
			const bool f = false) { TTable T(TableID,Cash); return CashWinUpdate(GameID,&T,f); }
	bool CashWinUpdate(
			const unsigned int GameID,
			TTable* Table,
			const bool f = false);
	bool CashWinAdd(
			const unsigned int GameID,
			const unsigned int TableID,
			const int Cash,
			const bool f = false) { TTable T(TableID,Cash); return CashWinAdd(GameID,&T,f); }
	bool CashWinAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f = false);
	void CashWinCommit(const TStr& Log);
	void CashWinRollback();

	bool CashResAdd(
			const unsigned int GameID,
			const unsigned int TableID,
			const int Cash,
			const bool f) { TTable T(TableID,Cash); return CashResAdd(GameID,&T,f); }
	bool CashResAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f);
	void CashResCommit(const unsigned int GameID);
	void CashResRollback();

	bool CashTmpAdd(
			const unsigned int GameID,
			const unsigned int TableID,
			const int Cash,
			const bool f)  { TTable T(TableID,Cash); return CashTmpAdd(GameID,&T,f); }
	bool CashTmpAdd(
			const unsigned int GameID,
			TTable* Table,
			const bool f);
	void CashTmpCommit();
	void CashTmpRollback();

	void CashCommit(
			const unsigned int GameID,
			const TStr& Log);
	void CashCommit();
	void CashRollback();

	int  Cash(const unsigned int GameID, const unsigned TableID) {
		return (int)p_CasinoControl->Cash(Mode(),GameID,TCasinoControl::GAME,TableID);
	}
/*
	void CashJackpotCommit(
			const TStr& Log,
			unsigned int GameID,
			const int Jackpot);
	void JackpotUpdate(const unsigned int GameID, const int Cash) {
		if( fStatus == tREAL )
			p_CasinoControl->UpdateCash(1,GameID,TCasinoControl::JACKPOT,Cash);
	}
	int Jackpot(const unsigned int GameID) {
//		return (int)p_CasinoControl->Cash(1,GameID,TCasinoControl::JACKPOT);
		return JackpotGet(GameID);
	}
*/
	int JackpotID(const unsigned int ID) {
		return JackpotGet(ID);
	}
};

void UserCashLog(
	const bool         StatusReal,
	const EPaySystem   SystemID,
	const unsigned int UserID,
	const unsigned int BalanceFree,
	const int          Cash,
	const unsigned int Balance,
	const unsigned int OrderID);

#endif
