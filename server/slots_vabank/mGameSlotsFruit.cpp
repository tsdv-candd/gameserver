#include "mGameSlotsFruit.h"
#include "server/mUser.h"
#include "server/mServer.h"
#include "server/mCommon.h"

const int TSlots2Session::fWheelLine[3][3] = {
	 {  0,  0,  0 },
	 {  1,  1,  1 },
	 { -1, -1, -1 }
};

/*
7        9
3BAR     8
2BAR     7
1BAR     6
арбуз    5
апельсин 4
колокол  3
лимон    2
вишня    1
*/

TSlots2Session::TWin TSlots2Session::fWheelWin[39] = {
	{ 9, 9, 9, 10 },
	{ 8, 8, 8, 9  },
	{ 7, 7, 7, 8  },
	{ 6, 6, 6, 7  },
	{ 7, 7, 8, 6  },
	{ 7, 7, 6, 6  },
	{ 7, 8, 7, 6  },
	{ 7, 8, 8, 6  },
	{ 7, 8, 6, 6  },
	{ 7, 6, 7, 6  },
	{ 7, 6, 8, 6  },
	{ 7, 6, 6, 6  },
	{ 8, 7, 8, 6  },
	{ 8, 7, 7, 6  },
	{ 8, 7, 6, 6  },
	{ 8, 8, 7, 6  },
	{ 8, 8, 6, 6  },
	{ 8, 6, 7, 6  },
	{ 8, 6, 8, 6  },
	{ 8, 6, 6, 6  },
	{ 6, 7, 7, 6  },
	{ 6, 7, 8, 6  },
	{ 6, 7, 6, 6  },
	{ 6, 8, 7, 6  },
	{ 6, 8, 8, 6  },
	{ 6, 8, 6, 6  },
	{ 6, 6, 7, 6  },
	{ 6, 6, 8, 6  },
	{ 5, 5, 5, 5  },
	{ 4, 4, 4, 15 },
	{ 3, 3, 3, 4  },
	{ 2, 2, 2, 14 },
	{ 1, 1, 1, 3  },
	{ 0, 1, 1, 2  },
	{ 1, 1, 0, 12 },
	{ 1, 0, 1, 16 },
	{ 0, 0, 1, 1  },
	{ 1, 0, 0, 11 }
};

unsigned int         TSlots2Session::fWheelID[3][20];

TSlots2Producer Slots2;

const unsigned int cSlots2MASCOUNT = 20;

unsigned int Slots2MasWin[cSlots2MASCOUNT] = {
	2,
	4,
	5,
	10,
	15,
	15,
	20,
	20,
	25,
	50,
	100,
	300,
	1000,
	50,
	100,
	300,
	1000,
	50,
	100,
	300
};

double Slots2MasBet[cSlots2MASCOUNT] = {
	20000,
	4500,
	7700,
	3000,
	2500,
	2000,
	1500,
	1300,
	1000,
	200,
	100,
	50,
	5,
	200,
	100,
	50,
	5,
	200,
	100,
	50
};

unsigned int Slots2MasP[cSlots2MASCOUNT] = {
	50,
	50,
	50,
	30,
	30,
	30,
	15,
	15,
	15,
	15,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100,
	100
};

int LineIDToTableID2(const unsigned int ID, const unsigned int Coin)
{
	// 12 - 16 - 19
	switch( ID )
	{
		case 10:               // 777
			switch( Coin )
			{
				case 3: return 20;
				case 2: return 16;
			}
			return 12;
		case 9:                // 3BAR
			switch( Coin )
			{
				case 3: return 19;
				case 2: return 15;
			}
			return 11;
		case 8:                // 2BAR
			switch( Coin )
			{
				case 3: return 18;
				case 2: return 14;
			}
			return 10;
		case 7:                // 1BAR
			switch( Coin )
			{
				case 3: return 17;
				case 2: return 13;
			}
			return 9;
		case 6:  return 8;  // any BAR
		case 5:  return 7;  // арбуз
		case 15: return 6;  // апельсин
		case 4:  return 5;  // колокол
		case 14: return 4;  // лимон
		case 3:  return 3;  // 3 вишни
		case 2:  return 2;  // 2 вишни справа
		case 12: return 2;  // 2 вишни слева
		case 16: return 1;  // вишня слева и справа
		case 1:  return 0;  // вишня справа
		case 11: return 0;  // вишня слева
	}
	return -1;
}

class TSlots2Game {
private:
	unsigned int Mas[1000];
	unsigned int iMas;
	unsigned int iCount;

	unsigned int fWheel[3];
	unsigned int fLineID[3];
	unsigned int fWheelID[3][3];

	void Wheel() {
		unsigned int i;
		for( i = 0; i < 3; i++ )
		{
			for( unsigned int j = 0; j < 3; j++ )
			{
				int k = fWheel[j] + TSlots2Session::fWheelLine[i][j];
				if( k < 0 )
					k = 19;
				else
				if( k > 19 )
					k = 0;
				fWheelID[i][j] = TSlots2Session::fWheelID[j][k];
			}
		}

/*
		char s[1024];
		sprintf(s,"%d %d %d\n",fWheelID[1][0],fWheelID[1][1],fWheelID[1][2]);
		print(s);
		sprintf(s,"%d %d %d\n",fWheelID[0][0],fWheelID[0][1],fWheelID[0][2]);
		print(s);
		sprintf(s,"%d %d %d\n",fWheelID[2][0],fWheelID[2][1],fWheelID[2][2]);
		print(s);
*/

		for( i = 0; i < 3; i++ )
		{
			fLineID[i] = 0;
			for( unsigned int j = 0; j < 39; j++ )
			{
				if(
					(TSlots2Session::fWheelWin[j].wheel1 == fWheelID[i][0] || TSlots2Session::fWheelWin[j].wheel1 == 0 ) &&
					(TSlots2Session::fWheelWin[j].wheel2 == fWheelID[i][1] || TSlots2Session::fWheelWin[j].wheel2 == 0 ) &&
					(TSlots2Session::fWheelWin[j].wheel3 == fWheelID[i][2] || TSlots2Session::fWheelWin[j].wheel3 == 0 ))
				{
					fLineID[i] = TSlots2Session::fWheelWin[j].pay;
					break;
				}
			}
		}
	}
public:
	TSlots2Game(): iCount(0) {
//		iMas = MasLoad(Mas,"test_slots2.txt");
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
			s += rs.ssi()->SSIRes("slots2_bet_wheel");
		}
		for( i = 0; i < Coin; i++ )
			if( fLineID[i] > 0 ) {
				rs.ssi()->SSIValue("ID",i+1);
				rs.ssi()->SSIValue("WINID",fLineID[i]);

				int iMas = LineIDToTableID2(fLineID[i],i+1);
				if( iMas >= 0 && iMas < 20 )
					rs.ssi()->SSIValue("CASH",CashBet*Slots2MasWin[iMas]);

				s += rs.ssi()->SSIRes("slots2_line");
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

void TSlots2Session::OnBet(TRequest *r)
{
	TServerSSI rs;
	TXMLNode* bet = r->xml()->FindNode("bet");
	if( bet != NULL ) {
		TServerSSI rs;
		unsigned int CashBet = bet->VarValue("cash").ToIntDef(0);
		unsigned int Coin = bet->VarValue("coin").ToIntDef(0);
		if( Coin > 3 )
			Coin = 3;
		if( CashBet > 0 && Coin > 0 ) {
			if( r->User()->Status() == tGIFT )
				CashBet = 1;
/*
			unsigned int BetTable = BetCashToTable(CashBet);
			r->User()->CashBetAdd(BetTable,CashBet*Coin);
			r->User()->CashBetCommit(cGameSlots2);
*/
			double DCash = CashBet*Coin;
			unsigned int iCoin = Coin - 1;

			TTable TableBet(BetCashToTable(CashBet,0),CashBet*Coin*Slots2MasBet[0]);
			DCash -= CashBet*Coin*Slots2MasBet[0];
			for( unsigned int i = 1; i < cSlots2MASCOUNT; i++ )
			{
				TableBet.CashAdd(BetCashToTable(CashBet,i),CashBet*Coin*Slots2MasBet[i]);
				DCash -= CashBet*Coin*Slots2MasBet[i];
			}
			TableBet.CashAdd(BetCashToTable(CashBet,0),DCash);

			if( !r->User()->CashBetCommit(cGameSlots2,&TableBet) )
				return;

			TSlots2Game game;
			int fCashWin;
			bool f;
			int iWin = 12+(iCoin*4);
			if( iWin > 19 )
				iWin = 19;
			for( ; iWin >= 0; iWin-- )
			{
				if(
					r->User()->Cash(cGameSlots2,BetCashToTable(CashBet,iWin)) >
					int(CashBet*Slots2MasWin[iWin])
				) {
					int p = Slots2MasP[iWin];
					if( p == 1 )
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
				for( unsigned int i = 0; i < Coin; i++ )
				{
					int iMas = LineIDToTableID2(game.Line(i),i+1);
					if( iMas == 20 ) {
/*
						if( r->User()->Jackpot(cGameSlots2) > 5000000 ) {
							Jackpot = r->User()->Jackpot(cGameSlots2) * (CashBet / 500.0);
							r->User()->JackpotUpdate(cGameSlots2,-Jackpot);
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
					}
					if( iMas >= 0 ) {
						if( iWin >= 0 && iMas == iWin )
							f = true;
						if( TableWin.IDCash() == 0 )
							TableWin.Update(BetCashToTable(CashBet,iMas),CashBet*Slots2MasWin[iMas]);
						else
							TableWin.CashAdd(BetCashToTable(CashBet,iMas),CashBet*Slots2MasWin[iMas]);
					}
				}
				if( Jackpot == 0 ) {
					if( iWin < 0 || iWin >= 0 && f ) {
						if( TableWin.IDCash() == 0 )
							f = r->User()->CashWinUpdate(cGameSlots2,NULL);
						else
							f = r->User()->CashWinUpdate(cGameSlots2,&TableWin);
						fCashWin = (int)(TableWin.Cash());
						if( !f && iWin >= 0 )
							iWin = -1;
					}
				} else
				if( f )
					fCashWin = Jackpot;
			} while( !f );

			TStr s(game.Result(CashBet,Coin));
			r->UserToSSI(rs.ssi());
			s += rs.ssi()->SSIRes("server_user");
			rs.ssi()->SSIValue("CASHBET",CashBet*Coin);
			rs.ssi()->SSIValue("CASHWIN",fCashWin);
/*
			if( Jackpot > 0 )
				r->User()->CashJackpotCommit(game.Log(Coin),cGameSlots2,Jackpot);
			else
*/
			r->User()->CashWinCommit(game.Log(Coin));
			r->User()->CashCommit();
			r->AddStatus("ok");
			s += rs.ssi()->SSIRes("slots2_game");
			r->AddReturn(s);
		}
	}
}

void TSlots2Session::Run(TRequest *r)
{
	ISession::Run(r);
	OnBet(r);
	TServerSSI rs;
	int Jackpot = r->User()->JackpotID(cJackpotSlots);
	rs.ssi()->SSIValue("CASH",Jackpot);
	r->AddReturn(rs.ssi()->SSIRes("server_jackpot"));
}

TSlots2Producer::TSlots2Producer()
	: ISessionProducer(cGameSlots2,cSlots2Name,cSlots2Size)
{
	unsigned int i;
/*
	TFile f("game_slots2.ini",false);
	for( i = 0; i < 39; i++ )
	{
		TStr str(Read(f.stream));
		TSlots2Session::fWheelWin[i].wheel1 = str.ToInt();
		str = Read(f.stream);
		TSlots2Session::fWheelWin[i].wheel2 = str.ToInt();
		str = Read(f.stream);
		TSlots2Session::fWheelWin[i].wheel3 = str.ToInt();
		str = Read(f.stream);
		TSlots2Session::fWheelWin[i].pay = str.ToInt();
	}
*/
	m_ssi::TSSIControl ssi;
	ssi.SSILoad("server_slots2.ini");
	for( i = 0; i < 3; i++ )
	{
		TXML xml;
		xml.FromStr(ssi.SSIRes("slots2_wheel"+TStr(i+1)).c_str());
		for( unsigned int j = 0; j < 20; j++ )
			TSlots2Session::fWheelID[i][xml.GetNode(j)->VarValue("id").ToInt()-1] = xml.GetNode(j)->VarValue("type").ToInt();
	}
/*
	TXML xml;
	xml.LoadFromFile("game_slots2.xml");
	TXMLNode* slots = xml.FindNode("slots");
	for( i = 0; i < 3; i++ )
	{
		unsigned int j;
		for( j = 0; j < cSlots2MASCOUNT; j++ )
		{
			Slots2MasWin[j][i] = 0;
			Slots2MasBet[j][i] = 0;
		}
		double Bet = 0;
		TXMLNode* p = slots->GetNode(i);
		for( j = 0; j < p->NodesCount(); j++ )
		{
			Slots2MasWin[j][i] = p->GetNode(j)->VarValue("win").ToInt()*(i+1);
			Slots2MasBet[j][i] = p->GetNode(j)->VarValue("bet").ToInt(); // *Slots2MasWin[j][i]
			Slots2MasP[j][i] = p->GetNode(j)->VarValue("p").ToInt();
			Bet += Slots2MasBet[j][i];
		}
		for( j = 0; j < p->NodesCount(); j++ )
			Slots2MasBet[j][i] /= Bet;
	}
*/
	double Bet = 0;
	int j;
	for( j = 0; j < cSlots2MASCOUNT; j++ )
		Bet += Slots2MasBet[j];
	for( j = 0; j < cSlots2MASCOUNT; j++ )
		Slots2MasBet[j] /= Bet;
}



