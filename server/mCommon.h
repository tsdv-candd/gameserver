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
	CARD_FIVE_OF_A_KIND = 0,  // ������ ���������� ����� � ������
	CARD_ROYAL_FLUSH = 1,     // ������, �����, ����, ������, ��� ����� �����
	CARD_STRAIGHT_FLUSH = 2,  // ���� ���� ����� ����� �� ����������� (��������, ������, ������, ������, �����, ���� ����� �����
	CARD_FOUR_OF_A_KIND = 3,  // ������ ���������� ����� (��������, 4 ����)
	CARD_FULL_HOUSE = 4,      // ��� ���������� ����� � ���� ���������� ���� (��������, 3 ������ � 2 ������)
	CARD_FLUSH = 5,           // ���� ���� ����� ����� (��������, ������, �������, ����, ������ � ��� ����� �����)
	CARD_STRAIGHT = 6,        // ���� ���� ����� ������ �� ����������� (��������, ������, �����, ����, ������ � ��� ������ ������)
	CARD_THREE_OF_A_KIND = 7, // ��� ���������� ����� (��������, 3 ����)
	CARD_TWO_PAIR = 8,        // ��� ���� ���������� ���� (��������, 2 ������� � 2 ������)
	CARD_PAIR_TEN = 9,        // ���� ���������� ���� ������� ������� (��������, 2 ������)
	CARD_PAIR = 10,           // ���� ���������� ����
	CARD_ACE_KING = 11,       // ������ � ���
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

