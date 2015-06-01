#include "mGameKeno.h"
#include "server/mUser.h"
#include "server/mServer.h"
#include "utils/m_utils.h"

TKeno Keno;

unsigned int TKeno::fWin[10][10] = {
	{ 2,  0,  0,   0,    0,    0,     0,     0,     0,      0 },
	{ 1, 10,  0,   0,    0,    0,     0,     0,     0,      0 },
	{ 0,  2, 50,   0,    0,    0,     0,     0,     0,      0 },
	{ 0,  2, 15, 100,    0,    0,     0,     0,     0,      0 },
	{ 0,  1,  5,  15, 1000,    0,     0,     0,     0,      0 },
	{ 0,  0,  5,  25,  150, 2500,     0,     0,     0,      0 },
	{ 0,  0,  2,  10,   80,  500, 10000,     0,     0,      0 },
	{ 0,  0,  1,   5,   25,  250,  5000, 30000,     0,      0 },
	{ 0,  0,  0,   2,   10,  150,  2500, 15000, 50000,      0 },
	{ 0,  0,  0,   1,    5,   50,   500,  7500, 25000, 100000 }
};

class TKenoGame {
private:
	unsigned int fNumber[10];
	unsigned int fNumberCount;
	unsigned int fWin[20];
	unsigned int fWinCash;
public:
	TKenoGame() : fNumberCount(0) {}

	void AddNumber( const unsigned int Num ) {
		if( fNumberCount < 10 )
			fNumber[fNumberCount++] = Num;
	}

	bool Bet() {
		return fNumberCount >= 1;
	}

	unsigned int Game( const unsigned int CashBet ) {
		fWinCash = 0;
		if( fNumberCount > 0 ) {
			unsigned int Win = 0;
/*
			unsigned int Mas[20];
			int iMas = MasLoad(Mas,"test_keno.txt");
			for( int i = 0; i < iMas; i++ )
				fWin[i] = Mas[i];
			getRnd(fWin+iMas,20-iMas,1,80,Mas,iMas);
*/
			getRnd(fWin,20,1,80);
			unsigned int Hits = 0;
			for( unsigned int j = 0; j < 20; j++ )
			{
				for( unsigned int k = 0; k < fNumberCount; k++ )
					if( fWin[j] == fNumber[k] ) {
						Hits++;
						break;
					}
			}
			if( Hits > 0 )
				fWinCash = TKeno::fWin[fNumberCount-1][Hits-1]*CashBet;
		}
		return fWinCash;
	}

	TStr Log() {
		TStr fLog;
		for( unsigned int i = 0; i < fNumberCount; i++ )
			fLog += "<bet id=\""+TStr(fNumber[i])+"\" />";
		for( unsigned int j = 0; j < 20; j++ )
			fLog += "<win id=\""+TStr(fWin[j])+"\" />";
		return fLog;
	}

	TStr Result() {
		TServerSSI rs;
		TStr s;
		for( unsigned int j = 0; j < 20; j++ )
		{
			rs.ssi()->SSIValue("NUM",fWin[j]);
			s += rs.ssi()->SSIRes("keno_game_round_win");
		}
		rs.ssi()->SSIValue("STREAM",s);
		rs.ssi()->SSIValue("CASHWIN",fWinCash);
		return rs.ssi()->SSIRes("keno_game_round");
	}
};

void TKeno::Run(TRequest *r)
{
	TGameRequest::Run(r);
	if( !r->isStatus() && r->cm() == "game" ) {
		TKenoGame keno;
		unsigned int CashBet = 0;
		unsigned int i = r->xml()->NodesCount();
		int Round = 0;
		while( i-- > 0 )
		{
			TXMLNode *p = r->xml()->GetNode(i);
			if( p->GetName() == "bet" ) {
				if( r->User()->Status() == tGIFT )
					CashBet = 1;
				else
					CashBet = p->VarValue("cash").ToIntDef(0);
				Round = p->VarValue("round").ToIntDef(1);
			} else
			if( p->GetName() == "number" )
				keno.AddNumber(p->VarValue("id").ToIntDef(0));
		}
		if( keno.Bet() && CashBet > 0 ) {
			unsigned int GameTable = BetCashToTable(CashBet);
			TServerSSI rs;
			rs.ssi()->SSIValue("CASH",r->User()->Cash()-CashBet*Round);
			rs.ssi()->SSIValue("BONUS",r->User()->CashBonus());
			r->AddReturn(rs.ssi()->SSIRes("server_user"));
			unsigned int CashWin = 0;
			rs.ssi()->SSIValue("CASHBET",CashBet*Round);
			while( Round-- > 0 ) {
				if( r->User()->CashBetCommit(cGameKeno,GameTable,CashBet) ) {
					unsigned int Win;
					int i = 0;
					while( !r->User()->CashWinUpdate(cGameKeno,GameTable,Win = keno.Game(CashBet),i >= 100) )
						if( i++ > 100 )
							break;
					CashWin += Win;
					r->User()->CashWinCommit(keno.Log());
					r->User()->CashCommit();
					r->AddReturn(keno.Result());
				}
			}
			rs.ssi()->SSIValue("CASHWIN",CashWin);
			r->AddReturn(rs.ssi()->SSIRes("server_gamebet"));
			r->AddStatus("ok");
		}
	}
}

TKeno::TKeno()
: TGameRequest(cGameKeno,cKenoName)
{}
