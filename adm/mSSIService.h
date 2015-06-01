#ifndef mSSIServiceH
#define mSSIServiceH

#include "map/m_map.h"
#include "sql/mSQL.h"
#include "ssi/mf_ssi.h"

TStr QueryToCombo(m_classes::TQuery *query, const TStr &ID = "");
void QueryToSSI(m_ssi::TSSIControl *ssi, m_classes::TQuery *query);
TStr ListToSSI(m_ssi::TSSIControl *ssi, const TStr &ssiName, const int ID);
void ValueListToSSI(m_ssi::TSSIControl *ssi, m_ssi::TStringList *sl);
void ValueListToSSISQL(m_ssi::TSSIControl *ssi, m_ssi::TStringList *sl);
void QueryToValueList(m_classes::TQuery *query, m_ssi::TStringList *sl);

class TScript
{
public:
	void virtual OnScript() = NULL;
};

class TObjectStorage;

class TSSIInfo
{
private:
	TStr Name;
	TObjectStorage* ObjList;
protected:
	m_ssi::TSSIControl *app;

	TSSIInfo(m_ssi::TSSIControl *_app, const TStr &_Name = ""): app(_app), Name(_Name), ObjList(NULL) {
	}
	virtual ~TSSIInfo() {}

	void    Do() { app->SSIValue(Name,Stream()); }
	virtual TStr Stream() = 0;
};

class TGroupNode
{
public:
	TStr ID;
	TStr AttrID;
	TStr ssiStream;
	TStr ssiBlock;
	TScript *p_script;
	TGroupNode *next;

	TGroupNode(const TStr &_AttrID, const TStr &_ssiStream, const TStr &_ssiBlock, TScript* _p_script = NULL)
	: next(NULL), AttrID(_AttrID), ssiStream(_ssiStream), ssiBlock(_ssiBlock), p_script(_p_script) {}
	~TGroupNode() { if( next != NULL ) delete next; }
};

class TGroup
{
private:
	m_ssi::TSSIControl *ssi;
	TGroupNode         *Root;
	m_classes::TQuery  *query;

	TGroupNode* GetNode(int Count);
	bool        isContinue(int Count);
	TStr        OnScript(int Count);
public:
	TGroup(m_ssi::TSSIControl *_ssi): ssi(_ssi),Root(NULL) {}
	~TGroup() { if( Root != NULL ) delete Root; }
	void   Add(const TStr &AttrID,const TStr &ssiStream,const TStr &ssiBlock, TScript* p_script = NULL);
	TStr   Script(m_classes::TQuery *query);
};

#endif
