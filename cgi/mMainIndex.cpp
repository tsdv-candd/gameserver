#include <time.h>
#include <stdio.h>

#include "file/m_file.h"
#include "xml/m_xmlparser.h"
#include "cgi/mCgi.h"
#include "sql/mMySQL.h"
#include "convert/mConvert.h"
#include "m_string.h"

int main(void)
{
	TCGI cgi;
	cgi.makeHeader(
			"text/html; charset=Windows-1251"
			"\nExpires: Mon, 26 Jul 1997 05:00:00 GMT"
			"\nX-Accel-Expires: 0"
//			"\nPragma: no-cache"
			"\nCache-Control: none");

	cgi.init();

	TStr Link(cgi.getParamValueByName("Link"));
	TStr Referrer(cgi.getParamValueByName("Referrer"));
	TStr Lang(cgi.getEnvironment("HTTP_ACCEPT_LANGUAGE"));

	if( Referrer.isEmpty() )
		Referrer = cgi.getEnvironment("HTTP_REFERER");

	TStr Language;
#ifdef CASINO_VABANK
	Lang = Lang.Copy(0,2);
	if( Lang != "en" && Lang != "ru" )
		Lang = "ru";
	TStr Cookie(cgi.getEnvironment("HTTP_COOKIE"));
	Cookie = Cookie.CopyAfter("lang=").CopyBefore(";");
	if( !Cookie.isEmpty() )
	{
		if( Cookie == "1" || Cookie == "RU" )
			Lang = "RU";
		else
			Lang = "EN";
	}
	if( TStr(cgi.getParamValueByName("lang")) == "ru" )
		Lang = "RU";
	if( TStr(cgi.getParamValueByName("lang")) == "en" )
		Lang = "EN";
	if( Lang.isEmpty() )
		Lang = "RU";
	if( Lang == "ru" )
		Language = "russian";
	else
		Language = "english";
#else
	Lang = "EN";
	Language = "english";
#endif

	TStr PID(cgi.getParamValueByName("cid")), PIDKEY;
	if( !PID.isEmpty() ) {
		PIDKEY = PID.CopyBefore(5);
	} else {
		if( cgi.paramCount() > 0 )
			if(
				cgi.getParamValue(0)[0] == '\0' &&
				TStr(cgi.getParamName(0)) != "referrer" &&
				TStr(cgi.getParamName(0)) != "link"
			)
				PID = cgi.getParamName(0);
		if( !PID.isEmpty() )
			PIDKEY = PID.CopyBefore(",");
	}

	TStr QUERY(cgi.getEnvironment("QUERY_STRING"));
	TStr IP(cgi.getEnvironment("REMOTE_ADDR"));
	m_classes::TMySqlConnect database;
	database.setHostName("localhost");
	database.setDatabase("casino_last");
	database.setLogin("root");
	database.setPasswd("");
	database.connect();
	m_classes::TMySqlQuery query(&database);
	query.execSql(
		"INSERT INTO log_url ("
			"fdate,"
			"ftime,"
			"freferrer,"
			"fip,"
			"fid,"
			"fidkey,"
			"fquery"
		") VALUES ("
			"now(),"
			"now(),"
			"'"+StrToSQL(Referrer)+"',"
			"'"+StrToSQL(IP)+"',"
			"'"+StrToSQL(PID)+"',"
			"'"+StrToSQL(PIDKEY)+"',"
			"'"+StrToSQL(QUERY)+"'"
		")");
	TStr RefID(query.last_id());
	try {
		TFile f;
		f.OpenLoad("index.ini");
		char Str[10000];
		int FileLen = fread(Str, 1, 10000, f.stream);
		Str[FileLen] = '\0';

		TStr _Str(Str,FileLen);

		_Str = _Str.Replace("$tb$",__escape(cgi.getParamValueByName("tb")));

		_Str = _Str.Replace("$LOG_PageReferrer$",__escape(cgi.getParamValueByName("Referrer")));
		_Str = _Str.Replace("$REFID$",RefID);
		_Str = _Str.Replace("$LANG$",ToLowerCase(Lang));
		_Str = _Str.Replace("$LANGUAGE$",Language);
		if( !Link.isEmpty() )
			Link = "&helpDefaultLink="+Link+"&";
		_Str = _Str.Replace("$LINK$",Link);
		printf("%s",_Str.c_str());
	} catch( ... ) {
		printf("error");
	}

	return 0;
}




