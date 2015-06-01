#ifndef mPayH
#define mPayH

#include "m_string.h"

enum EPaySystem {
	sysWm = 1,
	sysCard = 2,
	sysEport = 3,
	sysYandex = 5,
	sysWire = 4,
	sysPromo = 8,
	sysDeposit = 20,
	sysSignup = 21,
	sysAdm = 50,
	sysParimatch = 55,
	sysCreditpilot = 56,
	sysRapida = 57,
	sysEgold = 58,
	sysMoneyBookers = 59,
	sysRupay = 60,
	sysLotto = 51,
	sysMnogo = 61,
	sysEvoCash = 64,
	sysEcoCard = 65,
	sysFaktura = 66,
	sysCardSecure = 67,
	sysFUN = 100
};

enum EPayStatus {
	payPROCESSING = 1,
	payADM = 2,
	payWAIT = 3,
	payUNDEFINED = 4,

	payERROR = 10,
	payOK = 11,
	payCANCEL = 12,
	payExecuted = 13,
	payOK_CB = 20,
	payOK_RF = 21,
	payRF = 30,
	payCB = 31
};

#endif
