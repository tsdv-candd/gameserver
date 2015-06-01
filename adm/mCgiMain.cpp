#include <time.h>
#include <stdio.h>

#include "cgi/mCgi.h"
#include "utils/m_datetime.h"
#include "file/m_file.h"
#include "mMain.h"

#ifndef TARGET_WIN32
#include <sys/time.h>
unsigned long GetTickCount()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday( &tv, &tz );
	long ms = tv.tv_sec * 1000 + (tv.tv_usec/1000);
	return ms;
}
#else
#include <windows.h>
#endif

int main(void)
{
	long ScriptMs = GetTickCount();
	TCGI cgi;
	m_classes::TMySqlConnect database;
	m_classes::TMySqlQuery query(&database);
	TDatabaseSession session(&query,1);
	TMain app(&database,&session);
	bool fcgi = cgi.init();
	m_ssi::TStringList sl;
	TStr s;
	TStr Cookie;
	if ( fcgi ) {
		for( int a = 0; a < cgi.paramCount (); a ++ ) {
//      SaveFile(cgi.getParamName( a ), cgi.getParamValue( a ));
//      if( cgi.getParamValue( a )[0] != '\0' )
				sl.Add( cgi.getParamName( a ), cgi.getParamValue( a ));
		}
		if( cgi.getEnvironment( "HTTP_REFERER" )[0] != '\0' ) {
			sl.Add( "REFERER", cgi.getEnvironment( "HTTP_REFERER" ));
			sl.Add( "HTTP_REFERER", cgi.getEnvironment( "HTTP_REFERER" ));
		}
		if( cgi.getEnvironment( "REMOTE_ADDR" )[0] != '\0' )
			sl.Add( "REMOTE_ADDR", cgi.getEnvironment( "REMOTE_ADDR" ));
		if( cgi.getEnvironment( "REMOTE_HOST" )[0] != '\0' )
			sl.Add( "REMOTE_HOST", cgi.getEnvironment( "REMOTE_HOST" ));
		if( cgi.getEnvironment( "HTTP_X_FORWARDED_FOR" )[0] != '\0' )
			sl.Add( "HTTP_X_FORWARDED_FOR", cgi.getEnvironment( "HTTP_X_FORWARDED_FOR" ));
		if( cgi.getEnvironment( "HTTP_USER_AGENT" )[0] != '\0' )
			sl.Add( "HTTP_USER_AGENT", cgi.getEnvironment( "HTTP_USER_AGENT" ));
		for( int a = 0; a < cgi.FileCount(); a ++ ) {
//      TFile f(cgi.FileName(a).c_str(),true);
//      fwrite(cgi.File(a).c_str(), cgi.File(a).Length(), 1, f.stream);
			sl.Add( cgi.FileName(a), cgi.File(a) );
		}
		Cookie = cgi.getEnvironment( "HTTP_COOKIE" );
/*
		Cookie = Cookie.CopyAfter("GUID=");
		Cookie = Cookie.CopyBefore(";");
		Cookie = Cookie.CopyBefore(",");
*/
		sl.Add( "COOKIE", Cookie);
	}
	try {
		TStr Header;
		app.Script(&s,&Header,&sl);
		if( s.Length() > 0 )
			Header = "text/html; charset=Windows-1251\nConnection: Close" + Header;

/*
		if( Cookie.Length() == 0 ) {
			// Create cookie client
			long cookie1;
			time(&cookie1);
			Cookie = m_ssi::LongToStrHex(getRnd()) + m_ssi::LongToStrHex(cookie1) + m_ssi::LongToStrHex(getRnd()) + m_ssi::LongToStrHex(getRnd());
			Header = Header + "\nSet-Cookie: GUID=" + Cookie + ";";
		}
*/
		ScriptMs = GetTickCount() - ScriptMs;
		s += TStr("<!-- SCRIPTTIME = ") + (int)(ScriptMs) + " ms -->";

		if( sl.Values("system") == "debug" ) {
			s += TStr("\n<br>REQUEST_METHOD='") + cgi.getEnvironment( "REQUEST_METHOD" );
			s += TStr("\n<br>HTTP_USER_AGENT='") + cgi.getEnvironment( "HTTP_USER_AGENT" );
			s += TStr("\n<br>QUERY_STRING='") + cgi.getEnvironment( "QUERY_STRING" );
			s += TStr("\n<br>COOOKIE='") + cgi.getEnvironment( "HTTP_COOKIE" ) + "','" + Cookie + "'";
			s += TStr("\n<br>REFERER=") + cgi.getEnvironment( "HTTP_REFERER" );
			s += TStr("\n<br>HOST=") + cgi.getEnvironment( "REMOTE_ADDR" );
			s += TStr("\n<br>PROXY=") + cgi.getEnvironment( "HTTP_X_FORWARDED_FOR" );
			s += "\n<br>";
			if ( fcgi )
				for( int a = 0; a < cgi.paramCount(); a ++ )
					s += TStr("\n<br><dd>") + cgi.getParamName( a ) + "=" + cgi.getParamValue( a );
			s += "\n<br>";
			for( int a = 0; a < sl.Count(); a ++ )
				s += TStr("\n<br><dd>") + sl.Names(a) + "=" + sl.Values(a);
		}
		//
		if ( fcgi && app.Gen ) {
			Header += "\nContent-Length: " + IntToStr(s.Length());
			cgi.makeHeader(Header.c_str());
		}
		if( app.Gen )
			printf("%s",s.c_str());
	} catch ( TExcept &e ) {
		if( fcgi )
			cgi.makeHeader("text/html; charset=Windows-1251");
		printf("FATAL %s",e.getError());
	}
	if ( !fcgi ) {
		printf("\n\nPRESS ENTER\n");
		getchar();
	}

	return 0;
}


