#include "VirtualLab/net/Client.h"

#include <sstream>
#include <iostream>

namespace vl {

// get sockaddr, IPv4 or IPv6:
void *get_in_addr2(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Client::Client(const std::string &serverIP, int serverPort)
{
  std::cout <<"connecting..." << std::endl;


  std::stringstream port;
  port << serverPort;

#ifdef WIN32  // WinSock implementation

  WSADATA wsaData;
  SOCKET sockfd = INVALID_SOCKET;
  struct addrinfo hints, *servinfo, *p;
  int rv;

  rv = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (rv != 0) {
    std::stringstream s;
    s << "WSAStartup failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with Windows networking." << std::endl;
    exit(1);
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;



  if ((rv = getaddrinfo(serverIP.c_str(), port.str().c_str(), &hints, &servinfo)) != 0) {
    std::stringstream s;
    s << "getaddrinfo() failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with Windows networking." << std::endl;
    WSACleanup();
    exit(1);
  }

  //This is a temporary fix to ensure the client can connect and that the connection is not refused
  p = NULL;
  while(p == NULL){
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET) {
	      std::stringstream s;
        s << "socket() failed with error " << WSAGetLastError() << "; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      if (connect(sockfd, p->ai_addr, (int)p->ai_addrlen) == SOCKET_ERROR) {
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
	      std::stringstream s;
        s << "connect() to server socket failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      break;
    }
  }

  if (p == NULL) {
    std::cout <<"VRNetClient failed to connect -- exiting." << "Check for a problem with Windows networking." << std::endl;
    exit(2);
  }

  //inet_ntop(p->ai_family, get_in_addr2((struct sockaddr *)p->ai_addr), s, sizeof s);
  //printf("client: connecting to %s\n", s);
  std::cout <<"VRNetClient connected." << std::endl;

  freeaddrinfo(servinfo); // all done with this structure

  // Disable Nagle's algorithm
  char value = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

  socketFD = sockfd;


#else  // BSD sockets implementation
/*  //"server_socket"
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];
  char problemString[50];

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(serverIP.c_str(), port.str().c_str(), &hints, &servinfo)) != 0) {
    std::stringstream s;
    s << "getaddrinfo() failed with error: " << rv;
    std::cout <<s.str() << "Check for a problem with networking." << std::endl;
    exit(1);
  }

  //This is a temporary fix to ensure the client can connect and that the connection is not refused
  p = NULL;
  while(p == NULL){
    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
        std::stringstream s;
        s << "socket() failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
        std::stringstream s;
        s << "client (pid=" << getpid() << "): connect() to server socket failed; will retry.";
        std::cout <<s.str() << std::endl;
        continue;
      }

      break;
    }
  }
  if (p == NULL) {
    std::cout <<"VRNetClient failed to connect -- exiting." << "Check for a problem with networking." << std::endl;
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr2((struct sockaddr *)p->ai_addr), s, sizeof(s));
  std::cout <<"VRNetClient connected to " + std::string(s) << std::endl;

  freeaddrinfo(servinfo); // all done with this structure

  // Disable Nagle's algorithm
  char value = 1;
  setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

  socketFD = sockfd;*/
  
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   
   portno = serverPort;
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      std::cout << "ERROR opening socket" << std::endl;
      exit(1);
   }
	
   server = gethostbyname(serverIP.c_str());
   
   if (server == NULL) {
      std::cout << "ERROR no such host" << std::endl;
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   while (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      std::cout << "ERROR connecting" << std::endl;
      //exit(1);
   } 

    // Disable Nagle's algorithm
    char value = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

    socketFD = sockfd;

#endif

}


Client::~Client()
{
  std::cout <<"Client closing socket." << std::endl;
#ifdef WIN32
  closesocket(socketFD);
  WSACleanup();
#else
  std::cout << "close socket" << std::endl;
  close(socketFD);
#endif

/*  for (int f = 0; f < messageQueues.size(); f++) {
    delete messageQueues[f];
  }*/
}


/*ClientMessageQueue::ClientMessageQueue(NetInterface* net, SOCKET socketFD, int id)
  : net(net), socketFD(socketFD), id(id) {
}
int ClientMessageQueue::getId() { return id; }
void ClientMessageQueue::waitForMessage() {
  int id = getId();
  net->sendMessage(socketFD, MSG_receiveQueueMessage, (unsigned char *)&id, sizeof(int));
  int returnVal;
  net->receiveData(socketFD, (unsigned char *)&returnVal, sizeof(int));
}
void ClientMessageQueue::sendMessage() {
  int id = getId();
  net->sendMessage(socketFD, MSG_sendQueueMessage, (unsigned char *)&id, sizeof(int));
}
int ClientMessageQueue::sendData(const unsigned char *buf, int len) {
  int id = getId();
  net->sendMessage(socketFD, MSG_sendQueueData, (unsigned char *)&id, sizeof(int));
  net->sendData(socketFD, (unsigned char *)&len, sizeof(int));
  return net->sendData(socketFD, buf, len);
}
int ClientMessageQueue::receiveData(unsigned char *buf, int len) {
  int id = getId();
  net->sendMessage(socketFD, MSG_receiveQueueData, (unsigned char *)&id, sizeof(int));
  return net->receiveData(socketFD, buf, len);
}*/

}
