#include "mGameVideopokerJokerpoker.h"
#include "server/mUser.h"
#include "server/mServer.h"
#include "utils/m_utils.h"

TJokerpokerProducer jokerpoker;

unsigned int TJokerpokerSession::fWin[11] = {
	750,
	200,
	100,
	50,
	17,
	7,
	5,
	3,
	2,
	1,
	1
};

unsigned int JokerpokerCardsCombination(TCardsCombination& Combination, bool Joker)
{
	switch( Combination.Combination() )
	{
#ifdef CASINO_ROYAL
		case CARD_PAIR_TEN: {
			for( int i = 0; i < 5; i++ )
			{
				if( Combination.Hold(i) ) {
					if( Combination.Cards(i) >= 13 )
						return 11;
					else
						break;
				}
			}
			break;
		}
#endif
		case CARD_TWO_PAIR: return 10;
		case CARD_THREE_OF_A_KIND: return 9;
		case CARD_STRAIGHT: return 8;
		case CARD_FLUSH: return 7;
		case CARD_FULL_HOUSE: return 6;
		case CARD_FOUR_OF_A_KIND: return 5;
		case CARD_STRAIGHT_FLUSH: return 4;
		case CARD_ROYAL_FLUSH:
			if( Joker )
				return 3;
			else
				return 1;
		case CARD_FIVE_OF_A_KIND: return 2;
	}
	return 0;
}

unsigned int CardsToWinID(unsigned int *fCards, bool *Hold)
{
	int Joker = -1;
	unsigned int i;
	for( i = 0; i < 5; i++ )
		if( fCards[i] == 53 ) {
			Joker = i;
			break;
		}
	unsigned int ID = 0;
	unsigned int j;
	for( j = 0; j < 5; j++ )
		Hold[j] = false;
	if( Joker == -1 ) {
		TCardsCombination cc(fCards);
		for( j = 0; j < 5; j++ ) {
			Hold[j] = cc.Hold(j);
			if( cc.Combination() == CARD_ACE_KING || cc.Combination() == CARD_NOT )
				Hold[j] = false;
		}
		ID = JokerpokerCardsCombination(cc,false);
	} else {
		bool newHold[5];
		for( i = 1; i < 53; i++ ) {
			fCards[Joker] = i;
			TCardsCombination cc(fCards);
			for( j = 0; j < 5; j++ ) {
				newHold[j] = cc.Hold(j);
				if( cc.Combination() == CARD_ACE_KING || cc.Combination() == CARD_NOT )
					newHold[j] = false;
			}
			unsigned int newID = JokerpokerCardsCombination(cc,true);
			if( newID > 0 )
				if( ID == 0 || ID > 0 && newID < ID ) {
					ID = newID;
					for( unsigned int j = 0; j < 5; j++ )
						Hold[j] = newHold[j];
				}
		}
		fCards[Joker] = 53;
		Hold[Joker] = true;
	}
	return ID;
}

void TJokerpokerSession::OnBet(TRequest *r)
{
	r->User()->CashRollback();
	TServerSSI rs;
	TStr s;
	TXMLNode* bet = r->xml()->FindNode("bet");
	if( bet != NULL ) {
		fCashBet = bet->VarValue("cash").ToIntDef(0);
		Coin = bet->VarValue("coin").ToIntDef(0);
		if( fCashBet > 0 && Coin > 0 ) {
			if( r->User()->Status() == tGIFT )
				fCashBet = 1;
			BetTable = BetCashToTable(fCashBet);
			if( !r->User()->CashBetCommit(cGameJokerpoker,BetTable,fCashBet *= Coin) ) {
				fCashBet = 0;
				return;
			}
			unsigned int WinID = 0;
			bool Hold[5];
			do {
				getRnd(fCards,5,1,53);
//				if( WinID == 0 )
//					MasLoad(fCards,"cards_jokerpoker.txt");
				WinID = CardsToWinID(fCards,Hold);
				fCashWin = 0;
				if( WinID > 0 )
					if( WinID == 1 && Coin == 5 ) {
						fCashWin = 100000000;
					} else
						fCashWin = fCashBet * fWin[WinID-1];
		} while( !r->User()->CashWinUpdate(cGameJokerpoker,BetTable,fCashWin) );
			for( unsigned int i = 0; i < 5; i++ )
			{
				rs.ssi()->SSIValue("ID",i+1);
				rs.ssi()->SSIValue("TYPE",fCards[i]);
				rs.ssi()->SSIBlock("&isHold",Hold[i]);
				s += rs.ssi()->SSIRes("jokerpoker_card");
			}
			rs.ssi()->SSIValue("CASHBET",fCashBet);
			rs.ssi()->SSIValue("ID",WinID);
			rs.ssi()->SSIValue("STREAM",s);
		s = rs.ssi()->SSIRes("jokerpoker_bet");
			r->Add(s,"game");
			fStatus = tGame;
		}
	}
}

void TJokerpokerSession::OnGame(TRequest *r)
{
	if( fCashBet == 0 )
		return;
	TServerSSI rs;
	fStatus = tBet;
	TStr s;
	bool fCardsR[5];
	unsigned int i;
	for( i = 0; i < 5; i++ )
		fCardsR[i] = false;
	for( i = 0; i < r->xml()->NodesCount(); i++ )
	{
		TXMLNode* p = r->xml()->GetNode(i);
		if( p->GetName() == "card" ) {
			int id = p->VarValue("id").ToInt();
			if( id > 0 && id < 6 )
				fCardsR[id-1] = true;
		}
	}
	unsigned int WinID = 0;
	unsigned int fCardsNew[5];
//	int Jackpot = 0;
	bool Hold[5];
	do {
		getRnd(fCardsNew,5,1,53,fCards,5);
		for( i = 0; i < 5; i++ )
			if( !fCardsR[i] )
				fCardsNew[i] = fCards[i];
		WinID = CardsToWinID(fCardsNew,Hold);
		fCashWin = 0;
		if( WinID > 0 ) {
			if( (WinID == 1) && Coin == 5 )
				fCashWin = (int)(r->User()->JackpotID(cJackpotVideopoker) * (fCashBet / 5.0 / 500.0));
			else
				fCashWin = fCashBet * fWin[WinID-1];
/*
			if( (WinID == 1) && Coin == 5 ) {
				fCashWin = Jackpot = r->User()->Jackpot(cGameJokerpoker) * (fCashBet / 5.0 / 500.0);
				r->User()->JackpotUpdate(cGameJokerpoker,-Jackpot);
				break;
			} else
				fCashWin = fCashBet * fWin[WinID-1];
*/
		}
	} while( !r->User()->CashWinUpdate(cGameJokerpoker,BetTable,fCashWin) );

	TStr Log;
	fCardsChanceCount = 0;
	for( i = 0; i < 5; i++ )
	{
		unsigned int c = fCards[i];
		Log += TStr("<card id=\"")+i+"\" type=\""+c+"\" />";
		if( fCardsR[i] ) {
			rs.ssi()->SSIValue("ID",i+1);
			rs.ssi()->SSIValue("TYPE",c = fCardsNew[i]);
			fCardsChance[fCardsChanceCount++] = c;
			s += rs.ssi()->SSIRes("jokerpoker_card");
			Log += TStr("<hold id=\"")+i+"\" type=\""+c+"\" />";
		}
		// !!! WinComb
		if( WinID > 0 && Hold[i] )
			s += "<win id=\""+TStr(i+1)+"\" />";
		// !!!
		fCardsChance[fCardsChanceCount++] = c;
	}
/*
	if( Jackpot > 0 )
		r->User()->CashJackpotCommit(Log,cGameJokerpoker,Jackpot);
	else
*/
	r->User()->CashWinCommit(Log);
	r->User()->CashCommit();
	if( fCashWin > 0 ) {
		fStatus = tChance;
		r->AddStatus("chance");
	} else {
		fStatus = tBet;
		r->AddStatus("ok");
	}
	rs.ssi()->SSIValue("CASHWIN",fCashWin);
	rs.ssi()->SSIValue("ID",WinID);
	rs.ssi()->SSIValue("STREAM",s);
	getRnd(fCardsChance+fCardsChanceCount,1,1,52,fCardsChance,fCardsChanceCount);
	rs.ssi()->SSIValue("CHANCETYPE",fCardsChance[fCardsChanceCount++]/*=getRnd(52)+1*/);
	r->AddReturn(rs.ssi()->SSIRes("jokerpoker_game"));

	r->User()->CashBetAdd(BetTable,fCashWin);
}

void TJokerpokerSession::OnCash(TRequest *r)
{
	r->User()->CashRollback();
	if( fStatus != tChance )
		return;
	fStatus = tBet;
	fCashWin = 0;
	r->AddStatus("ok");
}

void TJokerpokerSession::OnChance(TRequest *r)
{
	if( fStatus != tChance )
		return;
	TServerSSI rs;
	unsigned int ID = r->xml()->TXMLNodes::VarValue("action/id").ToIntDef(1);
	if( ID == 0 || ID > 6 )
		return;
	if( r->xml()->TXMLNodes::VarValue("action/stat") == "save" ) {
		r->User()->CashBetAdd(BetTable, - fCashWin / 2);
		if( fCashWin == 0 )
			fCashWin = 1;
		fCashWin -= fCashWin / 2;
	} else
	if( r->xml()->TXMLNodes::VarValue("action/stat") == "bet" ) {
		if( r->User()->CashBetAdd(BetTable,fCashWin) )
			fCashWin += fCashWin;
	}
	r->User()->CashBetCommit(cGameJokerpokerChance);

	rs.ssi()->SSIValue("CASHBET",fCashWin);
	unsigned int CardChance = 0;
	int CashBet = fCashWin;
	int i = 0;
	do {
		getRnd(&CardChance,1,1,52,fCardsChance,fCardsChanceCount);

		if( CardColor(CardChance) == ID )
			fCashWin = CashBet * 4;
		else {
			bool f = CardColor(CardChance) == 3 || CardColor(CardChance) == 4;
			if( f && ID == 5 || !f && ID == 6 )
				fCashWin = CashBet * 2;
			else
				fCashWin = 0;
		}
	} while( !r->User()->CashWinUpdate(cGameJokerpokerChance,BetTable,fCashWin,i++ > 100) );
	fCardsChance[fCardsChanceCount++] = CardChance;
	rs.ssi()->SSIValue("CHANCETYPE",CardChance);
	r->User()->CashWinCommit(TStr("<chance type=\"")+CardChance+"\" id=\""+ID+"\" />");
	r->User()->CashCommit();
	if( fCashWin == 0 )
		r->AddStatus("ok");
	else
		r->AddStatus("chance");
	rs.ssi()->SSIValue("CASHWIN",fCashWin);
	r->AddReturn(rs.ssi()->SSIRes("jokerpoker_chance"));

	r->User()->CashBetAdd(BetTable,fCashWin);
}

void TJokerpokerSession::Run(TRequest *r)
{
	ISession::Run(r);
	bool f = false;
	if( !r->isStatus() ) {
		if( r->cm() == "bet" )  { f = true; OnBet(r); } else
		if( r->cm() == "game" ) { f = true; OnGame(r); } else
		if( r->cm() == "cash" ) { f = true; OnCash(r); } else
		if( r->cm() == "chance" ) { f = true; OnChance(r); }
	}
	TServerSSI rs;
	if( f ) {
		r->User()->ToSSI(rs.ssi());
		r->AddReturn(rs.ssi()->SSIRes("server_user"));
	}
	int Jackpot = r->User()->JackpotID(cJackpotVideopoker);
	rs.ssi()->SSIValue("CASH",Jackpot);
	r->AddReturn(rs.ssi()->SSIRes("server_jackpot"));
}

TJokerpokerProducer::TJokerpokerProducer()
: ISessionProducer(cGameJokerpoker,cJokerpokerName,cJokerpokerSize)
{}


void TJokerpokerSession::OnStart(TUser* User,const int i,const bool f)
{
	fCashWin = fCashBet = 0;
	TServerSSI rs;
	ISession::OnStart(User,i,f);
}

void TJokerpokerSession::OnLeave(TUser* User)
{
	ISession::OnLeave(User);
}
