#include <stdio.h>
#include "mMoney.h"
#include "mServer.h"

TStr AmountToStr(const TStr& Amount)
{
	int i = Amount.ToInt();
	TStr AMOUNT(i/100);
	int c = i%100;
	if( c > 0 ) {
		AMOUNT += ".";
		if( c < 10 )
			AMOUNT += "0"+TStr(c);
		else
			AMOUNT += TStr(c);
	}
	return AMOUNT;
}

void TPay::Run()
{
	TStr Command;
	TStr Code;

	TSocketClient Sock;
	if( Sock.Connect(fIP.c_str(),fPort.ToInt()) ) {
		TStr URL("GET ");
		if( fURL.isEmpty() || fURL[0] != '/' )
			URL += '/';
		URL += fURL+"?"+"ClientID=1&"+fRequest.ToURL()+" HTTP/1.0\r\n";

		printf("\n\n%s\n\n",fRequest.ToURL().c_str());

		if( !fHost.isEmpty() )
			URL += "Host: "+fHost+"\r\n";
		URL += "\r\n";
		Sock.SendBuffer(URL.c_str(),URL.Length());
		char  r[1024];
		for( int jj = 0; jj < 1024; jj++ )
			r[jj] = '\0';
		int ReadLen = Sock.ReadBuffer(r,1024,cPAYTIME);
		int iHTTP = 0;
		while( iHTTP < ReadLen-3 )
		{
			if(
				r[iHTTP] == '\r' &&
				r[iHTTP+1] == '\n' &&
				r[iHTTP+2] == '\r' &&
				r[iHTTP+3] == '\n' )
				break;
			iHTTP++;
		}
		int i = (iHTTP += 4);
/*
		while( iHTTP < 1024-2 )
		{
			if(
				r[iHTTP] == '\r' &&
				r[iHTTP+1] == '\n' )
				break;
			iHTTP++;
		}
*/
		TStr sPay(r+i,ReadLen-i);
		if( !sPay.isEmpty() ) {
			int i = sPay.Pos(":");
			if( i > -1 ) {
				Command = sPay.CopyBefore(i);
				TStr s(sPay.CopyAfter(i));
				int i = 0;
				while( i < s.Length() ) {
					if( s[i] == 0 || s[i] == '\0' || s[i] == ':' || s[i] == '\r' || s[i] == '\n' )
						break;
					Code += s[i++];
				}
			}
		}
	}
	if( Command.isEmpty() )
		Command = "timeout";
	if( !Code.isEmpty() )
		Code = "  code="+Code;
	fServer->Request(
		"<casino command=pay>\n"
		" <pay "
		  "  id="+TStr(fRequest.fOrderID)+
		  "  status="+Command+
		  Code+
		  " />"
		"</casino>\n");
}


EPayCommand StrToPayCommand(const TStr& s)
{
	if( s == "wait" )
		return cmWAIT;
	if( s == "error" )
		return cmERROR;
	if( s == "ok" )
		return cmOK;
	if( s == "out" )
		return cmOUT;
	if( s == "cancel" )
		return cmCANCEL;
	if( s == "BLOCK" )
		return cmBLOCK;
	if( s == "rf" )
		return cmRF;
	if( s == "cb" )
		return cmCB;
	return cmTIMEOUT;
}

