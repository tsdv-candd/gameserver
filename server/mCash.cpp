#include "thread/m_thread.h"
#include "xml/m_xmlparser.h"

#include "mCash.h"

#include <stdio.h>


/*
class TSub
{
private:
	unsigned int* fID;
	unsigned int  fCount;
public:
	TSub(const TStr& src) {
		TStr s(src);
		s = s.Trim();
		for( int i = 0; i < s.Length(); i++ )
			if( s[i] == ',' )
				fCount++;
		if( s.Last() != ',' )
			fCount++;
		fID = new unsigned int [fCount];
	}
	~TSub() {
		delete[] fID;
	}

	unsigned int Count() { return fCount; }
	unsigned int ID(const unsigned int i) { return fID[i]; }
};
*/


void TCasinoControl::Load(m_classes::TMySqlConnect* db)
{
//	TLockSection _l(&fLock);
	m_classes::TMySqlQuery query(db);
	m_classes::TMySqlQuery q(db);
	try {
		query.execSql("DELETE FROM money_gamestable WHERE fcash <= 0");
	} catch (...) {
	}
	query.openSql(
		"SELECT FID_Money,FID_Mode,FID_Game,fBet "
		"FROM money_games "
		"ORDER BY FID_Game,FID_Mode,FID_Money");
	while( !query.eof() )
	{
		unsigned int ModeID = query.fieldByNameAsInt("FID_Mode");
		unsigned int GameID = query.fieldByNameAsInt("FID_Game");
		unsigned int MoneyID = query.fieldByNameAsInt("FID_Money");
		unsigned int TableID = 0;
		double       TableCash = 0;
		q.openSql(
			"SELECT FID_Table,fCash,fBet "
			"FROM money_gamestable "
			"WHERE "
			"  FID_Game="+TStr(GameID)+" and "
			"  FID_Mode="+ModeID+" and "
			"  FID_Money="+MoneyID);
		if( !q.eof() ) {
			TableID = q.fieldByNameAsInt("FID_Table");
			TableCash = TStr(q.fieldByName("fCash")).ToDouble();
		}
		TGame* Game = TGame::Load(
				ModeID,
				GameID,
				MoneyID,
				query.fieldByNameAsInt("fBet"),
				TableID,
				TableCash);
		if( !q.eof() ) {
			Game->Money(MoneyID)->Table()->BetUpdate(
					q.fieldByNameAsInt("FID_Table"),
					q.fieldByNameAsInt("fBet"));
			q.next();
			while( !q.eof() )
			{
				Game->Money(MoneyID)->Table()->CashAdd(
						q.fieldByNameAsInt("FID_Table"),
						TStr(q.fieldByName("fCash")).ToDouble());
				Game->Money(MoneyID)->Table()->BetUpdate(
						q.fieldByNameAsInt("FID_Table"),
						q.fieldByNameAsInt("fBet"));
				q.next();
			}
		}
		if( Game->ID() == 1 )
			Game->Money(1)->Table()->Cash();
		query.next();
	}
}


void TCasinoControl::Save(m_classes::TMySqlConnect* db)
{
//	TLockSection _l(&fLock);
	m_classes::TMySqlQuery query(db);
	for( TGame* Game = TGame::Root(); Game != NULL; Game = Game->Next() )
	{
		for( TMoney* Money = Game->Money(); Money != NULL; Money = Money->Next() )
		{
			double Cash = 0;
			TStr Info;
			for( TTable* Table = Money->Table(); Table != NULL; Table = Table->Next() )
			{
				// NEW
				Cash += Table->IDCash();
				Info +=
					"<moneysub "
					"id="+TStr(Table->ID())+" "
					"cash="+TStr(Table->IDCash())+" "
					"/>\n";
				// OLD
				query.execSql(TStr("REPLACE money_gamestable SET ")+
					"  flast=now() , "
					"  fBet="+TStr(Table->fBet) + " , "
					"  fCash="+TStr(Table->IDCash()) + " , "
					"  FID_Table=" + Table->ID() + " , "
					"  FID_Money=" + Money->ID() + " , "
					"  FID_Mode="  + Game->ModeID()  + " , "
					"  FID_Game="  + Game->ID() );
			}
			query.execSql(TStr("REPLACE money_games SET ")+
				"  flast=now() , "
				"  fBet='"+Money->Bet() + "' , "
				"  fCash='"+TStr(Cash)+"' , "
				"  fInfo='"+Info+"' , "
				"  FID_Money=" + Money->ID() + " , "
				"  FID_Mode="  + Game->ModeID()  + " , "
				"  FID_Game="  + Game->ID() );
		}
	}
}


int TCasinoControl::Log(
	m_classes::TMySqlConnect* db,
	const unsigned int /*UserID*/,
	const unsigned int /*ModeID*/,
	const unsigned int /*GameID*/,
	const unsigned int /*MoneyID*/,
	const double& Cash,
	const TStr& /*Info*/,
	const int LogID)
{
	m_classes::TMySqlQuery query(db);
	if( LogID > 0 ) {
/*
		if( Cash == 0 ) {
			query.execSql(TStr("DELETE FROM log_game_")+ModeID+" WHERE ID="+LogID);
			return 0;
		}
*/
/*
		query.execSql(TStr("UPDATE log_game_")+ModeID+" SET "+
			" finfo = finfo+'"+Info+"'," +
			" fcash = fcash+" + Cash +
			" WHERE ID=" + LogID);
*/
		return LogID;
	}
	if( Cash == 0 )
		return 0;
/*
	query.execSql(TStr("INSERT INTO log_game_")+ModeID+" "
			"(fdate,ftime,fid_user,fid_money,fid_game,fcash,finfo) VALUES "
			"(now(),now(),"+
			UserID+","+
			MoneyID+","+
			GameID+","
			"'"+TStr(Cash)+"',"
			"'"+Info+"'"+
			")");
	return query.last_id();
*/
	return 0;
}


int TCasinoControl::TRBet(
			m_classes::TMySqlConnect* db,
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			TTable* Table,
			const int LogID,
			const double& Bonus)
{
	if( Table == NULL )
		return 0;
//	TLockSection _l(&fLock);
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		throw TExcept("TCasinoControl::TRBet");
	double CashP = 1;
	if( Bonus > 0 )
		CashP = (Table->Cash() - Bonus) / Table->Cash();
	if( CashP < 0 )
		CashP = 0;
	for( TTable* pTable = Table; pTable != NULL; pTable = pTable->Next() )
	{
//		printf("id=%d cash=%f\n",pTable->ID(),pTable->IDCash());
		double DCash = pTable->IDCash();
		for( TMoney* pMoney = Game->Money(); pMoney != NULL; pMoney = pMoney->Next() )
		{
			double Cash = pMoney->Bet() * pTable->IDCash() / Game->Bet();
			if( pMoney->Next() == NULL )
				Cash = DCash;
			if( CashP*Cash > 0.00001 )
				pMoney->Table()->CashAdd(pTable->ID(),CashP*Cash);
			DCash -= Cash;
		}
		if( DCash > 0 )
			printf("!!! CASH !!! Table->ID()=%d   DCash=%f\n",Table->ID(),DCash);
		/*
		// !!! ошибка центов кроется тут!!!
		// поставить контрольную точку и проверить на слотах
		if( DCash > 0 )
			Game->Money()->Table()->CashAdd(Table->ID(),DCash);
		*/
	}
	return Log(db,UserID,ModeID,GameID,0,Table->Cash(),Table->XML(),LogID);
}


int TCasinoControl::TRCreate(
			const unsigned int UserID,
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			TTable* Table,
			const bool f)
{
//	TLockSection _l(&fLock);
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		throw TExcept("TCasinoControl::TRCreate");;
	bool ok = true;
	if( !f && Table != NULL )
		if( Table->Cash() > 0 )
			ok = Game->Money(MoneyID)->Table()->CheckMin(Table);
	if( ok ) {
		int i = FindTR();
		if( i > -1 ) {
			if( Table != NULL )
				Game->Money(MoneyID)->Table()->AddTable(Table,false);
			fTR[i].Open(Game->Money(MoneyID),UserID,Table);
			return i;
		}
	}
	return -100;
}


bool TCasinoControl::TRAdd(
			const int TRID,
			TTable* Table,
			const bool f)
{
	if( TRID < 0 )
		throw TExcept("TCasinoControl::TRAdd");;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRAdd");;
	if( Table == NULL )
		return true;
//	TLockSection _l(&fLock);
	bool ok = true;
	TMoney* pMoney = fTR[TRID].Money();
	if( !f )
		ok = pMoney->Table()->CheckMin(Table);
	if( ok ) {
		pMoney->Table()->AddTable(Table,false);
		fTR[TRID].Table()->AddTable(Table,true);
		return true;
	}
	return false;
}


bool TCasinoControl::TRUpdate(
			const int TRID,
			TTable* Table,
			const bool f)
{
	if( TRID < 0 )
		throw TExcept("TCasinoControl::TRUpdate");;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRUpdate");;
//	TLockSection _l(&fLock);
	bool ok = true;
	if( !f ) {
		for( TTable* pTable = Table; pTable != NULL; pTable = pTable->Next() )
		{
			double Cash = pTable->IDCash() - fTR[TRID].Table()->Cash(pTable->ID()); 
			if( fTR[TRID].Money()->Table()->Cash(pTable->ID()) < Cash ) {
				ok = false;
				break;
			}
		}
	}
	if( ok ) {
		fTR[TRID].Money()->Table()->AddTable(fTR[TRID].Table(),true);
		fTR[TRID].Table()->AddTable(fTR[TRID].Table(),false);
		if( Table != NULL ) {
			fTR[TRID].Money()->Table()->AddTable(Table,false);
			fTR[TRID].Table()->AddTable(Table,true);
		}
		return true;
	}
	return false;
}


bool TCasinoControl::TRJoin(const int TRID1, const int TRID2)
{
//	TLockSection _l(&fLock);
	if( TRID1 < 0 || TRID2 < 0 )
		return false;
	if( fTR[TRID1].isClose() )
		throw TExcept("TCasinoControl::TRJoin");;
	if( fTR[TRID2].isClose() )
		throw TExcept("TCasinoControl::TRJoin");;
	fTR[TRID1].Table()->AddTable(fTR[TRID2].Table(),true);
	fTR[TRID2].Close();
	return true;
}


int TCasinoControl::TRCommit(m_classes::TMySqlConnect* db, const int TRID)
{
	if( TRID < 0 )
		return 0;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRCommit");;
//	TLockSection _l(&fLock);
	int i = Log(
			db,
			fTR[TRID].UserID(),
			fTR[TRID].Money()->Game()->ModeID(),
			fTR[TRID].Money()->Game()->ID(),
			fTR[TRID].Money()->ID(),
			fTR[TRID].Cash(),
			fTR[TRID].Table()->XML(),
			0);
	fTR[TRID].Close();
	return i;
}


void TCasinoControl::TRRollback(const int TRID)
{
	if( TRID < 0 )
		return;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRRollback");;
//	TLockSection _l(&fLock);
	fTR[TRID].Money()->Table()->AddTable(fTR[TRID].Table(),true);
	fTR[TRID].Close();
}


unsigned int TCasinoControl::TRCash(const int TRID)
{
	if( TRID < 0 )
		return 0;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRCash");
	unsigned int f = fTR[TRID].Cash();
	return f;
}


TCasinoControl::TGame* TCasinoControl::TRGame(const int TRID)
{
	if( TRID < 0 )
		return 0;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRGame");;
	return fTR[TRID].Money()->Game();
}


TTable* TCasinoControl::TRTable(const int TRID)
{
	if( TRID < 0 )
		return 0;
	if( fTR[TRID].isClose() )
		throw TExcept("TCasinoControl::TRTable");;
	return fTR[TRID].Table();
}


bool TCasinoControl::UpdateBet(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const int Bet)
{
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		return false;
	if( Game->Money(MoneyID) == NULL )
		return false;
	if( Bet < 0 )
		return false;
	Game->BetUpdate(MoneyID,Bet);
	return true;
}


bool TCasinoControl::UpdateBet(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const unsigned int TableID,
			const int Bet)
{
	if( Bet < 0 )
		return false;
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		return false;
	TMoney* Money = Game->Money(MoneyID);
	if( Money == NULL )
		return false;
	Money->Table()->BetUpdate(TableID,Bet);
	return true;
}


bool TCasinoControl::UpdateCash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const double& Cash)
{
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		return false;
	if( Game->Money(MoneyID) == NULL )
		return false;
	//
	Game->Money(MoneyID)->CashUpdate(Cash);
/*
	// CHANGE !!!
	Game->Money(1)->Table()->Cash();
	double AllCash = Game->Money(MoneyID)->Table()->Cash();
	TTable* tb = NULL;
	if( AllCash == 0 ) {
		tb = Game->Money(MoneyID)->Table();
		while( tb != NULL )
		{
			if( tb->ID() != 0 ) {
				tb->CashAdd(tb->ID(),1);
				AllCash++;
			}
			tb = tb->Next();
		}
	}
	tb = Game->Money(MoneyID)->Table();
	while( tb != NULL )
	{
		tb->CashAdd(tb->ID(),tb->IDCash()*Cash/AllCash-1);
		tb = tb->Next();
	}
	// CHANGE !!!
*/
	return true;
}


bool TCasinoControl::UpdateCash(
			const unsigned int ModeID,
			const unsigned int GameID,
			const unsigned int MoneyID,
			const unsigned int TableID,
			const double& Cash)
{
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game == NULL )
		return false;
	TMoney* Money = Game->Money(MoneyID);
	if( Money == NULL )
		return false;
	//
	Money->Table()->CashAdd(TableID,Cash);
	return true;
}


TStr TCasinoControl::XMLInfo()
{
	int j = 0;
	for( unsigned int i = 0; i < cTRCount; i++ )
		if( fTR[i].UserID() > 0 && fTR[i].Money() != NULL && fTR[i].Table() != NULL )
			j++;
	return "<tr use=\""+TStr(j)+"\" size=\""+TStr(cTRCount)+"\"  />";
}

TStr TCasinoControl::XML()
{
//	TLockSection _l(&fLock);
	TStr s;
	s += "<games>\n";
	TGame* Game = TGame::Root();
	while( Game != NULL )
	{
		s += Game->XML();
		Game = Game->Next();
	}
	s += "</games>\n";
	s += "<tr>\n";
	for( unsigned int i = 0; i < cTRCount; i++ )
		if( fTR[i].UserID() > 0 && fTR[i].Money() != NULL && fTR[i].Table() != NULL ) {
			s += TStr("<tr")+
				" id=\""+i+"\""
				" user=\""+fTR[i].UserID()+"\""
				" money=\""+fTR[i].Money()->ID()+"\">\n";
			s += fTR[i].Table()->XML();
			s += "</tr>\n";
		}
	s += "</tr>\n";
	return s;
}

float TCasinoControl::Cash(
		const unsigned int ModeID,
		const unsigned int GameID,
		const unsigned int MoneyID,
		const unsigned int TableID)
{
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game != NULL ) {
		TMoney* Money = Game->Money(MoneyID);
		if( Money != NULL )
			return Money->Table()->Cash(TableID);
	}
	return 0;
}

unsigned int TCasinoControl::Bet(
		const unsigned int ModeID,
		const unsigned int GameID,
		const unsigned int MoneyID)
{
	TGame* Game = TGame::Find(ModeID,GameID);
	if( Game != NULL ) {
		TMoney* Money = Game->Money(MoneyID);
		if( Money != NULL )
			return (int)(Money->Table()->Cash()+0.001);
	}
	return 0;
}

           








