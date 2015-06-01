#ifndef mCashH
#define mCashH

#include "sql/mMySQL.h"
#include "thread/m_thread.h"
#include "common/mCashUtils.h"

const unsigned int cTRCount = 1000;

template <class Type>
class TCashList
{
private:
		Type* fNext;
public:
		Type* Add(Type* p) {
			if( fNext == NULL )
				return fNext = p;
			return fNext->Add(p);
		}
public:
		TCashList(): fNext(NULL) {}
		virtual ~TCashList() { if( fNext != NULL ) delete fNext; }

		Type* Next() { return fNext; }
};

template <class Type>
class TRootList: public TCashList<Type>
{
private:
		static Type* fRoot;
protected:
		static Type* AddRoot(Type* p) {
			if( fRoot == NULL )
				return fRoot = p;
			return fRoot->TCashList<Type>::Add(p);
		}
public:
		static Type* Root() { return fRoot; }
		static void RootFree() {
			if( fRoot != NULL )
				delete fRoot;
		}
};

template <class Type> Type* TRootList<Type>::fRoot = NULL;


/*
class TCashList
{
private:
	struct TCash {
		unsigned int fID;
		unsigned int fCash;
	};
	TList<TSub>  fList;
	unsigned int fCash;
public:

};
*/

class TTable: public TCashList<TTable>
{
private:
		unsigned int fTable;
		double       fCash;

		TTable(const TTable&) {}
public:
		unsigned int fBet;
		TTable(
				const unsigned int* Table,
				const double*       Cash,
				const int           Size) :
			fBet(1),
			fCash(0),
			fTable(0)
		{
			if( Size > 0 ) {
				fTable = Table[0];
				fCash = Cash[0];
				for( int i = 1; i < Size; i++ )
					CashAdd(Table[i],Cash[i]);
			}
		}
		TTable(
				const unsigned int Table,
				const double& Cash = 0) :
			fBet(1),
			fCash(Cash),
			fTable(Table)
		{}
		TTable(TTable* Table) :
			fCash(Table->fCash),
			fTable(Table->fTable)
		{
			if( Table->Next() != NULL )
				AddTable(Table->Next(),true);
		}

		void AddTable(TTable* Table, const bool fAdd) {
			for( TTable* pTable = Table; pTable != NULL; pTable = pTable->Next() )
				if( fAdd )
					CashAdd(pTable->ID(),pTable->IDCash());
				else
					CashAdd(pTable->ID(),-pTable->IDCash());
		}

		unsigned int ID() { return fTable; }
		double       IDCash() { return fCash; }

		TStr XML() {
			TStr s;
			s = "<table id=\""+TStr(fTable)+"\" cash=\""+CashToStr(fCash)+"\" />\n";
			if( Next() != NULL )
				s += Next()->XML();
			return s;
		}

		bool CheckMax(TTable* Table) {
			for( TTable* p = Table; p != NULL; p = p->Next() )
				if( Cash(p->ID()) >= p->IDCash() )
					return false;
			return true;
		}

		bool CheckMin(TTable* Table) {
			for( TTable* p = Table; p != NULL; p = p->Next() )
				if( Cash(p->ID()) < p->IDCash() )
					return false;
			return true;
		}

		void Update(const unsigned int Table, const double& Cash) {
			fTable = Table;
			fCash = Cash;
		}

		double CashAdd(const unsigned int Table, const double& Cash) {
			if( fTable == Table )
				return fCash += Cash;
			if( Next() != NULL )
				return Next()->CashAdd(Table,Cash);
			Add(new TTable(Table,Cash));
			return Cash;
		}

		void BetUpdate(const unsigned int Table, const unsigned int Bet) {
			if( fTable == Table ) {
				if( Bet == 0 )
					fBet = 1;
				else
					fBet = Bet;
			} else
			if( Next() != NULL )
				Next()->BetUpdate(Table,Bet);
		}

		float Cash() {
			double r = 0;
			if( fTable > 0 )
				r += fCash;
			if( Next() != NULL )
				r += Next()->Cash();
			return (float)r;
		}

		float Cash(const unsigned int Table) {
			if( fTable == Table )
				return (float)fCash;
			if( Next() != NULL )
				return (float)Next()->Cash(Table);
			return 0;
		}
};


class TCasinoControl
{
private:

		class TGame;
		
		class TMoney: public TCashList<TMoney>
		{
		private:
				unsigned int  fID;
				unsigned int  fBet;
				TTable        fTable;
				TGame*        fGame;
		public:
				TMoney(
						TGame* Game,
						const unsigned int MoneyID,
						const unsigned int Bet,
						const unsigned int Table,
						const double& Cash)
				:fTable(Table,Cash), fGame(Game), fID(MoneyID), fBet(Bet) {}

				TMoney* AddMoney(TMoney* p) { return TCashList<TMoney>::Add(p); }

				unsigned int Bet() { return fBet; }
				void BetUpdate(const unsigned int Bet) { fBet = Bet; }
				unsigned int ID() { return fID; }
				TGame* Game() { return fGame; }
				TTable* Table() { return &fTable; }
				TStr XML() {
						TStr s;
						s = TStr("<money ") +
							" id=\""  + fID + "\""
							" bet=\"" + fBet + "\""
							">\n" + fTable.XML() + "</money>\n";
						if( Next() != NULL )
							s += Next()->XML();
						return s;
				}

				void CashUpdate(const double& Cash) {
					double TotalBet = 0;
					TTable* p = Table();
					while( p != NULL )
					{
						if( p->ID() > 0 )
							TotalBet += p->fBet;
						p = p->Next();
					}
					p = Table();
					double DCash = Cash;
					while( p != NULL )
					{
						if( p->ID() > 0 ) {
							double f;
							if( p->Next() == NULL )
								f = DCash;
							else {
								f = Cash*p->fBet/TotalBet;
								DCash -= f;
							}
							p->CashAdd(p->ID(),f);
						}
						p = p->Next();
					}
				}
		}; // TMoney

		class TGame: public TRootList<TGame>
		{
		friend class TRootList<TGame>;
		private:
				unsigned int  fModeID;
				unsigned int  fGameID;
				unsigned int  fBet;
				TMoney        *fMoney;
				TGame(
						const unsigned int ModeID,
						const unsigned int GameID,
						const unsigned int MoneyID,
						const unsigned int Bet,
						const unsigned int Table,
						const double& Cash) :
					fBet(0),
					fModeID(ModeID),
					fGameID(GameID)
				{
					fMoney = new TMoney(this,MoneyID,Bet,Table,Cash);
				}
		public:
				virtual ~TGame() { delete fMoney; }

				unsigned int ModeID() { return fModeID; }
				unsigned int ID() { return fGameID; }
				unsigned int Bet() { return fBet; }
				void BetUpdate(const unsigned int MoneyID, const unsigned int Bet) {
					fBet += Bet - Money(MoneyID)->Bet();
					Money(MoneyID)->BetUpdate(Bet);
				}
				TMoney* Money() { return fMoney; }
				TMoney* Money(const unsigned int MoneyID) {
					TMoney* p = fMoney;
					while( p != NULL )
					{
						if( p->ID() == MoneyID )
							break;
						p = p->Next();
					}
					return p;
				}
				TStr XML() {
					return TStr("<game")+
						" id=\""+fGameID+"\""
						" mode=\""+fModeID+"\""
						" bet=\""+fBet+"\">\n"+
						fMoney->XML()+
						"</game>\n";
				}
				static TGame* Find(const unsigned int ModeID, const unsigned int GameID) {
					TGame* p = Root();
					while( p != NULL ) {
						if( p->fGameID == GameID && p->fModeID == ModeID )
							break;
						p = p->Next();
					}
					return p;
				}
				static TGame* Load(
						const unsigned int ModeID,
						const unsigned int GameID,
						const unsigned int MoneyID,
						const unsigned int Bet,
						const unsigned int Table,
						const double& Cash)
				{
					TGame* p = Find(ModeID,GameID);
					if( p == NULL ) {
						p = TRootList<TGame>::AddRoot(new TGame(
								ModeID,GameID,MoneyID,Bet,Table,Cash));
					} else
						p->fMoney->AddMoney(new TMoney(p,MoneyID,Bet,Table,Cash));
					p->fBet += Bet;
					return p;
				}
		}; // TGame

	class TR
	{
	private:
			TMoney*      fMoney;
			unsigned int fUserID;
			TTable*      fTable;
	public:
			TR() :
					fMoney(NULL),
					fUserID(0),
					fTable(NULL) {}
			virtual ~TR() {
				Close();
			}
			void Open(
					TMoney* Money,
					const unsigned int UserID,
					TTable* Table)
			{
				if( fMoney != NULL )
					throw TExcept("TR::Open");
				fUserID = UserID;
				fMoney = Money;
				if( Table != NULL )
					fTable = new TTable(Table);
				else
					fTable = new TTable(0,0);
			}
			void Close() {
				fUserID = 0;
				fMoney = NULL;
				if( fTable != NULL )
					delete fTable;
				fTable = NULL;
			}
			bool         isClose() { return fMoney == NULL; }
			int Cash() { 
					if( fTable == NULL ) 
						return 0; 
					return (int)fTable->Cash(); 
			}
			TMoney*      Money() { return fMoney; }
			unsigned int UserID() { return fUserID; }
			TTable*      Table() { return fTable; }
	};

//	TLock_fLock;
	TR    fTR[cTRCount];

	int FindTR() {
		for( unsigned int i = 0; i < cTRCount; i++ )
			if( fTR[i].isClose() )
				return i;
		return -1;
	}

	TMoney* Find(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID)
	{
		TGame* Game = TGame::Find(ModeID,GameID);
		if( Game == NULL )
			throw TExcept("TCasinoControl::Find");
		TMoney* Money = Game->Money(MoneyID);
		if( Money == NULL )
			throw TExcept("TCasinoControl::Find");
		return Money;
	}

	int Log(
			m_classes::TMySqlConnect* db,
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const double& Cash,
			const TStr& Info,
			const int LogID);
public:
	enum EMoney {
		CASINO = 1,
		GAME = 2,
		JACKPOT = 3
	};

	TCasinoControl() {
		for( unsigned int i = 0; i < cTRCount; i++ )
			fTR[i].Close();
	}

	virtual ~TCasinoControl() {
		TGame::RootFree();
	}

	TStr XML();
	TStr XMLInfo();

	void Load(m_classes::TMySqlConnect* db);
	void Save(m_classes::TMySqlConnect* db);

	int TRBet(
			m_classes::TMySqlConnect* db,
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int TableID,
			const unsigned int Cash,
			const int LogID,
			const double& Bonus) { TTable T(TableID,Cash); return TRBet(db,UserID,ModeID,GameID,&T,LogID,Bonus); }
	int TRBet(
			m_classes::TMySqlConnect* db,
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			TTable* Table,
			const int LogID,
			const double& Bonus);
	int TRCreate(
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const unsigned int TableID,
			const unsigned int Cash,
			const bool f = false) { TTable T(TableID,Cash); return TRCreate(UserID,ModeID,GameID,MoneyID,&T,f); }
	int TRCreate(
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			TTable* Table,
			const bool f = false);
	bool TRAdd(
			const int TRID,
			TTable* Table,
			const bool f = false);
	bool TRAdd(
			const int TRID,
			const unsigned int TableID,
			const unsigned int Cash,
			const bool f = false) { TTable T(TableID,Cash); return TRAdd(TRID,&T,f); }
	bool TRUpdate(
			const int TRID,
			TTable* Table,
			const bool f = false);
	bool TRUpdate(
			const int TRID,
			const unsigned int TableID,
			const unsigned int Cash,
			const bool f = false) { TTable T(TableID,Cash); return TRUpdate(TRID,&T,f); }
	bool TRJoin(
			const int TRID1,
			const int TRID2);
	unsigned int TRCash(
			const int TRID);
	TGame* TRGame(
			const int TRID);
	TTable* TRTable(
			const int TRID);
	int TRCommit(
			m_classes::TMySqlConnect* db,
			const int TRID);
	void TRRollback(
			const int TRID);

	double Cash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID)
	{
		TGame* Game = TGame::Find(ModeID,GameID);
		if( Game == NULL )
			return 0;
		TMoney* Money = Game->Money(MoneyID);
		if( Money != NULL )
			return Money->Table()->Cash();
		return 0;
	}

	bool UpdateBet(
				const unsigned int ModeID,
				const unsigned int GameID,
				const unsigned int MoneyID,
				const unsigned int TableID,
				const int Bet);

	bool UpdateBet(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const int Bet);

	bool UpdateCash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const unsigned int TableID,
			const double& Cash);

	bool UpdateCash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const double& Cash);

	float Cash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const unsigned int TableID);

	unsigned int Bet(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID);
};


#endif
