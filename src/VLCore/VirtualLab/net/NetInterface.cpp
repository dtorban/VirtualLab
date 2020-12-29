#include "VirtualLab/net/NetInterface.h"

#include <iostream>

namespace vl {


#define LOGD(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGE(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)
#define LOGW(...) do { printf(__VA_ARGS__); printf("\n"); } while(0)

int NetInterface::sendData(SOCKET s, const unsigned char *buf, int len) const {
  int total = 0;        // how many bytes we've sent
  int bytesleft = len;  // how many we have left to send
  int n = 0;
  while (total < len) {
#ifdef WIN32
	  n = send(s, (char *)(buf + total), bytesleft, 0);
#else
	  n = send(s, buf + total, bytesleft, 0);
#endif
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  return n==-1?-1:total; // return -1 on failure, total on success
}

int NetInterface::receiveData(SOCKET s, unsigned char *buf, int len) const {
  int total = 0;        // how many bytes we've received
  int bytesleft = len; // how many we have left to receive
  int n = 0;
  while (total < len) {
#ifdef WIN32
	  n = recv(s, (char *)(buf + total), bytesleft, 0);
#else
	  n = recv(s, (char *)(buf + total), bytesleft, 0);
#endif
    if (n == -1) { break; }
    total += n;
    bytesleft -= n;
  }
  return n==-1?-1:total; // return -1 on failure, total on success
}

void NetInterface::sendMessage(SOCKET s, NetMessageType type, const unsigned char *data, int len) const {
	int dataSize =  len + 1 + sizeof(int);
	unsigned char *buf = new unsigned char[dataSize+1];
	//1. add 1-byte message header
	buf[0] = type;
	// 2. add the size of the message data so receive will know how
	// many bytes to expect.
	memcpy(&buf[1], &len, sizeof(int));
	// 3. send the chars that make up the data.
	memcpy(&buf[1 + sizeof(int)], data, len);
	//4. send package
	sendData(s,buf,dataSize);
	//5. delete buffer
	delete[] buf;
}

NetMessageType NetInterface::receiveMessage(SOCKET s, int& len) const {
	unsigned char type;
#ifdef WIN32
	len = recv(s, (char*)&type, 1, 0);
#else
	len = read(s, &type, 1);
#endif
	
	if (len == 0) {
		return MSG_none;
	}

#ifdef WIN32
	recv(s, (char*)&len, sizeof(int), 0);
#else
	read(s, &len, sizeof(int));
#endif

	return static_cast<NetMessageType>(type);
}

}
