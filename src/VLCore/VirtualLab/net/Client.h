#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#include <string>
#include <iostream>

#include "VirtualLab/net/NetInterface.h"
#include "VirtualLab/IVirtualLabAPI.h"

namespace vl {

/*class ClientMessageQueue : public MessageQueue {
public:
	ClientMessageQueue(NetInterface* net, SOCKET socketFD, int id);
	virtual ~ClientMessageQueue() {}
	int getId();
	void waitForMessage();
	void sendMessage();
	int sendData(const unsigned char *buf, int len);
	int receiveData(unsigned char *buf, int len);
private:
	NetInterface* net;
	SOCKET socketFD;
	int id;
};*/

class ClientModelSample : public IModelSample {
public:
    ClientModelSample(NetInterface* api, SOCKET sd, int modelSampleId) : api(api), sd(sd), modelSampleId(modelSampleId) {
    }

    virtual IDataSet& getNavigation() {
    }

    virtual const IDataSet& getData() const {
    }

    virtual void update() {
    }

private:
    CompositeDataSet navigation;
    CompositeDataSet data;
    NetInterface* api;
    SOCKET sd;
    int modelSampleId;
};

class ClientModel : public IModel {
public:
    ClientModel(NetInterface* api, SOCKET sd, const std::string& name, int modelId) : api(api), name(name), sd(sd), modelId(modelId) {}

    const std::string& getName() const { return name; }
    virtual IModelSample* create(const IQuery& query) const {
        api->sendMessage(sd, MSG_createModelSample, (const unsigned char*)&modelId, sizeof(int));
    }

private:
    NetInterface* api;
    std::string name;
    SOCKET sd;
    int modelId;
};

class Client : public NetInterface, public IVirtualLabAPI  {
public:
	Client(const std::string &serverIP = "127.0.0.1", int serverPort = 3457);
	~Client();

    virtual void registerModel(IModel* model) {
        localModels.push_back(model);
        std::string name = model->getName();
        sendMessage(socketFD, MSG_registerModel, (const unsigned char*)name.c_str(), name.size());
        int modelId = localModels.size() - 1;
        sendData(socketFD, (unsigned char*)& modelId, sizeof(int));
    }
    virtual void deregisterModel(IModel* model) {
        std::string name = model->getName();
        sendMessage(socketFD, MSG_deregisterModel, (const unsigned char*)name.c_str(), name.size());
    }
    virtual const std::vector<IModel*>& getModels() { 
        for (int i = 0; i < models.size(); i++) {
            delete models[i];
        }
        models.clear();

        sendMessage(socketFD, MSG_getModels, (const unsigned char*)0, 0);
        int numModels;
        receiveData(socketFD, (unsigned char*)& numModels, sizeof(int));
        for (int i = 0; i < numModels; i++) {
            int dataLength;
            receiveData(socketFD, (unsigned char*)& dataLength, sizeof(int));
            unsigned char* buf = new unsigned char[dataLength+1];
            receiveData(socketFD, buf, dataLength);
            buf[dataLength] = '\0';
            std::string name(reinterpret_cast<char*>(buf));
            delete[] buf;
            int modelId;
            receiveData(socketFD, (unsigned char*)& modelId, sizeof(int));
            //std::cout << name << " " << modelId << std::endl;
            models.push_back(new ClientModel(this, socketFD, name, modelId));
        }
        
        return models;
    }

    virtual void waitForMessage() {
        int len;
        NetMessageType type = receiveMessage(socketFD, len);
        std::cout << type << " " << len << std::endl;
        if (type == MSG_createModelSample) {
            std::cout << "it appears to work." << std::endl;
            int modelId;
            receiveData(socketFD, (unsigned char*)& modelId, sizeof(int));
            std::cout << modelId << std::endl;

            DefaultQuery q;
            localModels[modelId]->create(q);
        }

    }

    std::vector<IModel*> models;
    std::vector<IModel*> localModels;

/*	virtual void createSharedTexture(const std::string& name, const TextureInfo& info, int deviceIndex) {
	    sendMessage(socketFD, MSG_createSharedTexture, (const unsigned char*)name.c_str(), name.size());
	    sendData(socketFD, (unsigned char*)& deviceIndex, sizeof(int));
	    sendData(socketFD, (unsigned char*)& info, sizeof(TextureInfo));
	}

	virtual Texture getSharedTexture(const std::string& name, int deviceIndex) { 
	    sendMessage(socketFD, MSG_getSharedTexture, (const unsigned char*)name.c_str(), name.size());
	    sendData(socketFD, (unsigned char*)& deviceIndex, sizeof(int));
	    Texture tex;
#ifdef WIN32
		//HANDLE currentProcess = GetCurrentProcess();
		//sendData(socketFD, (unsigned char*)&currentProcess, sizeof(HANDLE));
		int pid = GetCurrentProcessId();
		receiveData(socketFD, (unsigned char*)& pid, sizeof(int));
		std::cout << "Pid client: " << pid << std::endl;
        receiveData(socketFD, (unsigned char*)& tex, sizeof(tex));

        HANDLE serverProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
		tex.externalHandle = getExternalHandle(serverProcess, tex.externalHandle);
		for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
			tex.externalSemaphores[f] = getExternalHandle(serverProcess, tex.externalSemaphores[f]);
		}
		//tex.externalHandle = 0;
#else
		int fd = NetInterface::recvfd(socketFD);
		int semaphoreFDs[NUM_TEXTURE_SEMAPHORES];
		for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
			semaphoreFDs[f] = NetInterface::recvfd(socketFD);
		}
		receiveData(socketFD, (unsigned char*)& tex, sizeof(tex));
		tex.externalHandle = fd;
		for (int f = 0; f < NUM_TEXTURE_SEMAPHORES; f++) {
			tex.externalSemaphores[f] = semaphoreFDs[f];
		}
#endif
		return tex;
	}

#ifdef WIN32
	HANDLE getExternalHandle(HANDLE serverProcess, HANDLE externalHandle) {
		HANDLE externalHandleDup;
		std::cout << externalHandle << " eh1" << std::endl;
		DuplicateHandle(serverProcess,
			externalHandle,
			GetCurrentProcess(),
			&externalHandleDup,
			0,
			FALSE,
			DUPLICATE_SAME_ACCESS);
		std::cout << externalHandleDup << " eh2" << std::endl;
		return externalHandleDup;
	}
#endif

	MessageQueue* getMessageQueue(const std::string& name) { 
		sendMessage(socketFD, MSG_getMessageQueue, (const unsigned char*)name.c_str(), name.size());
		int queueId;
		receiveData(socketFD, (unsigned char*)& queueId, sizeof(queueId));
		MessageQueue* queue = new ClientMessageQueue(this, socketFD, queueId);
		messageQueues.push_back(queue);
		return queue;
	}

	virtual Semaphore getSemaphore(const std::string& name, int deviceIndex) { 
	    sendMessage(socketFD, MSG_getSemaphore, (const unsigned char*)name.c_str(), name.size());
	    sendData(socketFD, (unsigned char*)& deviceIndex, sizeof(int));
	    Semaphore sem;
#ifdef WIN32
		int pid = GetCurrentProcessId();
		receiveData(socketFD, (unsigned char*)& pid, sizeof(int));
        receiveData(socketFD, (unsigned char*)& sem, sizeof(sem));

        HANDLE serverProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pid);
		sem.externalHandle = getExternalHandle(serverProcess, sem.externalHandle);
#else
		int fd = NetInterface::recvfd(socketFD);
		int semaphoreFDs[NUM_TEXTURE_SEMAPHORES];
		receiveData(socketFD, (unsigned char*)& sem, sizeof(sem));
		sem.externalHandle = fd;
#endif
		return sem;
	}*/

private:
	//std::vector<MessageQueue*> messageQueues;
	SOCKET socketFD;
};

}

#endif
