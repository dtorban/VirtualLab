#ifndef VIRTUALLAB_NET_NET_INTERFACE_H_
#define VIRTUALLAB_NET_NET_INTERFACE_H_

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>
#include <ws2tcpip.h>
#include <stdio.h>
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#else
#define SOCKET int
#include "stdint.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#ifndef WIN32
  #include <netinet/tcp.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <sys/wait.h>
  #include <signal.h>
	#include <sys/un.h>

	#include <stdio.h>  
	#include <string.h>   //strlen  
	#include <stdlib.h>  
	#include <errno.h>  
	#include <unistd.h>   //close  
	#include <arpa/inet.h>    //close  
	#include <sys/types.h>  
	#include <sys/socket.h>  
	#include <netinet/in.h>  
	#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#endif

#include <vector>

namespace vl {

//#define NET_SIZEOFINT 4

enum NetMessageType {
	MSG_none = 0,
	MSG_registerModel = 1,
	MSG_deregisterModel = 2,
	MSG_getModels = 3
};

class NetInterface {
public:
	virtual ~NetInterface() {}
	
	void sendMessage(SOCKET s, NetMessageType type, const unsigned char *data, int len);
	NetMessageType receiveMessage(SOCKET s, int& len);
	int sendData(SOCKET s, const unsigned char *buf, int len);
	int receiveData(SOCKET s, unsigned char *buf, int len);
};

}

#endif