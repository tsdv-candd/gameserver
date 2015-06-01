#include "mCashUtils.h"

TStr CashToStr(const double Cash)
{
	int c1 = (int)Cash/100;
	int c2 = (int)Cash%100;
	if( c2 < 10 )
		return TStr(c1)+".0"+TStr(c2);
	return TStr(c1)+"."+TStr(c2);
}

/*
TStr CashToStr(const TStr& Cash)
{
	return CashToStr(Cash.ToDouble());
}
*/

int StrToCash(const TStr& Cash)
{
	int l = Cash.CopyBefore(".").ToIntDef(0)*100;
	TStr a(Cash.CopyAfter("."));
	int r = a.ToIntDef(0);
	if( a.Length() == 1 )
		r *= 10;
	if( l < 0 || l == 0 && Cash[0] == '-')
		l -= r;
	else
		l += r;
	return l;
}
