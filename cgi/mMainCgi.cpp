#include <stdio.h>
#include "cgi/mCgi.h"
#include "file/m_file.h"
#include "socket/m_socket.h"
#include "xml/m_xmlparser.h"
#include "m_string.h"

int main(void)
{
	TCGI cgi;

	cgi.init();

	TSocketClient client;
	client.Connect("127.0.0.1",1024);
	client.Send(cgi.getPost()+"<user ip="+cgi.getEnvironment("REMOTE_ADDR")+" />");
	TStr s(client.Read(60000));
	// check last '\0' or '>'
	TStr Header;
	if( s.isEmpty() )
		s = "<server status=\"error\" />";
	else {
		try {
			TXML xml(s.c_str());
			TStr _Cookie = xml.VarValue("server/cookie/value");
			if( !_Cookie.isEmpty() ) {
	//			TStr Cookie(cgi.getEnvironment("HTTP_COOKIE"));
	//			int i = Cookie.Pos("lang=");
	//			if( i > -1 )
	//				Cookie = Cookie.CopyBefore(i)+Cookie.CopyAfter(i+6);
				Header = "\nSet-Cookie: "+_Cookie+"; path=/; expires=Friday, 7-Dec-2150 23:59:59 GMT;";
			}
		} catch(...) {
		}
	}
	cgi.makeHeader(TStr(
			"text/html; charset=Windows-1251\n"
			"Expires: Mon, 26 Jul 1997 05:00:00 GMT\n"
			"Connection: Close\n"
//			"Pragma: no-cache\n"
			"Cache-Control: no-cache, must-revalidate"+Header).c_str());
	printf("%s\0",s.c_str());

/*
	TFile f;
	f.OpenSave("log_cgi",false);
	fprintf(f.stream,"%s\n\n%s\n\n%s",
		(cgi.getPost()+"\n\nHeader="+Header).c_str(),
		s.c_str(),
		cgi.getEnvironment("HTTP_COOKIE"));
*/
}

