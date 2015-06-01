#ifndef m_storageH
#define m_storageH

#include "utils/m_object.h"
#include "ssi/mf_ssi.h"
#include "sql/mMySQL.h"

using namespace :: m_object;

class TMySqlStorage: public TResourceStorage<m_classes::TMySqlConnect>
{
private:
  TStr host;
  TStr login;
  TStr password;
  TStr db;
protected:
  m_classes::TMySqlConnect* OnCreate() {
    m_classes::TMySqlConnect* p = new m_classes::TMySqlConnect();
    p->setHostName(host);
    p->setLogin(login);
    p->setPasswd(password);
    p->setDatabase(db);
    p->connect();
    return p;
  }
public:
  TMySqlStorage(
    const unsigned int Size,
    const TStr& _host,
    const TStr& _login,
    const TStr& _password,
    const TStr& _db)
  : TResourceStorage<m_classes::TMySqlConnect>(Size),
    host(_host), login(_login), password(_password), db(_db)
  {}
};

class TSSIStorage: public TResourceStorage<m_ssi::TSSIControl>
{
private:
  TStr file;
protected:
  m_ssi::TSSIControl* OnCreate() {
    m_ssi::TSSIControl* p = new m_ssi::TSSIControl();
    p->SSILoad(file);
    return p;
  }
  void  OnGet(m_ssi::TSSIControl* p, const unsigned int) {
    p->SSIClear();
  }
public:
  TSSIStorage(const unsigned int Size, const TStr& _file)
  : TResourceStorage<m_ssi::TSSIControl>(Size), file(_file) {}
};

#endif
