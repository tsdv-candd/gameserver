#include "mGameSlots5line.h"
#include "server/mUser.h"
#include "server/mServer.h"
#include "server/mCommon.h"

const int TSlotsSession::fWheelLine[5][3] = {
	 {  0,  0,  0 },
	 {  1,  1,  1 },
	 { -1, -1, -1 },
	 {  1,  0, -1 },
	 { -1,  0,  1 }
};

/*
7        7 - 1000 - 1     0.000125    1/8000
3BAR     6 -  500 - 2     0.000125    8/8000
2BAR     5 -  100 - 2     0.001000    8/8000
1BAR     4 -   50 - 2     0.001125    8/8000
колокол  3 -   20 - 5     0.008
лимон    2 -   20 - 5     0.008
вишня    1 -   15 - 4     0.015625

any BAR        20 - 6     0.027
вишня 2         5         0.125
вишня 1         2         0.5 
*/

TSlotsSession::TWin TSlotsSession::fWheelWin[36] = {
	{ 7, 7, 7, 1 },
	{ 6, 6, 6, 2 },
	{ 5, 5, 5, 3 },
	{ 4, 4, 4, 4 },
	{ 4, 4, 5, 5 },
	{ 4, 4, 6, 5 },
	{ 4, 5, 4, 5 },
	{ 4, 5, 5, 5 },
	{ 4, 5, 6, 5 },
	{ 4, 6, 4, 5 },
	{ 4, 6, 5, 5 },
	{ 4, 6, 6, 5 },
	{ 5, 4, 4, 5 },
	{ 5, 4, 5, 5 },
	{ 5, 4, 6, 5 },
	{ 5, 5, 4, 5 },
	{ 5, 5, 6, 5 },
	{ 5, 6, 4, 5 },
	{ 5, 6, 5, 5 },
	{ 5, 6, 6, 5 },
	{ 6, 4, 4, 5 },
	{ 6, 4, 5, 5 },
	{ 6, 4, 6, 5 },
	{ 6, 5, 4, 5 },
	{ 6, 5, 5, 5 },
	{ 6, 5, 6, 5 },
	{ 6, 6, 4, 5 },
	{ 6, 6, 5, 5 },
	{ 3, 3, 3, 7 },
	{ 2, 2, 2, 6 },
	{ 1, 1, 1, 8 },
	{ 0, 1, 1, 9 },
	{ 1, 1, 0, 10 },
	{ 1, 0, 1, 13 },
	{ 0, 0, 1, 11 },
	{ 1, 0, 0, 12 }
};

unsigned int TSlotsSession::fWheelID[3][20];

TSlotsProducer Slots;

const unsigned int cSlots1MASCOUNT = 14;

unsigned int Slots1MasWin[cSlots1MASCOUNT] = {
	2,
	4,
	5,
	15,
	20,
	20,
	20,
	50,
	100,
	500,
	1000,
	2000,
	3000,
	4000
};

double Slots1MasBet[cSlots1MASCOUNT] = {
	2400,
	1200,
	7700,
	600,
	4500,
	4000,
	6000,
	490,
	280,
	50,
	20,
	8,
	5,
	2
};

unsigned int Slots1MasP[cSlots1MASCOUNT] = {
	3,
	50,
	50,
	50,
	30,
	30,
	30,
	30,
	30,
	10,
	10,
	10,
	10
};

int LineIDToTableID1(const unsigned int ID, const unsigned int Coin)
{
	switch( ID )
	{
			case 1:            // 777
					switch( Coin )
					{
					case 1: return 10;
					case 2: return 11;
					case 3: return 12;
					case 4: return 13;
					}
					return 14;
			case 2:  return 9; // 3BAR
			case 3:  return 8; // 2BAR
			case 4:  return 7; // 1BAR
			case 5:  return 6; // any BAR
			case 6:  return 5; // лимон
			case 7:  return 4; // колокол
			case 8:  return 3; // 3 вишни
			case 9:  return 2; // 2 вишни справа
			case 10: return 2; // 2 вишни слева
			case 13: return 1; // вишня слева и справа
			case 11: return 0; // вишня справа
			case 12: return 0; // вишня слева
	}
	return -1;
}

TSlotsSession::TSlotsSession()
: fStatus(tBet), fCashWin(0)
{
}

unsigned int ChanceToCash(const unsigned int Chance, const unsigned int ID)
{
	switch( Chance )
	{
		case 1: return 2;
		case 2: return 5;
		case 3: return 10;
		case 4: return 20;
		case 5: return 50;
		case 6: return 100;
	}
	return ID;
}

class TGame {
private:
	unsigned int Mas[1000];
	unsigned int iMas;
	unsigned int iCount;
	
	unsigned int fLineID[5];
	unsigned int fWheelID[5][3];

	void Wheel() {
		unsigned int i;
		for( i = 0; i < 5; i++ )
		{
			for( unsigned int j = 0; j < 3; j++ )
			{
				int k = fWheel[j] + TSlotsSession::fWheelLine[i][j];
				if( k < 0 )
					k = 19;
				else
				if( k > 19 )
					k = 0;
				fWheelID[i][j] = TSlotsSession::fWheelID[j][k];
			}
		}
		for( i = 0; i < 5; i++ )
		{
			fLineID[i] = 0;
			for( unsigned int j = 0; j < 36; j++ )
			{
				if(
					(TSlotsSession::fWheelWin[j].wheel1 == fWheelID[i][0] || TSlotsSession::fWheelWin[j].wheel1 == 0 ) &&
					(TSlotsSession::fWheelWin[j].wheel2 == fWheelID[i][1] || TSlotsSession::fWheelWin[j].wheel2 == 0 ) &&
					(TSlotsSession::fWheelWin[j].wheel3 == fWheelID[i][2] || TSlotsSession::fWheelWin[j].wheel3 == 0 ))
				{
					fLineID[i] = TSlotsSession::fWheelWin[j].pay;
					break;
				}
			}
		}
	}
public:
	unsigned int fWheel[3];

	TGame(): iCount(0) {
//		iMas = MasLoad(Mas,"test_slots.txt");
		iMas = 0;
	}

	void Bet(const unsigned int CashBet, const unsigned int Coin) {
		for( unsigned int i = 0; i < 3; i++ )
			if( iCount < iMas )
				fWheel[i] = Mas[iCount++]-1;
			else
				fWheel[i] = getRnd(20);
		Wheel();
	}

	unsigned int Line(const unsigned int i) {
		return fLineID[i];
	}

	TStr Result(const unsigned int CashBet, const unsigned int Coin) {
		TStr s;
		TServerSSI rs;
		unsigned int i;
		for( i = 0; i < 3; i++ )
		{
			rs.ssi()->SSIValue("ID",i+1);
			rs.ssi()->SSIValue("SYMBOL",fWheel[i]+1);
			s += rs.ssi()->SSIRes("slots_bet_wheel");
		}
		for( i = 0; i < Coin; i++ )
			if( fLineID[i] > 0 ) {

				int iMas = LineIDToTableID1(fLineID[i],i+1);
				if( iMas >= 0 ) {
					if( iMas < 14 )
						rs.ssi()->SSIValue("CASH",CashBet*Slots1MasWin[iMas]);
					rs.ssi()->SSIValue("ID",i+1);
					rs.ssi()->SSIValue("WINID",fLineID[i]);
					s += rs.ssi()->SSIRes("slots_line");
				}
			}
		return s;
	}

	TStr Log(const unsigned int Coin) {
		TStr s;
		s += TStr(
			"  <symbol id=\"1\" type=\"") + fWheelID[1][0] + "\" />" +
			"  <symbol id=\"2\" type=\"" + fWheelID[0][0] + "\" />" +
			"  <symbol id=\"3\" type=\"" + fWheelID[2][0] + "\" />" +
			"  <symbol id=\"4\" type=\"" + fWheelID[1][1] + "\" />" +
			"  <symbol id=\"5\" type=\"" + fWheelID[0][1] + "\" />" +
			"  <symbol id=\"6\" type=\"" + fWheelID[2][1] + "\" />" +
			"  <symbol id=\"7\" type=\"" + fWheelID[1][2] + "\" />" +
			"  <symbol id=\"8\" type=\"" + fWheelID[0][2] + "\" />" +
			"  <symbol id=\"9\" type=\"" + fWheelID[2][2] + "\" />";
		for( unsigned int i = 0; i < Coin; i++ )
			if( fLineID[i] == 0 )
				s += TStr("<line id=\"")+TStr(i+1)+"\" />";
			else
				s += TStr("<line id=\"")+TStr(i+1)+"\" win=\""+fLineID[i]+"\" />";
		return s;
	}
};

void TSlotsSession::OnBet(TRequest *r)
{
	r->User()->CashRollback();
	TServerSSI rs;
	fChance = fCashWin = 0;

	TXMLNode* bet = r->xml()->FindNode("bet");
	if( bet != NULL ) {
		unsigned int CashBet = bet->VarValue("cash").ToIntDef(0);
		unsigned int Coin = bet->VarValue("coin").ToIntDef(0);
		if( Coin > 5 )
			Coin = 5;
		if( CashBet > 0 && Coin > 0 ) {
			if( r->User()->Status() == tGIFT )
				CashBet = 1;
			double DCash = CashBet*Coin;
//			unsigned int iCoin = Coin - 1;
			TTable TableBet(BetCashToTable(CashBet,0),CashBet*Coin*Slots1MasBet[0]);
			DCash -= CashBet*Coin*Slots1MasBet[0];
			for( unsigned int i = 1; i < cSlots1MASCOUNT; i++ )
			{
				TableBet.CashAdd(BetCashToTable(CashBet,i),CashBet*Coin*Slots1MasBet[i]);
				DCash -= CashBet*Coin*Slots1MasBet[i];
			}
			TableBet.CashAdd(BetCashToTable(CashBet,0),DCash);

			if( !r->User()->CashBetCommit(cGameSlots,&TableBet) )
				return;

			TGame game;
			bool f = true;
			int iWin;
			for( iWin = Coin+cSlots1MASCOUNT-6; iWin >= 0; iWin-- )
			{
				if(
					r->User()->Cash(cGameSlots,BetCashToTable(CashBet,iWin)) >
					int(CashBet*Slots1MasWin[iWin])
				) {
					int p = Slots1MasP[iWin];
					if( p <= 1 )
						break;
					if( getRnd(p) == 0 )
						break;
				}
			}
			int Jackpot = 0;
			do {
				Jackpot = 0;
				f = false;
				game.Bet(CashBet,Coin);

				TTable TableWin(0,0);
				Jackpot = 0;
				for( unsigned int i = 0; i < Coin; i++ )
				{
					int iMas = LineIDToTableID1(game.Line(i),i+1);
					if( iMas >= 0 )
					{
						if( iWin >= 0 && iMas == iWin )
							f = true;
						if( iMas == 14 ) {
/*
							if( r->User()->Jackpot(cGameSlots) > 5000000 ) {
								Jackpot = (double)r->User()->Jackpot(cGameSlots) * ((double)CashBet / 500.0);
								r->User()->JackpotUpdate(cGameSlots,-Jackpot);
								f = true;
							} else {
*/
							{
								Jackpot = 1;
								f = false;
							}
							if( iWin == iMas )
								iWin = -1;
							break;
						} else
						if( TableWin.IDCash() == 0 )
							TableWin.Update(BetCashToTable(CashBet,iMas),CashBet*Slots1MasWin[iMas]);
						else
							TableWin.CashAdd(BetCashToTable(CashBet,iMas),CashBet*Slots1MasWin[iMas]);
					}
				}

				int Cash1 = (int)TableWin.Cash();

				if( Jackpot == 0 ) {
					if( iWin < 0 || iWin >= 0 && f ) {
/*
						if( ((int)TableWin.Cash()) == 0 )
							f = r->User()->CashWinUpdate(cGameSlots,NULL);
						else
*/
						if( Cash1 == 0 )
							f = r->User()->CashWinUpdate(cGameSlots,&TableWin);
						else
							f = r->User()->CashWinUpdate(cGameSlots,&TableWin);
						if( f )
							fCashWin = (int)(TableWin.Cash());
						if( !f && iWin >= 0 )
							iWin = -1;
					}
				} else
				if( f )
					fCashWin = Jackpot;
			} while( !f );
			rs.ssi()->SSIValue("CASHBET",CashBet*Coin);
			rs.ssi()->SSIValue("CASHWIN",fCashWin);
			TStr s(game.Result(CashBet,Coin));
/*
			if( Jackpot > 0 )
				r->User()->CashJackpotCommit(game.Log(Coin),cGameSlots,Jackpot);
			else
*/
			r->User()->CashWinCommit(game.Log(Coin));
			r->User()->CashCommit();

			if( Jackpot == 0 && fCashWin > 0 ) {
				unsigned int CashWin = 0;
				int iCount = 0;
				do {
					fChance = 0;
					int f1 = getRnd(100) <= 49;
					int f2 = getRnd(100) <= 39;
					int f3 = getRnd(100) <= 49;
					int f4 = getRnd(100) <= 49;
					int f5 = getRnd(100) <= 39;
					int f6 = getRnd(100) <= 49;
					if( f1 == 0 )
						if( f2 == 0 )
							if( f3 == 0 )
								if( f4 == 0 )
									if( f5 == 0 )
										if( f6 == 0 )
											fChance = 6;
										else
											fChance = 5;
									else
										fChance = 4;
								else
									fChance = 3;
							else
								fChance = 2;
						else
							fChance = 1;
#ifdef TARGET_WIN32
					fChance = 6;
#endif
					CashWin = fCashWin * ChanceToCash(fChance,0);
				} while( !r->User()->CashWinUpdate(cGameSlotsChance,BetCashToTable(CashBet),CashWin,iCount++ > 100) );
				rs.ssi()->SSIValue("CHANCEID",fChance);
				fStatus = tGame;
				r->AddStatus("game");
				r->User()->CashBetAdd(BetCashToTable(fCashWin),fCashWin);
			} else {
				fStatus = tBet;
				r->AddStatus("ok");
			}
			s += rs.ssi()->SSIRes("slots_game");
			r->AddReturn(s);
		}
	}
}

void TSlotsSession::OnGame(TRequest *r)
{
	if( fStatus != tGame )
		return;
	TXMLNode* chance = r->xml()->FindNode("chance");
	if( chance != NULL ) {
		TServerSSI rs;
		if( chance->VarValue("ID") == "0" || chance->VarValue("ID") == "-1" )
			r->User()->CashRollback();
		else {
			unsigned int BetTable = r->User()->CashBet()->ID();
			r->User()->CashBetCommit(cGameSlotsChance);
			if( chance->VarValue("ID") != "lose" ) {
				unsigned int c = chance->VarValue("ID").ToIntDef(0);
				if( c < fChance )
					fChance = c;
				fCashWin *= ChanceToCash(fChance,1);
			} else {
				fCashWin = 0;
				fChance = 0;
			}
			r->User()->CashWinUpdate(cGameSlotsChance,BetTable,fCashWin);
			r->User()->CashWinCommit(TStr("<chance id=\"")+fChance+"\" />");
			r->User()->CashCommit();

			r->User()->ToSSI(rs.ssi());
			r->AddReturn(rs.ssi()->SSIRes("server_user"));
		}
		rs.ssi()->SSIValue("CASHWIN",fCashWin);
		fStatus = tBet;
		r->AddStatus("ok");
	}
}

void TSlotsSession::Run(TRequest *r)
{
	ISession::Run(r);
	TServerSSI rs;
	if( !r->isStatus() ) {
		if( r->cm() == "bet" ) {
			OnBet(r);
			r->User()->ToSSI(rs.ssi());
			r->AddReturn(rs.ssi()->SSIRes("server_user"));
		} else
		if( r->cm() == "game" )
			OnGame(r);
	}
	int Jackpot = r->User()->JackpotID(cJackpotSlots);
	rs.ssi()->SSIValue("CASH",Jackpot);
	r->AddReturn(rs.ssi()->SSIRes("server_jackpot"));
}

TSlotsProducer::TSlotsProducer()
	: ISessionProducer(cGameSlots,cSlotsName,cSlotsSize)
{
	unsigned int i;
/*
	TFile f("game_slots.ini",false);
	for( i = 0; i < 36; i++ )
	{
		TStr str(Read(f.stream));
		TSlotsSession::fWheelWin[i].wheel1 = str.ToInt();
		str = Read(f.stream);
		TSlotsSession::fWheelWin[i].wheel2 = str.ToInt();
		str = Read(f.stream);
		TSlotsSession::fWheelWin[i].wheel3 = str.ToInt();
		str = Read(f.stream);
		TSlotsSession::fWheelWin[i].pay = str.ToInt();
	}
*/
	m_ssi::TSSIControl ssi;
	ssi.SSILoad("server_slots.ini");
	for( i = 0; i < 3; i++ )
	{
		TXML xml;
		xml.FromStr(ssi.SSIRes("slots_wheel"+TStr(i+1)).c_str());
		for( unsigned int j = 0; j < 20; j++ )
			TSlotsSession::fWheelID[i][xml.GetNode(j)->VarValue("id").ToInt()-1] = xml.GetNode(j)->VarValue("type").ToInt();
	}
/*
	TXML xml;
	xml.LoadFromFile("game_slots.xml");
	TXMLNode* slots = xml.FindNode("slots");
	for( i = 0; i < 5; i++ )
	{
		unsigned int j;
		for( j = 0; j <cSlots1MASCOUNT; j++ )
		{
			Slots1MasWin[j][i] = 0;
			Slots1MasBet[j][i] = 0;
		}
		double Bet = 0;
		TXMLNode* p = slots->GetNode(i);
		for( j = 0; j < p->NodesCount(); j++ )
		{
			Slots1MasWin[j][i] = p->GetNode(j)->VarValue("win").ToInt();
			Slots1MasBet[j][i] = p->GetNode(j)->VarValue("bet").ToInt(); // *Slots1MasWin[j][i]
			Slots1MasP[j][i] = p->GetNode(j)->VarValue("p").ToInt();
			Bet += Slots1MasBet[j][i];
		}
		for( j = 0; j < p->NodesCount(); j++ )
			Slots1MasBet[j][i] /= Bet;
	}
*/
	double Bet = 0;
	int j;
	for( j = 0; j < cSlots1MASCOUNT; j++ )
		Bet += Slots1MasBet[j];
	for( j = 0; j < cSlots1MASCOUNT; j++ )
		Slots1MasBet[j] /= Bet;
}

void TSlotsSession::OnStart(TUser* User, const int i,const bool f)
{
	TServerSSI rs;
	fCashWin = 0;
	ISession::OnStart(User,i,f);
}

void TSlotsSession::OnLeave(TUser* User)
{
	User->CashRollback();
	ISession::OnLeave(User);
}




