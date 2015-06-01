#include "mGameVideopokerTenorbetter.h"
#include "server/mUser.h"
#include "server/mServer.h"
#include "utils/m_utils.h"

TVideopokerProducer Videopoker;

unsigned int TVideopokerSession::fWin[9] = {
	250,
	50,
	25,
	9,
	6,
	4,
	3,
	2,
	1
};

unsigned int VideopokerCardsCombination(FCardsCombination Combination)
{
	switch( Combination )
	{
		case CARD_PAIR_TEN: return 9;
		case CARD_TWO_PAIR: return 8;
		case CARD_THREE_OF_A_KIND: return 7;
		case CARD_STRAIGHT: return 6;
		case CARD_FLUSH: return 5;
		case CARD_FULL_HOUSE: return 4;
		case CARD_FOUR_OF_A_KIND: return 3;
		case CARD_STRAIGHT_FLUSH: return 2;
		case CARD_ROYAL_FLUSH: return 1;
	}
	return 0;
}

void TVideopokerSession::OnBet(TRequest *r)
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
			if( !r->User()->CashBetCommit(cGameVideopoker,BetTable,fCashBet *= Coin) ) {
				fCashBet = 0;
				return;
			}
			unsigned int WinID = 0;
			bool Hold[5];
			unsigned int i;
			do {
				getRnd(fCards,5,1,52);
//				if( WinID == 0 )
//					MasLoad(fCards,"cards_videopoker.txt");
				TCardsCombination cc(fCards);
				WinID = VideopokerCardsCombination(cc.Combination());
				for( i = 0; i < 5; i++ ) {
					Hold[i] = cc.Hold(i);
					if( cc.Combination() == CARD_ACE_KING || cc.Combination() == CARD_NOT )
						Hold[i] = false;
				}
				fCashWin = 0;
				if( WinID > 0 )
					if( WinID == 1 && Coin == 5 ) {
						fCashWin = 100000000;
					} else
						fCashWin = fCashBet * fWin[WinID-1];
			} while( !r->User()->CashWinUpdate(cGameVideopoker,BetTable,fCashWin) );
			for( i = 0; i < 5; i++ )
			{
				rs.ssi()->SSIValue("ID",i+1);
				rs.ssi()->SSIValue("TYPE",fCards[i]);
				rs.ssi()->SSIBlock("&isHold",Hold[i]);
				s += rs.ssi()->SSIRes("videopoker_card");
			}
			rs.ssi()->SSIValue("CASHBET",fCashBet);
			rs.ssi()->SSIValue("ID",WinID);
			rs.ssi()->SSIValue("STREAM",s);
			s = rs.ssi()->SSIRes("videopoker_bet");
//      r->UserToSSI(rs.ssi());
			r->Add(s,"game");
			fStatus = tGame;
		}
	}
}

void TVideopokerSession::OnGame(TRequest *r)
{
	if( fStatus != tGame )
		return;
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
		getRnd(fCardsNew,5,1,52,fCards,5);
		for( i = 0; i < 5; i++ )
			if( !fCardsR[i] )
				fCardsNew[i] = fCards[i];
		TCardsCombination cc(fCardsNew);
		WinID = VideopokerCardsCombination(cc.Combination());
		fCashWin = 0;
		if( WinID > 0 ) {
			if( WinID == 1 && Coin == 5 )
				fCashWin = (int)(r->User()->JackpotID(cJackpotVideopoker) * (fCashBet / 5.0 / 500.0));
			else
				fCashWin = fCashBet * fWin[WinID-1];
/*
			Защита от джекпота
			if( WinID == 1 && Coin == 5 ) {
				fCashWin = Jackpot = r->User()->Jackpot(cGameVideopoker) * (fCashBet / 5.0 / 500.0);
				r->User()->JackpotUpdate(cGameVideopoker,-Jackpot);
				break;
			} else
				fCashWin = fCashBet * fWin[WinID-1];
*/
			for( i = 0; i < 5; i++ )
				Hold[i] = cc.Hold(i);
		}
	} while( !r->User()->CashWinUpdate(cGameVideopoker,BetTable,fCashWin) );

	TStr Log;
	fCardsChanceCount = 0;
	for( i = 0; i < 5; i++ )
	{
		unsigned int c = fCards[i];
		Log += TStr("<card id=\"")+i+"\" type=\""+c+"\" />";
		if( fCardsR[i] ) {
			rs.ssi()->SSIValue("ID",i+1);
			fCardsChance[fCardsChanceCount++] = c;
			rs.ssi()->SSIValue("TYPE",c = fCardsNew[i]);
			s += rs.ssi()->SSIRes("videopoker_card");
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
		r->User()->CashJackpotCommit(Log,cGameVideopoker,Jackpot);
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
	r->AddReturn(rs.ssi()->SSIRes("videopoker_game"));

	r->User()->CashBetAdd(BetTable,fCashWin);
}

void TVideopokerSession::OnCash(TRequest *r)
{
	r->User()->CashRollback();
	if( fStatus != tChance )
		return;
	fStatus = tBet;
	fCashWin = 0;
	r->AddStatus("ok");
}

void TVideopokerSession::OnChance(TRequest *r)
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
	r->User()->CashBetCommit(cGameVideopokerChance);

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
	} while( !r->User()->CashWinUpdate(cGameVideopokerChance,BetTable,fCashWin,i++ > 100) );
	fCardsChance[fCardsChanceCount++] = CardChance;
	rs.ssi()->SSIValue("CHANCETYPE",CardChance);
	r->User()->CashWinCommit(TStr("<chance type=\"")+CardChance+"\" id=\""+ID+"\" />");
	r->User()->CashCommit();
	if( fCashWin == 0 )
		r->AddStatus("ok");
	else
		r->AddStatus("chance");
	rs.ssi()->SSIValue("CASHWIN",fCashWin);
	r->AddReturn(rs.ssi()->SSIRes("videopoker_chance"));

	r->User()->CashBetAdd(BetTable,fCashWin);
}

void TVideopokerSession::Run(TRequest *r)
{
	ISession::Run(r);
	bool f = false;
	if( !r->isStatus() ) {
		if( r->cm() == "bet" )  { f = true; OnBet(r);  }else
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

TVideopokerProducer::TVideopokerProducer()
: ISessionProducer(cGameVideopoker,cVideopokerName,cVideopokerSize)
{
}


void TVideopokerSession::OnStart(TUser* User,const int i,const bool f)
{
	fCashWin = fCashBet = 0;
	TServerSSI rs;
	ISession::OnStart(User,i,f);
}

void TVideopokerSession::OnLeave(TUser* User)
{
	ISession::OnLeave(User);
}
