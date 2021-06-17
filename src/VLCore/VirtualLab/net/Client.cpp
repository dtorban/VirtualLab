#include "VirtualLab/net/Client.h"

#include <sstream>
#include <iostream>
#include "VirtualLab/util/JSONSerializer.h"

namespace vl {

ClientModelSample::ClientModelSample(NetInterface* api, SOCKET sd, SOCKET usd, int modelSampleId, ClientSampleUpdateQueue* updateQueue) : api(api), sd(sd), usd(usd), modelSampleId(modelSampleId), updateQueue(updateQueue) {
    std::string params = api->receiveString(sd);
    std::string nav = api->receiveString(sd);
    std::string ds = api->receiveString(sd);
    JSONSerializer::instance().deserialize(params, parameters);
    JSONSerializer::instance().deserialize(nav, navigation);
    JSONSerializer::instance().deserialize(ds, data);
}

ClientModelSample::~ClientModelSample() {
    std::cout << "delete this craziness" << std::endl;
    updateQueue->removeSample(modelSampleId);
    //std::cout << "Sample Nullified" << std::endl;
    api->sendMessage(sd, MSG_deleteModelSample, (const unsigned char*)&modelSampleId, sizeof(int));
}

void ClientModelSample::update() {
    //unsigned char bytes[512];
    std::string nav = JSONSerializer::instance().serialize(navigation);
    ByteBufferWriter buf;
    buf.addData(modelSampleId);
    buf.addString(nav);
    api->sendMessage(sd, MSG_updateModelSample, buf.getBytes(), buf.getSize());
    int dataLength;
    api->receiveData(sd, (unsigned char*)& dataLength, sizeof(int));
    
    unsigned char* bytes = new unsigned char[dataLength];
    api->receiveData(sd, bytes, dataLength);
    ByteBufferReader reader(bytes);

    std::string ds;
    reader.readString(nav);
    reader.readString(ds);
    JSONSerializer::instance().deserialize(nav, navigation);
    JSONSerializer::instance().deserialize(ds, data);
    delete[] bytes;
}

void ClientModelSample::update(IUpdateCallback* callback) {
    //unsigned char bytes[512];

    /*std::string nav = JSONSerializer::instance().serialize(navigation);
    ByteBufferWriter buf;
    buf.addData(modelSampleId);
    buf.addString(nav);
    api->sendMessage(usd, MSG_updateModelSampleAsync, buf.getBytes(), buf.getSize());*/

    //int dataLength;
    //api->receiveData(usd, (unsigned char*)& dataLength, sizeof(int));

    updateQueue->scheduleForUpdate(modelSampleId, this, callback);

    //updateQueue->resolveUpdate();
    
    /*unsigned char* bytes = new unsigned char[dataLength];
    api->receiveData(usd, bytes, dataLength);
    ByteBufferReader reader(bytes);
    int sampleId;
    reader.readData(sampleId);

    resolveUpdate(reader, callback);

    delete[] bytes;*/
}

void ClientModelSample::resolveUpdate(ByteBufferReader& reader, IUpdateCallback* callback) {
    std::string ds;
    std::string nav;
    reader.readString(nav);
    reader.readString(ds);
    JSONSerializer::instance().deserialize(nav, navigation);
    JSONSerializer::instance().deserialize(ds, data);

    //std::cout << callback << " about to be called" << std::endl;
    callback->onComplete();
    //std::cout << callback << " called" << std::endl;
    delete callback;
    //std::cout << callback << " deleted" << std::endl;
}

void ClientSampleUpdateQueue::removeSample(int modelSampleId) {
  std::unique_lock<std::mutex> lock(updateMutex);
  if (callbacks[modelSampleId]) {
    delete callbacks[modelSampleId];
  }
  samples[modelSampleId] = NULL;
  callbacks[modelSampleId] = NULL;
}

void ClientSampleUpdateQueue::scheduleForUpdate(int modelSampleId, ClientModelSample* sample, IUpdateCallback* callback) {
  std::unique_lock<std::mutex> lock(updateMutex);
  std::string nav = JSONSerializer::instance().serialize(sample->getNavigation());
  ByteBufferWriter buf;
  buf.addData(modelSampleId);
  buf.addString(nav);
  api->sendMessage(usd, MSG_updateModelSampleAsync, buf.getBytes(), buf.getSize());
  //std::cout << callback << " Scheduled for update." << std::endl;

  samples[modelSampleId] = sample;
  callbacks[modelSampleId] = callback;
  waiting++;
  //std::cout << "Notify " << modelSampleId << " " << callbacks[modelSampleId] << std::endl;
  cond.notify_all();

  /*int dataLength;
  api->receiveData(usd, (unsigned char*)& dataLength, sizeof(int));
  unsigned char* bytes = new unsigned char[dataLength];
  api->receiveData(usd, bytes, dataLength);
  ByteBufferReader reader(bytes);
  int sampleId;
  reader.readData(sampleId);

  samples[sampleId]->resolveUpdate(reader, callbacks[modelSampleId]);*/
}

void ClientSampleUpdateQueue::update() {
  while(true) {
    std::unique_lock<std::mutex> lock(updateMutex);
    if (waiting == 0) {
      cond.wait(lock);
    }
    lock.unlock();
    resolveUpdate();
  }
}

void ClientSampleUpdateQueue::resolveUpdate() {
  //std::unique_lock<std::mutex> lock(updateMutex);
  //cond.wait(lock);
  int dataLength;
  api->receiveData(usd, (unsigned char*)& dataLength, sizeof(int));
  unsigned char* bytes = new unsigned char[dataLength];
  api->receiveData(usd, bytes, dataLength);
  ByteBufferReader reader(bytes);
  int sampleId;
  reader.readData(sampleId);

  std::unique_lock<std::mutex> lock(updateMutex);
  if (samples[sampleId] != NULL) {
    //std::cout << "Resolve " << sampleId << " " << callbacks[sampleId] << std::endl;
    samples[sampleId]->resolveUpdate(reader, callbacks[sampleId]);
    callbacks[sampleId] = NULL;
  }

  waiting--;
  lock.unlock();

  delete[] bytes;
}

ClientModel::ClientModel(NetInterface* api, SOCKET sd, SOCKET usd, const std::string& name, int modelId, ClientSampleUpdateQueue* updateQueue) : api(api), name(name), sd(sd), usd(usd), modelId(modelId), updateQueue(updateQueue) {
    std::string json = api->receiveString(sd);
    JSONSerializer::instance().deserialize(json, parameters);
}

IModelSample* ClientModel::create(const DataObject& params) {
    api->sendMessage(sd, MSG_createModelSample, (const unsigned char*)&modelId, sizeof(int));
    /*std::string json = api->receiveString(sd);
    DataObject ds;
    serializer.deserialize(json, ds);
    query.setParameters(ds);*/
    std::string json = JSONSerializer::instance().serialize(params);
    api->sendString(sd, json);
    int modelSampleId;
    api->receiveData(sd, (unsigned char*)&modelSampleId, sizeof(int));
    return new ClientModelSample(api, sd, usd, modelSampleId, updateQueue);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr2(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

Client::Client(const std::string &serverIP, int serverPort) {
  socketFD = createSocket(serverIP, serverPort);
  updateSocketFD = createSocket(serverIP, serverPort);
  updateQueue = new ClientSampleUpdateQueue(this, updateSocketFD);
}

SOCKET Client::createSocket(const std::string &serverIP, int serverPort)
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
    setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, &value, sizeof(value));

    return sockfd;

#endif

}


Client::~Client()
{
  for (std::map<int,IModel*>::iterator it = models.begin(); it != models.end(); it++) {
    delete it->second;
  }

  for (int i = 0; i < localModels.size(); i++) {
    delete localModels[i];
  }

  delete updateQueue;

  std::cout <<"Client closing socket." << std::endl;
#ifdef WIN32
  closesocket(socketFD);
  WSACleanup();
#else
  std::cout << "close socket" << std::endl;
  close(socketFD);
  close(updateSocketFD);
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
