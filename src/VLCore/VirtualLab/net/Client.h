#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#include <string>
#include <iostream>

#include "VirtualLab/net/NetInterface.h"
#include "VirtualLab/IVirtualLabAPI.h"
#include "VirtualLab/util/ByteBuffer.h"

#include <thread>
#include <mutex>
#include <condition_variable>


namespace vl {

class Client;

/*class ClientQuery : public IQuery {
public:
    ClientQuery(NetInterface* api, SOCKET sd) : api(api), sd(sd) {}

    virtual void setParameters(DataObject& params, DataObjectStack& context) const {
        std::string s = serializer.serialize(params);
        api->sendString(sd, s);
        s = api->receiveString(sd);
        serializer.deserialize(s, params);
    }

private:
    NetInterface* api;
    SOCKET sd;
    JSONSerializer serializer;
};*/

class ClientSampleUpdateQueue;

class ClientModelSample : public IModelSample {
public:
    ClientModelSample(NetInterface* api, SOCKET sd, SOCKET usd, int modelSampleId, ClientSampleUpdateQueue* updateQueue);

    ~ClientModelSample();

    virtual const DataObject& getParameters() const {
        return parameters;
    }

    virtual DataObject& getNavigation() {
        return navigation;
    }

    virtual const DataObject& getData() const {
        return data;
    }

    virtual void update();

    void update(IUpdateCallback* callback);

    void resolveUpdate(ByteBufferReader& reader, IUpdateCallback* callback);

private:
    DataObject parameters;
    DataObject navigation;
    DataObject data;
    NetInterface* api;
    SOCKET sd;
    SOCKET usd;
    int modelSampleId;
    ClientSampleUpdateQueue* updateQueue;
};

class ClientSampleUpdateQueue {
public:
    ClientSampleUpdateQueue(NetInterface* api, SOCKET usd) : api(api), usd(usd), waiting(0), running(true) {
        thread = new std::thread(&ClientSampleUpdateQueue::update, this);
    }
    virtual ~ClientSampleUpdateQueue() { 
        std::unique_lock<std::mutex> lock(updateMutex);
        running = false;
        std::cout << "Notify all" << std::endl;
        while (waiting) {
            cond.notify_all();
        }

        std::cout << "Join / delete" << std::endl;
        thread->join();
        delete thread;
    }

    void scheduleForUpdate(int modelSampleId, ClientModelSample* sample, IUpdateCallback* callback);
    void removeSample(int modelSampleId);
    void resolveUpdate();
    void update();

private:
    int waiting;
    bool running;
    std::map<int, ClientModelSample*> samples;
    std::map<int, IUpdateCallback*> callbacks;
    NetInterface* api;
    SOCKET usd;
    std::thread* thread;
    std::mutex updateMutex;
    std::condition_variable cond;
};

class ClientModel : public IModel {
public:
    ClientModel(NetInterface* api, SOCKET sd, SOCKET usd, const std::string& name, int modelId, ClientSampleUpdateQueue* updateQueue);

    const std::string& getName() const { return name; }
    virtual const DataObject& getParameters() { return parameters; }
    virtual IModelSample* create(const DataObject& params);

private:
    NetInterface* api;
    std::string name;
    SOCKET sd;
    SOCKET usd;
    int modelId;
    DataObject parameters;
    ClientSampleUpdateQueue* updateQueue;
};

class ExternalModel : public IModel {
public:
    ExternalModel(NetInterface* api, SOCKET sd, const std::string& name, int modelId) : api(NULL), name(name), modelId(modelId) {
        ip = api->receiveString(sd);
	ip = "127.0.0.1";
        api->receiveData(sd, (unsigned char*)&port, sizeof(int));
        std::cout << ip << ":" << port << std::endl;
    }
    ~ExternalModel() { delete api; }

    const std::string& getName() const { return name; }

    virtual const DataObject& getParameters() { 
        lazyLoadApi();
        return parameters;
    }
    
    virtual IModelSample* create(const DataObject& params) {
        lazyLoadApi();
        return model.create(params);
    }

private:
    void lazyLoadApi();
    ModelProxy model;

    IVirtualLabAPI* api;
    std::string name;
    int modelId;
    DataObject parameters;
    std::string ip;
    int port;
};

class RemoteModel : public IModel {
public:
    RemoteModel(int serverPort, IModel* model) : serverPort(serverPort), model(model) {}

    const std::string& getName() const { return model->getName(); }
    const DataObject& getParameters() { return model->getParameters(); }
    IModelSample* create(const DataObject& params) {
        return model->create(params);
    }

    int getPort() { return serverPort; }

private:
    int serverPort;
    IModel* model;
};

class Client : public NetInterface, public IVirtualLabAPI  {
public:
	Client(const std::string &serverIP = "127.0.0.1", int serverPort = 3457);
	~Client();

    virtual void registerModel(IModel* model) {
        RemoteModel* remoteModel = dynamic_cast<RemoteModel*>(model);
        if (remoteModel) {
            std::string name = model->getName();
            sendMessage(socketFD, MSG_registerModel, (const unsigned char*)name.c_str(), name.size());
            int modelId = 0;//localModels.size() - 1;
            sendData(socketFD, (unsigned char*)& modelId, sizeof(int));
            //sendString(socketFD, remoteModel->getIP());
            int port = remoteModel->getPort();
            sendData(socketFD, (unsigned char*)& port, sizeof(int));
            //std::cout << remoteModel->getIP() << ":" << remoteModel->getPort() << std::endl;
        }

        localModels.push_back(model);
    }
    virtual void deregisterModel(IModel* model) {
        std::string name = model->getName();
        sendMessage(socketFD, MSG_deregisterModel, (const unsigned char*)name.c_str(), name.size());
    }
    virtual const std::vector<ModelProxy> getModels() { 
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
            int modelType;
            receiveData(socketFD, (unsigned char*)& modelType, sizeof(int));
            std::cout << name << " " << modelId << std::endl;

            IModel* tempModel = NULL;
            if (modelType == 1) {
                tempModel = new ExternalModel(this, socketFD, name, modelId);
            }
            else {
                tempModel = new ClientModel(this, socketFD, updateSocketFD, name, modelId, updateQueue);
            }

            if (models.find(modelId) == models.end()) {
                models[modelId] = tempModel;
            }
            else {
                delete tempModel;
            }

        }

        std::vector<ModelProxy> modelProxies;
        for (std::map<int,IModel*>::iterator it = models.begin(); it != models.end(); it++) {
            modelProxies.push_back(ModelProxy(it->second));
        }

        for (int i = 0; i < localModels.size(); i++) {
            modelProxies.push_back(ModelProxy(localModels[i]));
        }
        
        return modelProxies;
    }

    /*virtual void service() {
        int len;
        NetMessageType type = receiveMessage(socketFD, len);
        std::cout << type << " " << len << std::endl;
        if (type == MSG_createModelSample) {
            std::cout << "it appears to work." << std::endl;
            int modelId;
            receiveData(socketFD, (unsigned char*)& modelId, sizeof(int));
            std::cout << modelId << std::endl;

            ClientQuery q(this, socketFD);
            IModelSample* modelSample = localModels[modelId]->create(q);
            localModelSamples.push_back(modelSample);

            std::cout << "go to stable" << std::endl;
            int modelSampleId = localModelSamples.size() - 1;
            sendData(socketFD, (const unsigned char*)&modelSampleId, sizeof(int));
            std::cout << "sent to stable" << std::endl;
            //sendString(socketFD, JSONSerializer::instance().serialize(modelSample->getNavigation()));
            //sendString(socketFD, JSONSerializer::instance().serialize(modelSample->getData()));
        }

    }*/

    std::map<int,IModel*> models;
    std::vector<IModel*> localModels;
    ClientSampleUpdateQueue* updateQueue;
    //std::vector<IModelSample*> localModelSamples;

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
    SOCKET createSocket(const std::string &serverIP, int serverPort);

	//std::vector<MessageQueue*> messageQueues;
	SOCKET socketFD;
	SOCKET updateSocketFD;
};

inline void ExternalModel::lazyLoadApi() {
    if (!api) {
        api = new Client(ip, port);
        std::vector<ModelProxy> models = api->getModels();
        for (int i = 0; i < models.size(); i++) {
            if (name == models[i].getName()) {
                model = models[i];
                parameters = model.getParameters();
                break;
            }
        }
    }
}

}

#endif
