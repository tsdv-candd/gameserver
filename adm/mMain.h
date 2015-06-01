#ifndef mMainH
#define mMainH

#include "mCommon.h"
#include "socket/m_socket.h"

class TMain: public TApp
{
private:
	TStr UserCashToXML(const TStr& Name);
	TStr UserCashToXML(const TStr& Name1, const TStr& Name2);

	bool TRCheck(const TStr& ID);

	void CheckProxy(const TStr& IP);
	TStr GUID;
	unsigned int fGameID;
	unsigned int fModeID;
	TStr fSearch;
	TStr fUsersStatus;   
	TStr fSort;
	TStr fDateBegin;
	TStr fDateEnd;
	TStr fSessionsID;
	TStr fUsersID;
	unsigned int fRecords;
	unsigned int fPage;
	unsigned int fEditID;
	unsigned int fDelID;

	void OnPages(const TStr& sCount);
	void OnPageBegin();
	void OnPageDefault();
	void OnPageEnd();
	TStr PageValue(const TStr& Name);
	TStr OnPageHead(const TStr&);
	TStr OnPageSort(const TStr&);

	struct TPageWhere {
		TStr Search0;
		TStr Value[10];
		TStr Search[10];
		int  Count;
		TPageWhere(): Count(0) {}
	};

	TPageWhere OnPageWhereMulti(const TStr& FormMenu);
	TStr       OnPageWhere(const TStr& FormMenu);

	TSocketClient fSock;

	bool Connect();
	TStr Read();
	void Send(const TStr &);

	TStr ScriptPage(const TStr&);

	TStr OnStreamUserIP(const TStr& UserID);
	TStr OnStreamUserGame(const TStr& UserID, const TStr& ModeID, const TStr& Date = "");
	TStr OnStreamUserPayAdm(const TStr& UserID);
	TStr OnStreamUserPay(const TStr& UserID, const TStr& ModeID, const TStr& Limit);
	TStr OnStreamUserPaySystem(const TStr& UserID);
	TStr OnStreamUserPayStatus(const TStr& UserID);
	TStr OnStreamUserOrderPay(const TStr& UserID);
	TStr OnStreamUserOrderMethod(const TStr& UserID, const TStr& OrderID);

	TStr OnStreamUserGamePay(const TStr& UserID);
	TStr OnStreamUserGamePay(const TStr& UserID, const TStr& DateStart, const TStr& TimeStart, const TStr& DateEnd, const TStr& TimeEnd);

	TStr ScriptAdmOnlineList();

	TStr ScriptAdmMethodEdit();
	TStr ScriptAdmMethodList();

	TStr ScriptChat();

	TStr ScriptAdmUserPaymentList();

	TStr ScriptServer();
	TStr ScriptView();

	TStr ScriptAdmPromoList();
	TStr ScriptAdmPromoEdit();
	TStr ScriptAdmPromoUsersList();

	TStr ScriptAdmClubCardList();
	TStr ScriptAdmClubCardEdit();

	TStr ScriptAdmCasinoList();
	TStr ScriptAdmDateList();
	TStr ScriptAdmGCLList();
	TStr ScriptAdmPeriod(const TStr& res);
	TStr ScriptAdmPeriodList();
	TStr ScriptAdmUrlList();
	TStr ScriptAdmIDList();

	TStr ScriptAdmTRList();
	TStr ScriptAdmTREdit();
	TStr ScriptAdmTRCreate();
	TStr ScriptAdmTRCardCreate();

	TStr ScriptAdmPlayerList();
	TStr ScriptAdmUserActionList();
	TStr ScriptAdmUserActionEdit();
	TStr ScriptAdmUserList();
	TStr ScriptAdmUserEdit();
	TStr ScriptAdmSessionList();

	TStr ScriptAdmGCLSearchList();
	TStr ScriptAdmUserSearchList();

	TStr ScriptAdmPayList();
	TStr ScriptAdmPayLogList();

	TStr ScriptAdmLogList();
	TStr ScriptAdmLogView();
	TStr ScriptAdmLogSessionList();
	TStr ScriptAdmGameList();
	
	TStr ScriptAdmMoneyList();
	TStr ScriptAdmMoneyEdit();
	TStr ScriptAdmMoneyLimitList();
	TStr ScriptAdmMoneyLimitEdit();

	TStr ScriptAdmNewsList();
	TStr ScriptAdmNewsEdit();

	TStr ScriptAdmSubscribeList();
	TStr ScriptAdmSubscribeEdit();

	TStr ScriptAdmSetupList();
	TStr ScriptAdmSetupEdit();

	TStr ScriptAdmPopupList();
	TStr ScriptAdmPopupEdit();

	virtual void Init();
public:
	TMain(m_classes::TMySqlConnect *database, TSession *p = NULL);
	virtual void Script(TStr *Result, TStr *Header, m_ssi::TStringList *ValueList);
	TStr Script(const TStr& Name);
};

#endif
