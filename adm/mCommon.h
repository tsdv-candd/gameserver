#ifndef mCommonH
#define mCommonH

#include "ssi/mf_ssi.h"
#include "sql/mMySQL.h"
#include "map/m_map.h"
#include "utils/m_utils.h"
#include "mSession.h"

class TSession
{
private:
protected:
	TStr Cookie;
	void SetCookie(const TStr &Cookie) {
		if( this->Cookie != Cookie ) {
			ClearSession();
			this->Cookie = Cookie;
			if( Cookie.Length() > 0 )
				LoadSession();
		}
	}
	virtual void ClearSession() = 0;
	virtual void LoadSession() = 0;
public:
	virtual ~TSession() {
	}
	virtual void SetValue(const TStr &Name, const TStr &Value) = NULL;
	virtual TStr GetValue(const TStr &Name) = NULL;
	virtual void ClearValue(const TStr &Cookie) = NULL;
	virtual void ClearValue(const TStr &Cookie, const TStr &Name) = NULL;
	virtual void SetValue(const TStr &Cookie, const TStr &Name, const TStr &Value) = NULL;
	virtual TStr GetValue(const TStr &Cookie, const TStr &Name) = NULL;
	virtual void SaveValue(const TStr &Cookie, const TStr &Name, const char *Value, const int Size) = NULL;
	virtual bool LoadValue(const TStr &Cookie, const TStr &Name, char *Value, const int Size) = NULL;
	virtual bool LoadValue(const TStr &Cookie, const TStr &Name, TStr &Value) = NULL;
	void         SaveValue(const TStr &Cookie, const TStr &Name, TSessionValue *);
	void         LoadValue(const TStr &Cookie, const TStr &Name, TSessionValue *);
};

class TDatabaseSession: public TSession
{
private:
	m_classes::TQuery* query;
	TStr               DaySave;
	m_ssi::TStringList SessionList;
	m_ssi::TStringList AppList;
	bool               fAppLoad;
protected:
	void CheckAppLoad() {
		if( fAppLoad )
			return;
		query->openSql("SELECT FName,FValue FROM tbs_app");
		while( !query->eof() )
		{
			AppList.Add(query->fields(0),query->fields(1));
			query->next();
		}
		query->close();
		fAppLoad = true;
	}
	virtual void ClearSession() {
		SessionList.Clear();
	}
	virtual void LoadSession() {
		query->openSql("SELECT FName,FValue FROM tbs_session WHERE FCookie='"+Cookie+"' and to_days(fDate)+("+DaySave+") > to_days(now())");
//    query->openSql("SELECT FName,FValue FROM tbs_session WHERE FCookie='"+Cookie+"' and to_days(fDate) = to_days(now())");
		while( !query->eof() )
		{
			SessionList.Add(query->fields(0),query->fields(1));
			query->next();
		}
		query->close();
	}
public:
	TDatabaseSession(m_classes::TQuery *query, const int DaySave) {
		this->query = query;
		this->DaySave = IntToStr(DaySave);
		fAppLoad = false;
	}
	virtual ~TDatabaseSession() {}
	virtual void ClearValue(const TStr &Cookie = "");
	virtual void ClearValue(const TStr &Cookie, const TStr &Name);
	virtual void SetValue(const TStr &Name, const TStr &Value);
	virtual TStr GetValue(const TStr &Name);
	virtual void SetValue(const TStr &Cookie, const TStr &Name, const TStr &Value);
	virtual TStr GetValue(const TStr &Cookie, const TStr &Name);
	virtual void SaveValue(const TStr &Cookie, const TStr &Name, const char *Value, const int Size);
	virtual bool LoadValue(const TStr &Cookie, const TStr &Name, char *Value, const int Size);
	virtual bool LoadValue(const TStr &Cookie, const TStr &Name, TStr &Value);
};

class TApp;

class TAppScript
{
private:
protected:
	TApp *app;
public:
	TAppScript(TApp *_app): app(_app) {}
	virtual void Init() {}
	virtual void Script(TStr *Result, m_ssi::TStringList *ValueList) = 0;
};

class TApp: public m_ssi::TSSIControl
{
private:
	m_ssi::TMap<TAppScript*> AppList;
	bool fInit;
protected:
	TStr                     IP;
	TStr                     Host;
	TStr                     Referer;
	TStr                     Cookie;
	TStr                     FormName;
	TSession                 *p_session;
	m_classes::TMySqlConnect *p_database;
	m_ssi::TStringList       *p_ValueList;
	void LogClient(const TStr &Client) {
		SSIValue("PageClient",Client,false);
	}
	void LogEvent(const TStr &Event) {
		SSIValue("PageEvent",Event,false);
	}
	void LogTitle(const TStr &Title, const TStr &TitleType = "") {
		SSIValue("PageTitle",Title,false);
		SSIValue("PageTitleType",TitleType,false);
	}
	void LogType(const TStr &Type) {
		SSIValue("PageType",Type,false);
	}
public:
	bool Gen;
	TApp(m_classes::TMySqlConnect *_p_database, TSession *_p_session)
	:m_ssi::TSSIControl(), fInit(false), p_database(_p_database), p_session(_p_session) {
		initRnd();
	}
	virtual ~TApp() {
		for( int i = 0; i < AppList.Count(); i++ )
			delete *(AppList.Values(i));
	}
	m_classes::TMySqlConnect* GetDatabase()  { return p_database; }
	m_ssi::TStringList*       GetValueList() { return p_ValueList; }
	TSession*                 GetSession()   { return p_session; }
	void SetSessionValue(const TStr& Name, const TStr& Value) {
		if( Cookie.Length() > 0 )
			p_session->SetValue(Cookie,Name,Value);
	}
	TStr GetSessionValue(const TStr& Name) {
		if( Cookie.Length() > 0 )
			return p_session->GetValue(Cookie,Name);
		return "";
	}

	virtual void Init();
	virtual void Script(TStr *Result, TStr *Header, m_ssi::TStringList *ValueList);

	void AddAppScript(const TStr &Name, TAppScript *p) {
		m_ssi::TMapItem<TAppScript*> *f = AppList.Find(Name);
		if( f != NULL ) {
			delete *(f->GetValue());
			f->SetValue(p);
		} else
			AppList.Add(Name,p);
	}
	TAppScript* FindAppScript(const TStr &Name);
};

TStr CookieCreate(int i);
TStr CookieHeader(const TStr &Name,const TStr &Cookie);
TStr CookieExpand(TStr &Cookie, const TStr &Name = "GUID");

#endif
