#ifndef VIRTUALLAB_NET_SERVER_H_
#define VIRTUALLAB_NET_SERVER_H_

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "VirtualLab/net/NetInterface.h"
#include "VirtualLab/impl/VirtualLabAPI.h"

namespace vl {

class ServerMessageQueue {
public:
	ServerMessageQueue(NetInterface* net, int id) : net(net), id(id), currentMessageId(0), sending(false) {}
	virtual ~ServerMessageQueue() {}
	int getId() { return id; }
	void pushClient(SOCKET clientFD);
	void pushMessage();
	void pushMessageData(SOCKET sd);
private:
	struct Message {
		Message(int id) : id(id), data(NULL) {}
		bool isStart() { return !data; }

		int id;
		unsigned char* data;
		int len;
	};

	void sendData();

	NetInterface* net;
	std::queue<SOCKET> clientWaitQueue;
	std::queue<Message> sendDataQueue;
	int id;
	int currentMessageId;
	bool sending;
};

class Server : public NetInterface, public IVirtualLabAPI {
public:
	Server(int listenPort = 3457, int numExpectedClients = 0);
	~Server();

    virtual void registerModel(IModel* model) {
        impl.registerModel(model);
    }

    virtual void deregisterModel(IModel* model) {
        impl.deregisterModel(model);
    }
    
    virtual const std::vector<IModel*>& getModels() const {
        return impl.getModels();
    }

	void service();

private:
	ServerMessageQueue* getServerMessageQueue(const std::string& name) { 
		std::map<std::string,int>::iterator it = messageQueueMap.find(name);
		int id = messageQueues.size();
		if (it != messageQueueMap.end()) {
			id = it->second;
		}
		else {
			ServerMessageQueue* queue = new ServerMessageQueue(this, id);
			messageQueueMap[name] = id;
			messageQueues.push_back(queue);
		}
		return messageQueues[id];
	}

	ServerMessageQueue* getQueueFromMessage(SOCKET sd);

	SOCKET serverSocketFD;
	std::vector<SOCKET> clientSocketFDs;
	VirtualLabAPI impl;
	fd_set readfds;
	std::map<std::string, int> messageQueueMap;
	std::vector<ServerMessageQueue*> messageQueues;
};

}

#endif