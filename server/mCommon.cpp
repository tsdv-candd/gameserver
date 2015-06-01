#include "mCommon.h"

#include <stdio.h>
#include "utils/m_datetime.h"
#include "utils/m_utils.h"
#include "file/m_file.h"
#include "thread/m_thread.h"

bool fDebug = false;
bool fLogPrint = false;
bool fLogBD = false;
bool fLogFile = false;


TStr CashToUSD(const int Cash)
{
	TStr s((int)abs(Cash)%100);
	if( s.Length() < 1 )
		s = TStr('0')+s;
	if( s.Length() < 2 )
		s = TStr('0')+s;
	s = TStr(abs(Cash)/100)+"."+s;
	if( Cash < 0 )
		s = "-" + s;
	return s;
}

TStr CashToUSD(const char* str)
{
	return CashToUSD(TStr(str).ToInt());
/*
	TStr s(str);
	if( s.ToInt() < 10 )
		return "0.0"+s;
	if( s.ToInt() < 100 )
		return "0."+s;
	if( s.Copy(s.Length(	-2,2).ToInt() == 0 )
		return s.Copy(0,s.Length()-2);
	return s.Copy(0,s.Length()-2)+"."+s.Copy(s.Length()-2,2);
*/
}


TLock print_log_l;

void print_log(const TStr& s1, const TStr& s2)
{
	if( fLogFile ) {
		TStr s;
		TStr sName(DateTimeToStr(Now(),"server_log_%y_%m_%d.log"));
		if( s1.Last() == '\0' )
			s = s1.CopyBefore(s1.Length()-1);
		else
			s = s1;
		s += s2;
		TLockSection _l(&print_log_l);
		TFile flog;
		try {
			flog.OpenSave(sName.c_str(),true);
			fprintf(flog.stream,"\n\n%s:%s",DateTimeToStr(Now(),"%H:%M:%S").c_str(),s.c_str());
		} catch( ... ) {
		}
	}
}

TLock print_s_l;

void print_s(const TStr& s)
{
	if( fLogPrint ) {
		TLockSection _l(&print_s_l);
		printf("%s\n",s.c_str());
	}
}

void print(const TStr& s1, const TStr& s2, const TStr& s3, const TStr& s4, const TStr& s5, const TStr& s6, const TStr& s7, const TStr& s8, const TStr& s9, const TStr& s10, const TStr& s11)
{
	print_s(s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8 + s9 + s10 + s11);
}


void printDump()
{
}


// TCardsCombination

TCardsCombination::TCardsCombination(const unsigned int *Cards)
{
	if( Cards != NULL )
		Init(Cards);
}

void TCardsCombination::Init(const unsigned int *Cards)
{
	unsigned int Color = CardColor(Cards[0]);
	unsigned int i;
	for( i = 1; i < 5; i++ )
	{
		if( CardColor(Cards[i]) != Color || Cards[i] == 0 ) {
			Color = 0;
			break;
		}
	}
	for( i = 0; i < 5; i++ )
	{
		fHold[i] = false;
		fCardsSort[i].index = i;
		fCards[i] = fCardsSort[i].id = CardID(Cards[i]);
	}
	for( i = 0; i < 4; i++ )
		for( unsigned int j = i+1; j < 5; j++ )
			if( fCardsSort[i].id > fCardsSort[j].id ) {
				TCard c = fCardsSort[i];
				fCardsSort[i] = fCardsSort[j];
				fCardsSort[j] = c;
			}
	// xxxxx_FLUSH
	if( Color > 0 ) {
		for( int j = 0; j < 5; j++ )
			fHold[j] = true;
		// ROYAL_FLUSH
		if(
			fCardsSort[0].id == 10 &&
			fCardsSort[1].id == 11 &&
			fCardsSort[2].id == 12 &&
			fCardsSort[3].id == 13 &&
			fCardsSort[4].id == 14
		) {
			fCombination = CARD_ROYAL_FLUSH;
			return;
		}
		// STRAIGHT_FLUSH
		if( (
			 fCardsSort[0].id > 0 &&
			(fCardsSort[1].id - fCardsSort[0].id) == 1 &&
			(fCardsSort[2].id - fCardsSort[1].id) == 1 &&
			(fCardsSort[3].id - fCardsSort[2].id) == 1 &&
			(fCardsSort[4].id - fCardsSort[3].id) == 1
			) || (
			fCardsSort[0].id == 2 &&
			fCardsSort[1].id == 3 &&
			fCardsSort[2].id == 4 &&
			fCardsSort[3].id == 5 &&
			fCardsSort[4].id == 14)
		) {
			fCombination = CARD_STRAIGHT_FLUSH;
			if( fCardsSort[4].id == 14 && fCardsSort[0].id == 2 )
				fCards[fCardsSort[4].index] = fCardsSort[4].id = 1;
			return;
		}
		// FLUSH
		fCombination = CARD_FLUSH;
		return;
	}
	// FIVE_OF_A_KIND
	if(
			fCardsSort[0].id > 0 &&
			fCardsSort[0].id == fCardsSort[1].id &&
			fCardsSort[0].id == fCardsSort[2].id &&
			fCardsSort[0].id == fCardsSort[3].id &&
			fCardsSort[0].id == fCardsSort[4].id)
	{
		for( int j = 0; j < 5; j++ )
			fHold[fCardsSort[j].index] = true;
		fCombination = CARD_FIVE_OF_A_KIND;
		return;
	}
	// FOUR_OF_A_KIND
	for( i = 0; i < 2; i++ )
		if(
				fCardsSort[i].id > 0 &&
				fCardsSort[i].id == fCardsSort[i+1].id &&
				fCardsSort[i].id == fCardsSort[i+2].id &&
				fCardsSort[i].id == fCardsSort[i+3].id)
		{
			for( int j = 0; j < 4; j++ )
				fHold[fCardsSort[i+j].index] = true;
			fCombination = CARD_FOUR_OF_A_KIND;
			return;
		}
	// FULL_HOUSE
	if(
		fCardsSort[0].id > 0 &&
		( fCardsSort[0].id == fCardsSort[1].id &&
			fCardsSort[0].id == fCardsSort[2].id &&
			fCardsSort[3].id == fCardsSort[4].id
		) ||
		( fCardsSort[0].id == fCardsSort[1].id &&
			fCardsSort[2].id == fCardsSort[3].id &&
			fCardsSort[2].id == fCardsSort[4].id)
	) {
		for( int j = 0; j < 5; j++ )
			fHold[j] = true;
		fCombination = CARD_FULL_HOUSE;
		return;
	}
	// STRAIGHT
	if( (
		fCardsSort[0].id > 0 &&
		fCardsSort[1].id - fCardsSort[0].id == 1 &&
		fCardsSort[2].id - fCardsSort[1].id == 1 &&
		fCardsSort[3].id - fCardsSort[2].id == 1 &&
		fCardsSort[4].id - fCardsSort[3].id == 1
		) || (
		fCardsSort[0].id == 2 &&
		fCardsSort[1].id == 3 &&
		fCardsSort[2].id == 4 &&
		fCardsSort[3].id == 5 &&
		fCardsSort[4].id == 14)
	) {
		for( int j = 0; j < 5; j++ )
			fHold[j] = true;
		fCombination = CARD_STRAIGHT;
		if( fCardsSort[4].id == 14 && fCardsSort[0].id == 2 )
			fCards[fCardsSort[4].index] = fCardsSort[4].id = 1;
		return;
	}
	// THREE_OF_A_KIND
	for( i = 0; i < 3; i++ )
		if(
			fCardsSort[i].id > 0 &&
			fCardsSort[i].id == fCardsSort[i+1].id &&
			fCardsSort[i].id == fCardsSort[i+2].id
		) {
			for( int j = 0; j < 3; j++ )
				fHold[fCardsSort[i+j].index] = true;
			fCombination = CARD_THREE_OF_A_KIND;
			return;
		}
	// PAIR hold
	for( i = 0; i < 4; i++ )
		if( !fHold[fCardsSort[i].index] )
			if( fCardsSort[i].id > 0 && fCardsSort[i].id == fCardsSort[i+1].id )
				fHold[fCardsSort[i].index] = fHold[fCardsSort[i+1].index] = true;
	// TWO_PAIR
	if(
		(fCardsSort[0].id > 0 && fCardsSort[0].id == fCardsSort[1].id && fCardsSort[2].id == fCardsSort[3].id) ||
		(fCardsSort[0].id > 0 && fCardsSort[0].id == fCardsSort[1].id && fCardsSort[3].id == fCardsSort[4].id) ||
		(fCardsSort[1].id > 0 && fCardsSort[1].id == fCardsSort[2].id && fCardsSort[3].id == fCardsSort[4].id)
	) {
		fCombination = CARD_TWO_PAIR;
		return;
	}
	// PAIR
	for( i = 0; i < 4; i++ )
		if( fCardsSort[i].id > 0 && fCardsSort[i].id == fCardsSort[i+1].id ) {
			if( fCardsSort[i].id >= 10 )
				fCombination = CARD_PAIR_TEN;
			else
				fCombination = CARD_PAIR;
			return;
		}
	if( fCardsSort[4].id == 14 && fCardsSort[3].id == 13 ) {
		fHold[fCardsSort[4].index] = fHold[fCardsSort[3].index] = true;
		fCombination = CARD_ACE_KING;
		return;
	}
	// NOT
	fCombination = CARD_NOT;
}


unsigned int TCardsCombination::isWin(TCardsCombination &c, const bool f)
{
	if( this->fCombination < c.fCombination )
		return 2;
	if( this->fCombination > c.fCombination )
		return 0;
	int j = 4;
	for( int i = 4; i >= 0; i-- )
	{
		if( fHold[fCardsSort[i].index] )
		{
			while( j >= 0 && !c.fHold[c.fCardsSort[j].index] )
				j--;
			if( this->fCardsSort[i].id > c.fCardsSort[j].id )
				return 2;
			else
			if( this->fCardsSort[i].id < c.fCardsSort[j].id )
				return 0;
			j--;
		}
	}
	if( f ) {
		j = 4;
		for( int i = 4; i >= 0; i-- )
		{
			if( !fHold[fCardsSort[i].index] )
			{
				while( j >= 0 && c.fHold[c.fCardsSort[j].index] )
					j--;
				if( this->fCardsSort[i].id > c.fCardsSort[j].id )
					return 2;
				else
				if( this->fCardsSort[i].id < c.fCardsSort[j].id )
					return 0;
				j--;
			}
		}
	}
	return 1;
}


unsigned int CardColor(unsigned int CardID)
{
	if( CardID >= 1  && CardID <= 13 ) return 1;
	if( CardID >= 14 && CardID <= 26 ) return 2;
	if( CardID >= 27 && CardID <= 39 ) return 3;
	if( CardID >= 40 && CardID <= 52 ) return 4;
	return 0;
}


unsigned int CardPrice(unsigned int CardID)
{
	switch( CardID )
	{
		case  1: case 14: case 27: case 40: return 2;  // 2
		case  2: case 15: case 28: case 41: return 3;  // 3
		case  3: case 16: case 29: case 42: return 4;  // 4
		case  4: case 17: case 30: case 43: return 5;  // 5
		case  5: case 18: case 31: case 44: return 6;  // 6
		case  6: case 19: case 32: case 45: return 7;  // 7
		case  7: case 20: case 33: case 46: return 8;  // 8
		case  8: case 21: case 34: case 47: return 9;  // 9
		case  9: case 22: case 35: case 48: return 10; // 10
		case 10: case 23: case 36: case 49: return 10; // V
		case 11: case 24: case 37: case 50: return 10; // Q
		case 12: case 25: case 38: case 51: return 10; // K
		case 13: case 26: case 39: case 52: return 11; // A
	}
	return 0;
}


unsigned int CardID(unsigned int CardID)
{
	switch( CardID )
	{
		case  1: case 14: case 27: case 40: return 2;  // 2
		case  2: case 15: case 28: case 41: return 3;  // 3
		case  3: case 16: case 29: case 42: return 4;  // 4
		case  4: case 17: case 30: case 43: return 5;  // 5
		case  5: case 18: case 31: case 44: return 6;  // 6
		case  6: case 19: case 32: case 45: return 7;  // 7
		case  7: case 20: case 33: case 46: return 8;  // 8
		case  8: case 21: case 34: case 47: return 9;  // 9
		case  9: case 22: case 35: case 48: return 10; // 10
		case 10: case 23: case 36: case 49: return 11; // V  (J)
		case 11: case 24: case 37: case 50: return 12; // Q
		case 12: case 25: case 38: case 51: return 13; // K
		case 13: case 26: case 39: case 52: return 14; // A
	}
	return 0;
}


void CardsDelivery(unsigned int *fCards, const unsigned int fCardsCount)
{
	for( unsigned int i = 0; i < fCardsCount; i++ )
		fCards[i] = 0;
	unsigned int k = fCardsCount / 52;
	unsigned int l = 52;
	if( fCardsCount == 53 )
		l = 53;
	while( k-- > 0 )
		for( unsigned int i = 0; i < l; i++ )
		{
			unsigned int c = getRnd(fCardsCount);
			while( fCards[c] != 0 )
				if( ++c == fCardsCount )
					c = 0;
			fCards[c] = i+1;
		}
}


int MasLoad(unsigned int *fMas, const char* File)
{
	try {
		TFile f(File,false);
		int i = 0;
		TStr str(Read(f.stream));
		while( !str.isEmpty() )
		{
			fMas[i++] = str.ToIntDef(1);
			str = Read(f.stream);
		}
		return i;
	} catch (...) {
	}
	return 0;
}


TStr CardToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int Card,
	const unsigned int ID)
{
	return CardToSSI(ssi,Action,Card,ID,0,"",0);
}


TStr CardToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int Card,
	const unsigned int ID,
	const unsigned int Cash,
	const TStr& Score,
	const unsigned int CardID)
{
	ssi->SSIValue("BOXID",ID);
	ssi->SSIValue("ACTION",Action);
	ssi->SSIValue("CARD",Card);
	// Cash
	if( Cash > 0 )
		ssi->SSIValue("CASH",Cash);
	ssi->SSIBlock("&isCash",Cash > 0);
	// Score
	if( !Score.isEmpty() )
		ssi->SSIValue("SCORE",Score);
	ssi->SSIBlock("&isScore",!Score.isEmpty());
	// CardID
	if( CardID > 0 )
		ssi->SSIValue("CARDID",CardID);
	ssi->SSIBlock("&isCardID",CardID > 0);
	return ssi->SSIRes("cards_card");
}


TStr CardsToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int* Cards,
	const unsigned int CardsCount,
	const unsigned int ID)
{
	ssi->SSIValue("BOXID",ID);
	ssi->SSIValue("ACTION",Action);
	ssi->SSIBlock("&isCash",false);
	ssi->SSIBlock("&isScore",false);
	ssi->SSIBlock("&isCardID",false);
	TStr s;
	for( unsigned int i = 0; i < CardsCount; i++ )
	{
		if( Cards[i] == 0 )
			break;
		ssi->SSIValue("CARD",Cards[i]);
		s += ssi->SSIRes("cards_card");
	}
	return s;
}


void CardsScore(
		const unsigned int *fCards,
		const unsigned int fCardsCount,
		int &r1, int &r2)
{
	r1 = 0;
	r2 = -1;
	for( unsigned int i = 0; i < fCardsCount; i++ )
	{
			unsigned int s = CardPrice(fCards[i]);
			if( s == 11 ) {
				s = 1;
				if( r2 == -1 )
					r2 = r1 + 10;
			}
			r1 += s;
			if( r2 > -1 )
				r2 += s;
	}
}


unsigned int CardsScore(const unsigned int *fCards, const unsigned int fCardsCount)
{
		int r1,r2;
		CardsScore(fCards,fCardsCount,r1,r2);
		if( r2 > 0 && r2 <= 21 )
			return r2;
		return r1;
}


TStr CardsScoreAsStr(const unsigned int *fCards, const unsigned int fCardsCount)
{
	int r1,r2;
	CardsScore(fCards,fCardsCount,r1,r2);
	TStr s(r1);
	if( r2 > -1 && r2 <= 21 )
		s += TStr(" / ") + r2;
	return s;
}


TStr CardsCombinationAsStr(const FCardsCombination Score)
{
	switch( Score )
	{
		case CARD_FIVE_OF_A_KIND:  return "FIVE OF A KIND";
		case CARD_ROYAL_FLUSH:  return "ROYAL FLUSH";
		case CARD_STRAIGHT_FLUSH:  return "STRAIGHT FLUSH";
		case CARD_FOUR_OF_A_KIND:  return "FOUR OF A KIND";
		case CARD_FULL_HOUSE:  return "FULL HOUSE";
		case CARD_FLUSH:  return "FLUSH";
		case CARD_STRAIGHT:  return "STRAIGHT";
		case CARD_THREE_OF_A_KIND:  return "THREE OF A KIND";
		case CARD_TWO_PAIR:  return "TWO PAIR";
		case CARD_PAIR_TEN:
		case CARD_PAIR:  return "PAIR";
		case CARD_ACE_KING: return "ACE KING";
	}
	return "NO HAND";
}

TStr CardsCombinationToStr(const FCardsCombination Score)
{
	switch( Score )
	{
		case CARD_FIVE_OF_A_KIND:   return "FIVE OF A KIND ";
		case CARD_ROYAL_FLUSH:      return "ROYAL FLUSH    ";
		case CARD_STRAIGHT_FLUSH:   return "STRAIGHT FLUSH ";
		case CARD_FOUR_OF_A_KIND:   return "FOUR OF A KIND ";
		case CARD_FULL_HOUSE:       return "FULL HOUSE     ";
		case CARD_FLUSH:            return "FLUSH          ";
		case CARD_STRAIGHT:         return "STRAIGHT       ";
		case CARD_THREE_OF_A_KIND:  return "THREE OF A KIND";
		case CARD_TWO_PAIR:         return "TWO PAIR       ";
		case CARD_PAIR_TEN:         return "PAIR           ";
		case CARD_PAIR:             return "PAIR           ";
		case CARD_ACE_KING:         return "ACE KING       ";
	}
	return "NO HAND        ";
}

TStr CardToStr(const unsigned int ID)
{
	switch( ID )
	{
		case 14: return "T ";
		case 13: return "K ";
		case 12: return "Q ";
		case 11: return "J ";
		case 10: return "10";
	}
	return "0" + TStr(ID);
}



TStr BoxAction(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int BoxID,
	const unsigned int Cash)
{
	ssi->SSIValue("BOXID",BoxID);
	ssi->SSIValue("ACTION",Action);
	ssi->SSIBlock("&isCash",Cash > 0);
	if( Cash > 0 )
		ssi->SSIValue("CASH",Cash);
	return ssi->SSIRes("cards_action");
}


TStr BoxScore(
	m_ssi::TSSIControl *ssi,
	const unsigned int BoxID,
	const TStr& Score)
{
	ssi->SSIValue("BOXID",BoxID);
	ssi->SSIValue("SCORE",Score);
	return ssi->SSIRes("cards_action_score");
}

TStr BoxCombination(
	m_ssi::TSSIControl *ssi,
	const unsigned int BoxID,
	const unsigned int* Cards)
{
	TCardsCombination cc(Cards);
	TStr s;
	s = CardsCombinationAsStr(cc.Combination());
	if( !s.isEmpty() ) {
		ssi->SSIValue("BOXID",BoxID);
		ssi->SSIValue("SCORE",s);
		return ssi->SSIRes("cards_action_score");
	}
	return "";
}

unsigned int BetCashToTable(const unsigned int Cash)
{
	if( Cash < 25 )
		return 10;
	if( Cash < 50 )
		return 30;
	if( Cash < 100 )
		return 50;
	if( Cash < 500 )
		return 70;
	return 90;
}

unsigned int BetCashToTable(const unsigned int Cash, const unsigned int Win)
{
	return BetCashToTable(Cash)+Win;
}
