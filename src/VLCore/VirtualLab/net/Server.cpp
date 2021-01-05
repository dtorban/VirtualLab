#include "VirtualLab/net/Server.h"
#include "VirtualLab/util/JSONSerializer.h"

#include <sstream>
#include <iostream>

#define BACKLOG 100

namespace vl {

Server::Server(int listenPort, int numExpectedClients) {
  std::cout << "Starting networking" << std::endl;


#ifdef WIN32  // Winsock implementation

    WSADATA wsaData;
    int iResult;
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::stringstream s;
        s << "WSAStartup failed with error: " << iResult;
  		std::cout << s.str() << "Check for a problem with Windows networking." << std::endl;
        exit(1);
    }


    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
    
    // Resolve the server address and port
    struct addrinfo *result = NULL;
    std::stringstream port;
    port << listenPort;
    iResult = getaddrinfo(NULL, port.str().c_str(), &hints, &result);
    if ( iResult != 0 ) {
        std::stringstream s;
        s << "WSAStartup failed with error: " << iResult;
  		std::cout << s.str() << "Check for a problem with Windows networking." << std::endl;
        WSACleanup();
        exit(1);
    }
    

    SOCKET serv_fd = INVALID_SOCKET;
    serv_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serv_fd == INVALID_SOCKET) {
  		std::cout << "cannot create a socket socket() failed" << std::endl;
        WSACleanup();
        exit(1);
    }
    
    const char yes = 1;
    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
  		std::cout << "setsockopt() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        exit(1);
    }
    
    if (::bind(serv_fd, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
  		std::cout << "bind() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        closesocket(serv_fd);
        exit(1);
    }
    
    if (listen(serv_fd, BACKLOG) == SOCKET_ERROR) {
  		std::cout << "listen() failed. Check for a problem with networking." << std::endl;
        WSACleanup();
        exit(1);
    }

	serverSocketFD = serv_fd;
    
    std::cout << "listening for client connection(s) on port " << listenPort << "..." << std::endl;
    
    socklen_t client_len;
    struct sockaddr_in client_addr;
    SOCKET client_fd;
    
    int numConnected = 0;
    while (numConnected < numExpectedClients) {
        client_len = sizeof(client_addr);
        client_fd = accept(serv_fd, (struct sockaddr *) &client_addr, &client_len);
        if (client_fd == INVALID_SOCKET) {
  			std::cout << "accept() failed. Check for a problem with networking." << std::endl;
            WSACleanup();
            exit(1);
        }
        
        // Disable Nagle's algorithm on the client's socket
        setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
        
        numConnected++;
        
        char clientname[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, clientname, sizeof(clientname));
        
        std::stringstream s;
        s << "Received connection " << numConnected << " of " << numExpectedClients << " from " << clientname;
        std::cout << s.str() << std::endl;
        
        clientSocketFDs.push_back(client_fd);
    }

	std::cout << "Established all expected connections." << std::endl;


#else  // BSD sockets implementation

    int yes=1;
    int serv_fd;
    
    if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        std::cout << "cannot create a socket: socket(AF_INET, SOCK_STREAM, 0) failed" << std::endl;
        exit(1);
    }

    if (setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        std::cout << "setsockopt() failed: Check for a problem with networking." << std::endl;
        exit(1);
    }
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(listenPort);
    
    if (::bind(serv_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
  		std::cout << "bind() failed. Check for a problem with networking." << std::endl;
        close(serv_fd);
        exit(1);
    }

    if (listen(serv_fd, BACKLOG) == -1) {
  		std::cout << "listen() failed. Check for a problem with networking." << std::endl;
        exit(1);
    }

    std::cout << "listening for client connection(s) on port " << listenPort << "..." << std::endl;
    
    socklen_t client_len;
    struct sockaddr_un client_addr;
    int client_fd;

	std::cout << "Established all expected connections." << std::endl;

    serverSocketFD = serv_fd;

#endif
}

Server::~Server() {
	std::cout << "Closing all sockets." << std::endl;
	for (std::vector<SOCKET>::iterator i=clientSocketFDs.begin(); i < clientSocketFDs.end(); i++) {
	#ifdef WIN32
	  closesocket(*i);
	#else
	  close(*i);
	#endif
	}

	#ifdef WIN32
	  WSACleanup();
	#endif
}

class ServerModelSample : public IModelSample {
public:
    ServerModelSample(NetInterface* api, SOCKET sd, int modelSampleId) : api(api), sd(sd), modelSampleId(modelSampleId) {
        //sendString(socketFD, JSONSerializer::instance().serialize(modelSample->getNavigation()));
        //sendString(socketFD, JSONSerializer::instance().serialize(modelSample->getData()));
    }

    virtual ~ServerModelSample() {}

    IDataSet& getNavigation() {
    }

    const IDataSet& getData() const {
    }

    void update() {
    }

private:
    TypedData<std::string> navigation;
    NetInterface* api;
    SOCKET sd;
    int modelSampleId;
};

class ServerQuery : public IQuery {
public:
    ServerQuery(NetInterface* api, SOCKET sd) : api(api), sd(sd) {}

    virtual void setParameters(IDataSet& params, DataSetStack& context) const {
        std::string s = serializer.serialize(params);
        api->sendString(sd, s);
        s = api->receiveString(sd);
        serializer.deserialize(s, params);
    }

private:
    NetInterface* api;
    SOCKET sd;
    JSONSerializer serializer;
};

class ServerModel : public IModel {
public:
    ServerModel(NetInterface* api, SOCKET sd, const std::string& name, int modelId) : api(api), name(name), sd(sd), modelId(modelId) {}

    const std::string& getName() const { return name; }
    virtual IModelSample* create(const IQuery& query) const {
        api->sendMessage(sd, MSG_createModelSample, (const unsigned char*)&modelId, sizeof(int));

        // query
        std::string json = api->receiveString(sd);
        CompositeDataSet ds;
        serializer.deserialize(json, ds);
        json = serializer.serialize(ds);
        query.setParameters(ds);
        api->sendString(sd, json);

        int modelSampleId;
        return new ServerModelSample(api, sd, modelSampleId);
    }

private:
    NetInterface* api;
    std::string name;
    SOCKET sd;
    int modelId;
    JSONSerializer serializer;
};

void Server::service() {
    //Adapted from https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
    //clear the socket set  
    FD_ZERO(&readfds);   
 
    //add master socket to set  
    FD_SET(serverSocketFD, &readfds);   
    int max_sd = serverSocketFD;   
         
    //add child sockets to set  
    for (int f = 0; f < clientSocketFDs.size(); f++) {
        //socket descriptor  
        int sd = clientSocketFDs[f]; 

        //if valid socket descriptor then add to read list  
        if (sd != 0) {
            FD_SET(sd , &readfds); 
        }

        //highest file descriptor number, need it for the select function   
        if(sd > max_sd) {
            max_sd = sd;
        }
    }
 
    //wait for an activity on one of the sockets , timeout is NULL ,  
    //so wait indefinitely  
#ifdef WIN32
	/*struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 200;
	int activity = select(0, &readfds, NULL, NULL, &timeout);
	if (activity == 0) {
		return;
	}*/
	int activity = select(0, &readfds, NULL, NULL, NULL);
#else
	int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
#endif
   
    if ((activity < 0) && (errno!=EINTR)) {   
        printf("select error");   
    }   
         
    //If something happened on the master socket ,  
    //then its an incoming connection  
    if (FD_ISSET(serverSocketFD, &readfds)) {   
#ifdef WIN32
		struct sockaddr_in client_addr;
#else
        struct sockaddr_un client_addr;
#endif
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(serverSocketFD, (struct sockaddr *) &client_addr, &client_len);
#ifdef WIN32
		if (client_fd == INVALID_SOCKET) {
#else
        if (client_fd == -1) {
#endif
            std::cout << "accept() failed. Check for a problem with networking." << std::endl;
            exit(1);
        }
        
        std::cout << "New Connection." << std::endl; 
       
        bool foundReplacement = false;
        for (int f = 0; f < clientSocketFDs.size(); f++) {
            if (clientSocketFDs[f] == 0) {
                clientSocketFDs[f] = client_fd;
                foundReplacement = true;
                break;
            }
        }
        if (!foundReplacement) {
            clientSocketFDs.push_back(client_fd);  
        }
    }

    //else its some IO operation on some other socket 
    for (int f = 0; f < clientSocketFDs.size(); f++) {
        //socket descriptor  
        int sd = clientSocketFDs[f]; 

        if (FD_ISSET(sd , &readfds)) {
            //Check if it was for closing , and also read the  
            //incoming message  
            int dataLength;
            //char buffer[1025]; 
            NetMessageType messageType = receiveMessage(sd, dataLength);
            if (messageType == MSG_none) { //(valread = read( sd , buffer, 1025)) == 0) {   
                //Somebody disconnected , get his details and print  
#ifdef WIN32
				struct sockaddr_in client_addr;
#else
				struct sockaddr_un client_addr;
#endif
                socklen_t client_len;
                getpeername(sd , (struct sockaddr*)&client_addr , (socklen_t*)&client_len);   
                std::cout <<"Host disconnected " << std::endl;   
                clientSocketFDs[f] = 0;
            }
            else if (messageType == MSG_registerModel) {
                unsigned char* buf = new unsigned char[dataLength+1];
                receiveData(sd, buf, dataLength);
                int modelId;
                receiveData(sd, (unsigned char*)& modelId, sizeof(int));
                buf[dataLength] = '\0';
                std::string val(reinterpret_cast<char*>(buf));
                registerModel(new ServerModel(this, sd, val, modelId));
                delete[] buf;
            }
            else if (messageType == MSG_getModels) {
                std::vector<IModel*> models = getModels();
                int numModels = models.size();
                sendData(sd, (unsigned char*)& numModels, sizeof(int));
                for (int i = 0; i < numModels; i++) {
                    std::string name = models[i]->getName();
                    int nameSize = name.size();
                    sendData(sd, (unsigned char*)& nameSize, sizeof(int));
                    sendData(sd, (const unsigned char*)name.c_str(), name.size());
                    int modelId = i;
                    sendData(sd, (unsigned char*)& modelId, sizeof(int));
                    //std::cout << name << std::endl;
                }
            }
            else if (messageType == MSG_createModelSample) {
                int modelId;
                receiveData(sd, (unsigned char*)& modelId, sizeof(int));
                ServerQuery q(this, sd);
                IModelSample* sample = getModels()[modelId]->create(q);
                static int modelSamplesCount = 0;
                int modelSampleId = modelSamplesCount;
                modelSamplesCount++;
                serverModelSamples[modelSampleId] = sample;
                sendData(sd, (unsigned char*)& modelSampleId, sizeof(int));
                sendString(sd, JSONSerializer::instance().serialize(sample->getNavigation()));
                sendString(sd, JSONSerializer::instance().serialize(sample->getData()));
            }
            else if (messageType == MSG_updateModelSample) {
                JSONSerializer serializer;
                int modelSampleId;
                receiveData(sd, (unsigned char*)& modelSampleId, sizeof(int));
                IModelSample* sample = serverModelSamples[modelSampleId];
                std::string nav = receiveString(sd);
                serializer.deserialize(nav, sample->getNavigation());
                sample->update();
                sendString(sd, JSONSerializer::instance().serialize(sample->getNavigation()));
                sendString(sd, JSONSerializer::instance().serialize(sample->getData()));
            }
            else if (messageType == MSG_deleteModelSample) {
                int modelSampleId;
                receiveData(sd, (unsigned char*)& modelSampleId, sizeof(int));
                std::map<int, IModelSample*>::iterator it = serverModelSamples.find(modelSampleId);
                delete it->second;
                serverModelSamples.erase(it);
            }
        }

    }
}

}
