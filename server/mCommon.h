#ifndef mCommonH
#define mCommonH

#include <string.h>
#include "m_string.h"
#include "ssi/mf_ssi.h"

typedef unsigned int Uint;

extern bool fLogPrint;
extern bool fLogFile;
extern bool fLogBD;
extern bool fDebug;

void print_log(const TStr& s1, const TStr& s2 = "");
void print(const TStr& s1, const TStr& s2 = "", const TStr& s3 = "", const TStr& s4 = "", const TStr& s5 = "", const TStr& s6 = "", const TStr& s7 = "", const TStr& s8 = "", const TStr& s9 = "", const TStr& s10 = "", const TStr& s11 = "");
void printDump();

TStr CashToUSD(const int Cash);
TStr CashToUSD(const char*);

enum FCardsCombination {
	CARD_FIVE_OF_A_KIND = 0,  // четыре одинаковых карты и джокер
	CARD_ROYAL_FLUSH = 1,     // десять, валет, дама, король, туз одной масти
	CARD_STRAIGHT_FLUSH = 2,  // пять карт одной масти по возрастанию (например, восемь, девять, десять, валет, дама одной масти
	CARD_FOUR_OF_A_KIND = 3,  // четыре одинаковых карты (например, 4 туза)
	CARD_FULL_HOUSE = 4,      // три одинаковых карты и пара одинаковых карт (например, 3 короля и 2 двойки)
	CARD_FLUSH = 5,           // пять карт одной масти (например, двойка, семерка, дама, король и туз одной масти)
	CARD_STRAIGHT = 6,        // пять карт любых мастей по возрастанию (например, десять, валет, дама, король и туз разных мастей)
	CARD_THREE_OF_A_KIND = 7, // три одинаковых карты (например, 3 дамы)
	CARD_TWO_PAIR = 8,        // две пары одинаковых карт (например, 2 девятки и 2 вальта)
	CARD_PAIR_TEN = 9,        // пара одинаковых карт больших десятки (например, 2 вальта)
	CARD_PAIR = 10,           // пара одинаковых карт
	CARD_ACE_KING = 11,       // король и туз
	CARD_NOT = 12
};

class TCardsCombination
{
protected:

	struct TCard {
		unsigned int index;
		unsigned int id;
	};
	TCard fCardsSort[5];

	unsigned int      fCards[5];
	bool              fHold[5];
	FCardsCombination fCombination;
public:
	TCardsCombination(const unsigned int *Cards = NULL);

	void Init(const unsigned int *Cards);

	FCardsCombination Combination() { return fCombination; }
	bool              Hold(const unsigned int j) { return fHold[j]; }
	unsigned int      isWin(TCardsCombination &, const bool = true);
	unsigned int      Cards(const unsigned int i) { return fCards[i]; }

	unsigned int      CardsSort(const unsigned int i) { return fCardsSort[i].id; }
	unsigned int      CardsSortIndex(const unsigned int i) { return fCardsSort[i].index; }
};

extern unsigned int CardColor(unsigned int CardID);
extern unsigned int CardPrice(unsigned int CardID);
extern unsigned int CardID(unsigned int CardID);

void         CardsDelivery(unsigned int *fCards, const unsigned int fCardsCount);

int          MasLoad(unsigned int *fCards, const char* File);

void         CardsScore(const unsigned int *fCards, const unsigned int fCardsCount, int &r1, int &r2);
unsigned int CardsScore(const unsigned int *fCards, const unsigned int fCardsCount);
TStr         CardsScoreAsStr(const unsigned int *fCards, const unsigned int fCardsCount);

TStr  CardsCombinationAsStr(const FCardsCombination Score);
TStr  CardsCombinationToStr(const FCardsCombination Score);

TStr  CardToStr(const unsigned int ID);

extern TStr CardToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int Card,
	const unsigned int BoxID,
	const unsigned int Cash,
	const TStr& Score,
	const unsigned int CardID);

extern TStr CardToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int Card,
	const unsigned int BoxID = 0);

extern TStr CardsToSSI(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int* Cards,
	const unsigned int CardsCount,
	const unsigned int BoxID = 0);

TStr BoxAction(
	m_ssi::TSSIControl *ssi,
	const TStr& Action,
	const unsigned int BoxID,
	const unsigned int Cash = 0);

TStr BoxScore(
	m_ssi::TSSIControl *ssi,
	const unsigned int BoxID,
	const TStr& Score);

TStr BoxCombination(
	m_ssi::TSSIControl *ssi,
	const unsigned int BoxID,
	const unsigned int* Cards);

unsigned int BetCashToTable(const unsigned int Cash);
unsigned int BetCashToTable(const unsigned int Cash, const unsigned int Win);

#endif

