#ifndef mCasinoH
#define mCasinoH


#include "sql/mMySQL.h"
#include "ssi/mf_ssi.h"


TStr CashToStr(TStr s);
TStr BalanceToStr(int i);
TStr BalanceToStr(const TStr& s);

enum EPeriod {
//	cdateHOUR,
	cdateDAY,
//	cdateDAYOFWEEK,
	cdateMONTH,
	cdateYEAR
};


long OnPeriod(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date);


void OnPeriodItem(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	long                    cDate);


TStr CasinoPeriodList(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date);


TStr GamePeriodList(
	m_ssi::TSSIControl*     ssi,
	m_classes::TMySqlQuery* query,
	const EPeriod           Period,
	const TStr&             Date);


#endif
