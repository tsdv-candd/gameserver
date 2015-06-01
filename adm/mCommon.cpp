#include <time.h>
#include "utils/m_datetime.h"
#include "mCommon.h"
#include "mSSIService.h"

const char* S_SESSION_FAILED = "Session var not present";
const char* S_APP_FAILED = "App var not present";

TStr CookieCreate(int i)
{
	time_t cookie1;
	time(&cookie1);
	TStr Cookie(LongToStrHex((long)cookie1));
	while( Cookie.Length() < i )
		Cookie = LongToStrHex(getRnd()) + Cookie;
	return Cookie.CopyBefore(i);
}

TStr CookieHeader(const TStr &Name, const TStr &Cookie)
{
	return "\nSet-Cookie: "+Name+"=" + Cookie + "; expires=Friday, 7-Dec-2007 23:59:59 GMT;";
}

TStr CookieExpand(TStr &Cookie, const TStr &Name)
{
		TStr c = Cookie.CopyAfter(Name + "=");
		c = c.CopyBefore(";").CopyBefore(",");
		if( c.Length() != 32 ) {
			c = Cookie.CopyAfter(Name + "=").CopyAfter(Name + "=");
			c = c.CopyBefore(";").CopyBefore(",");
		}
		if( c.Length() != 32 ) {
			c = Cookie.CopyAfter(Name + "=").CopyAfter(Name + "=").CopyAfter(Name + "=");
			c = c.CopyBefore(";").CopyBefore(",");
		}
		if( c.Length() != 32 )
			c.Clear();
		return c;
}

// -- TApp

void TApp::Init()
{
	fInit = true;
	for( int i = 0; i < AppList.Count(); i++ )
		(*AppList.Values(i))->Init();
	initRnd();
}

// -- TSession

void TSession::SaveValue(
	const TStr &Cookie,
	const TStr &Name,
	TSessionValue *p)
{
	TStr s;
	m_ssi::TStringList sl;
	p->SaveValues(sl);
	for( int i = 0; i < sl.Count(); i++ )
	{
		TStr l(sl.Names(i) + "=" + sl.Values(i));
		s += TStr(l.Length()) + "-" + l;
	}
	SaveValue(Cookie,Name,s.c_str(),s.Length());
}

void TSession::LoadValue(
	const TStr &Cookie,
	const TStr &Name,
	TSessionValue *p)
{
	TStr s;
	if( LoadValue(Cookie,Name,s) ) {
		m_ssi::TStringList sl;
		while( s.Length() > 0 )
		{
			int Len = s.GetBefore("-").ToIntDef(0);
			TStr n(s.GetBefore(Len));
			sl.Add(n.CopyBefore("="),n.CopyAfter("="));
		}
		p->LoadValues(sl);
	}
}

// -- TDatabaseSession

void TDatabaseSession::SetValue(
	const TStr &Name,
	const TStr &Value)
{
	CheckAppLoad();
	m_ssi::TMapItem<TStr> *p = AppList.Find(Name);
	if( p == NULL ) {
		query->execSql("INSERT INTO tbs_app (FName,FValue) VALUES ('"+Name+"','"+Value+"')");
		AppList.Add(Name,Value);
	} else {
		query->execSql("UPDATE tbs_app SET FValue='"+Value+"' WHERE FName='"+Name+"'");
		p->SetValue(Value);
	}
}

TStr TDatabaseSession::GetValue(const TStr &Name)
{
	CheckAppLoad();
	return AppList.Values(Name);
}

// Session

void TDatabaseSession::SetValue(
	const TStr &Cookie,
	const TStr &Name,
	const TStr &Value)
{
	SetCookie(Cookie);
	if( Cookie.Length() > 0 ) {
		m_ssi::TMapItem<TStr> *p = SessionList.Find(Name);
		if( p == NULL ) {
			query->execSql("INSERT INTO tbs_session (FName,FCookie,FValue,FDate) VALUES ('"+Name+"','"+Cookie+"','"+Value+"',now())");
			SessionList.Add(Name,Value);
		} else {
			query->execSql("UPDATE tbs_session SET FValue='"+Value+"', FDate=now() WHERE FName='"+Name+"' and FCookie='"+Cookie+"'");
			p->SetValue(Value);
		}
	}
}

TStr TDatabaseSession::GetValue(
	const TStr &Cookie,
	const TStr &Name)
{
	SetCookie(Cookie);
	return SessionList.Values(Name);
}

void TDatabaseSession::SaveValue(
	const TStr &Cookie,
	const TStr &Name,
	const char *Value,
	const int Size)
{
/*
NULL ASCII 0.  represent this by `\0'
\    ASCII 92, Represent this by `\\'
'    ASCII 39, Represent this by `\''
"    ASCII 34, Represent this by `\"'
*/
	if( Cookie.Length() > 0 ) {
		query->openSql("SELECT ID FROM tbs_session_blob WHERE FName='"+Name+"' and FCookie='"+Cookie+"' and to_days(fDate) > to_days(now())-"+DaySave);
		char * s = new char[Size+Size];
		mysql_escape_string(s,Value,Size);
		TStr v(s);
		delete[] s;
		if( query->eof() )
			query->execSql("INSERT INTO tbs_session_blob (FName,FCookie,FValue,FDate) VALUES ('"+Name+"','"+Cookie+"','"+v+"',now())");
		else
			query->execSql(TStr("UPDATE tbs_session_blob SET FValue='"+v+"', FDate=now() WHERE ID=")+query->fields(0));
		query->close();
	}
}

bool TDatabaseSession::LoadValue(
	const TStr &Cookie,
	const TStr &Name,
	char *Value,
	const int Size)
{
	if( Cookie.Length() > 0 ) {
		query->openSql("SELECT FValue,Length(FValue) FROM tbs_session_blob WHERE FCookie='"+Cookie+"' and FName='"+Name+"' and to_days(fDate) > to_days(now())-"+DaySave);
		if( !query->eof() ) {
			if( Size != StrToInt(query->fields(1)) )
				throw TExcept("LoadValue(): Size != BDSize");
			memmove(Value,query->fields(0),Size);
			return true;
		}
	}
	return false;
}

bool TDatabaseSession::LoadValue(
	const TStr &Cookie,
	const TStr &Name,
	TStr  &Value)
{
	if( Cookie.Length() > 0 ) {
		query->openSql("SELECT FValue,Length(FValue) FROM tbs_session_blob WHERE FCookie='"+Cookie+"' and FName='"+Name+"' and to_days(fDate) > to_days(now())-"+DaySave);
		if( !query->eof() ) {
			Value += query->fields(0);
			return true;
		}
	}
	return false;
}

void TDatabaseSession::ClearValue(const TStr &Cookie)
{
	if( Cookie.Length() > 0 ) {
		query->execSql("DELETE FROM tbs_session WHERE FCookie='"+Cookie+"'");
		query->execSql("DELETE FROM tbs_session_blob WHERE FCookie='"+Cookie+"'");
	}
}

void TDatabaseSession::ClearValue(const TStr &Cookie, const TStr &Name)
{
	if( Cookie.Length() > 0 ) {
		query->execSql("DELETE FROM tbs_session WHERE FCookie='"+Cookie+"' and FName='"+Name+"'");
		query->execSql("DELETE FROM tbs_session_blob WHERE FCookie='"+Cookie+"' and FName='"+Name+"'");
	}
}

// TApp

TAppScript* TApp::FindAppScript(const TStr &Name)
{
	m_ssi::TMapItem<TAppScript*> *f = AppList.Find(Name);
	if( f != NULL )
		return *(f->GetValue());
	return NULL;
}

void TApp::Script(TStr *Result, TStr *Header, m_ssi::TStringList *ValueList)
{
	Gen = true;
	p_ValueList = ValueList;

	if( !fInit )
		Init();
	else
	if( ValueList->Values("reload").Length() > 0 )
		Init();

	IP = p_ValueList->Values("IP");
	Host = p_ValueList->Values("Host");
	Referer = p_ValueList->Values("Referer");
	Cookie = p_ValueList->Values("Cookie");
	Cookie = CookieExpand(Cookie,"GUID").CopyBefore(32);
	if( Cookie.Length() == 0 ) {
		Cookie = CookieCreate(32);
		*Header += CookieHeader("GUID",Cookie);
	}

	SSIClear();
	p_database->rollback();

	// !!!
//  if( this->Cookie.Length() > 16 )
//    this->Cookie.Delete(16,this->Cookie.Length());
	// !!!
	FormName = p_ValueList->Values("FormName");
	if( FormName.Length() == 0 )
		for( int i = 0; i < p_ValueList->Count(); i++ )
			if( p_ValueList->Values(i).Length() == 0 ) {
				FormName = p_ValueList->Names(i);
				break;
			}
}

