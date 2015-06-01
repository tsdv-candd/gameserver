#ifndef mMoneyH
#define mMoneyH

#include "utils/m_utils.h"
#include "socket/m_socket.h"
#include "xml/m_xmlparser.h"
#include "thread/m_thread.h"
#include "convert/mConvert.h"
#include "common/mPay.h"
#include "m_string.h"


const unsigned int cPAYTIME = 60000*10;

TStr AmountToStr(const TStr& Amount);

enum EPayCommand {
	cmBLOCK,
	cmCANCEL,
	cmTIMEOUT,
	cmWAIT,
	cmERROR,
	cmOUT,
	cmOK,
	cmRF,
	cmCB
};

class TServer;


class TCCard: public TThread {
private:
	TServer* fServer;
public:
	TCCard(TServer* Server): fServer(Server) {
	}

	void Run() {
//		fServer->OnCasher();
	}
};


struct TPayTransaction
{
	unsigned int  fSystemID;
	int           fCash;
	int           fCurrency;
	int           fCurrencyCash;

	unsigned int  fOrderID;
	TStr          fOrderValues;

	unsigned int  fUserID;
	TStr          fUserIP;
	TStr          fUserInfo;

	TPayTransaction() : fCurrency(0),fOrderID(0) {}

	TStr ToURL() {
		TStr s("Type=");
		if( fCash > 0 )
			s += "IN";
		else
			s += "OUT";
		s +=
			"&"
			"OrderID="+TStr(fOrderID)+"&"
			"UserID="+TStr(fUserID)+"&"
			"UserIP="+fUserIP+"&";
		if( fCurrency > 0 )
			s += "Amount="+TStr(fCurrencyCash)+"&"+"Currency="+TStr(fCurrency)+"&";
		else
			s += "Amount="+TStr(fCash)+"&";
		TXML xml(fOrderValues.c_str());
		for( unsigned int i = 0; i < xml.NodesCount(); i++ )
			s += xml.GetNode(i)->GetName()+"="+__escape(xml.GetNode(i)->VarValue("value"))+"&";
		return s;
	}
};


class TPay: public TThread {
private:
	TServer* fServer;
	TStr fIP;
	TStr fPort;
	TStr fURL;
	TStr fHost;
	TPayTransaction fRequest;
public:

	TPay(
		TServer* Server,
		const TStr& IP,
		const TStr& Port,
		const TStr& URL,
		const TStr& Host)
	:
		fServer(Server),
		fIP(IP),
		fPort(Port),
		fURL(URL),
		fHost(Host)
	{ }

	void Create(const TPayTransaction& Request)
	{
		fRequest = Request;
		TThread::Create(true);
	}

	void ToPay(TSocket& Sock);
	void Run();
};

EPayCommand StrToPayCommand(const TStr& s);

#endif
