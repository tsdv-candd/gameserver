#ifndef mSessionH
#define mSessionH

#include "map/m_map.h"
#include "m_string.h"

class TSessionValue
{
friend class TDatabaseSession;
private:
protected:
public:
	virtual      ~TSessionValue() {}
	virtual void LoadValues(m_ssi::TStringList &sl) = NULL;
	virtual void SaveValues(m_ssi::TStringList &sl) = NULL;
	virtual void Clear() = NULL;
};

#endif
