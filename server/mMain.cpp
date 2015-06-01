#include "mServer.h"
#include "mCommon.h"
#include "utils/m_utils.h"

#include <stdio.h>

 int main(/*int argc, char* argv[]*/)
{

	initRnd();
	fDebug = true;

//  if ( argc >= 2 )
//    Port = atoi( argv[1] );
//  if ( argc >= 3 )
//    fDebug = (argv[2][0] == 'd') || (argv[1][0] == 'd');

	static TServer s;

	s.Create(false);
	while( s.isOpen() ) {
		Sleep(1000);
	}

	return 0;
}






