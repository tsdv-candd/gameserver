#include "cgi/mCgi.h"
#include "file/m_file.h"
#include "socket/m_socket.h"
#include "m_string.h"
#include <stdio.h>
#include <time.h>

int main()
{
	TCGI cgi;
	cgi.init();
	cgi.makeHeader("text/html; charset=Windows-1251");
	TStr OrderID(cgi.getParamValueByName("OrderID"));
	TStr Status(cgi.getParamValueByName("Status"));
	TStr Code(cgi.getParamValueByName("Code"));
	TStr s;

	if( !OrderID.isEmpty() && !Status.isEmpty() ) {
		TStr IP(cgi.getEnvironment("REMOTE_ADDR"));
//		if( IP == "127.0.0.1" ) {
			TSocketClient client;
			if( client.Connect("127.0.0.1",1024))
			{
			  s = "<pay status=\""+Status+"\" id="+OrderID;
			  if( !Code.isEmpty() )
				s += " code=\""+Code+"\"";
			  s += " />";
			  client.Send("<casino command=pay>"+s+"</casino>");
			 s = client.Read(60000);
//		        } else printf("error while connecting\n");
		}
	}
	if( s.isEmpty() )
		s = "Error";
	printf("%s",s.c_str());
	return 0;
}

 
