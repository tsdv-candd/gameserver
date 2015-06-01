#include "mSSIService.h"

void QueryToValueList(m_classes::TQuery *query, m_ssi::TStringList *sl)
{
	for( int i = 0; i < query->fieldCount(); i++ )
		sl->SetValue(query->fieldName(i),query->fields(i));
}

void ValueListToSSI(m_ssi::TSSIControl *ssi, m_ssi::TStringList *sl)
{
	for( int i = 0; i < sl->Count(); i++ )
		ssi->SSIValue(sl->Names(i),sl->Values(i),false);
}

void ValueListToSSISQL(m_ssi::TSSIControl *ssi, m_ssi::TStringList *sl)
{
	for( int i = 0; i < sl->Count(); i++ )
		ssi->SSIValue(sl->Names(i),StrToSQL(sl->Values(i)),false);
}

TStr ListToSSI(m_ssi::TSSIControl *ssi, const TStr &ssiName, const int ID)
{
	TStr s;
	int i = 1;
	ssi->SSIBlock("&isSelected", ID == i,false);
	TStr ss(ssi->SSIResDef(ssiName+i,""));
	while( ss.Length() > 0 )
	{
		s += ss;
		ssi->SSIBlock("&isSelected", ID == ++i);
		ss = ssi->SSIResDef(ssiName+i,"");
	}
	return s;
}

void QueryToSSI(m_ssi::TSSIControl *app, m_classes::TQuery *query)
{
	for( int i = 0; i < query->fieldCount(); i++ )
	if( query->eof() )
		app->SSIValue(query->fieldName(i),"",false);
	else
		app->SSIValue(query->fieldName(i),query->fields(i),false);
}

TStr QueryToCombo(m_classes::TQuery *query, const TStr &ID)
{
	TStr s;
	while( !query->eof() )
	{
		s += TStr("<option value=")+query->fields(0);
		if( ID == query->fields(0) )
			s += " selected";
		s += TStr(">")+query->fields(1)+"</option>";
		query->next();
	}
	return s;
}

// TGroup

void TGroup::Add(
	const TStr &AttrID,
	const TStr &ssiStream,
	const TStr &ssiBlock,
	TScript* p_script)
{
	TGroupNode *p = new TGroupNode(AttrID,ssiStream,ssiBlock,p_script);
	if( Root == NULL )
		Root = p;
	else {
		TGroupNode *n = Root;
		while( n->next != NULL )
			n = n->next;
		n->next = p;
	}
}

TStr TGroup::Script(m_classes::TQuery *query)
{
	this->query = query;
	if( Root == NULL || query->eof() )
		return "";
	TGroupNode *p = Root;
	while( p != NULL && p->AttrID.Length() > 0 ) {
		p->ID = query->fieldByName(p->AttrID);
		p = p->next;
	}
	return OnScript(1);
}

TGroupNode* TGroup::GetNode(int Count)
{
	TGroupNode *p = Root;
	while( Count > 1 )
	{
		p = p->next;
		Count--;
	}
	return p;
}

bool TGroup::isContinue(int Count)
{
	if( query->eof() )
		return false;
	TGroupNode *p = Root;
	while( Count > 0 && p->AttrID.Length() > 0 )
	{
		if( p->ID != query->fieldByName(p->AttrID) )
			return false;
		Count--;
		p = p->next;
	}
	return true;
}

TStr TGroup::OnScript(int Count)
{
	TStr Result;
	bool fFirst = true;
	TGroupNode *p = GetNode(Count);
	while( isContinue(Count) ) {
		if( p->p_script != NULL )
			p->p_script->OnScript();
		if( p->next != NULL ) {
			ssi->SSIValue(p->ssiStream,OnScript(Count+1));
			if( !query->eof() && p->AttrID.Length() > 0 )
				p->ID = query->fieldByName(p->AttrID);
		} else {
			QueryToSSI(ssi,query);
			query->next();
		}
		ssi->SSIBlock("&notFirst",!fFirst,false);
		fFirst = false;
		Result += ssi->SSIRes(p->ssiBlock);
	}
	return Result;
}
