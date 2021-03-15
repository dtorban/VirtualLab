#ifndef VISLINK_CLIENT_H_
#define VISLINK_CLIENT_H_

#include <string>
#include <iostream>

#include "VirtualLab/net/NetInterface.h"
#include "VirtualLab/IVirtualLabAPI.h"
#include "VirtualLab/util/JSONSerializer.h"
#include "VirtualLab/util/ByteBuffer.h"


namespace vl {

class ClientQuery : public IQuery {
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
};

class ClientModelSample : public IModelSample {
public:
    ClientModelSample(NetInterface* api, SOCKET sd, int modelSampleId) : api(api), sd(sd), modelSampleId(modelSampleId) {
        std::string params = api->receiveString(sd);
        std::string nav = api->receiveString(sd);
        std::string ds = api->receiveString(sd);
        serializer.deserialize(params, parameters);
        serializer.deserialize(nav, navigation);
        serializer.deserialize(ds, data);
    }

    ~ClientModelSample() {
        std::cout << "delete this craziness" << std::endl;
        api->sendMessage(sd, MSG_deleteModelSample, (const unsigned char*)&modelSampleId, sizeof(int));
    }

    virtual const DataObject& getParameters() const {
        return parameters;
    }

    virtual DataObject& getNavigation() {
        return navigation;
    }

    virtual const DataObject& getData() const {
        return data;
    }

    virtual void update() {
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
        serializer.deserialize(nav, navigation);
        serializer.deserialize(ds, data);
        delete[] bytes;
    }

private:
    DataObject parameters;
    DataObject navigation;
    DataObject data;
    NetInterface* api;
    SOCKET sd;
    int modelSampleId;
    JSONSerializer serializer;
};

class ClientModel : public IModel {
public:
    ClientModel(NetInterface* api, SOCKET sd, const std::string& name, int modelId) : api(api), name(name), sd(sd), modelId(modelId) {
        std::string json = api->receiveString(sd);
        serializer.deserialize(json, parameters);
    }

    const std::string& getName() const { return name; }
    /*virtual IModelSample* create(const IQuery& query) const {
        api->sendMessage(sd, MSG_createModelSample, (const unsigned char*)&modelId, sizeof(int));
        std::string json = api->receiveString(sd);
        DataObject ds;
        serializer.deserialize(json, ds);
        query.setParameters(ds);
        json = serializer.serialize(ds);
        api->sendString(sd, json);
        int modelSampleId;
        api->receiveData(sd, (unsigned char*)&modelSampleId, sizeof(int));
        return new ClientModelSample(api, sd, modelSampleId);
    }*/
    virtual const DataObject& getParameters() { return parameters; }
    virtual IModelSample* create(const DataObject& params) {
        api->sendMessage(sd, MSG_createModelSample, (const unsigned char*)&modelId, sizeof(int));
        /*std::string json = api->receiveString(sd);
        DataObject ds;
        serializer.deserialize(json, ds);
        query.setParameters(ds);*/
        std::string json = serializer.serialize(params);
        api->sendString(sd, json);
        int modelSampleId;
        api->receiveData(sd, (unsigned char*)&modelSampleId, sizeof(int));
        return new ClientModelSample(api, sd, modelSampleId);
    }

private:
    NetInterface* api;
    std::string name;
    SOCKET sd;
    int modelId;
    JSONSerializer serializer;
    DataObject parameters;
};

class ExternalModel : public IModel {
public:
    ExternalModel(NetInterface* api, SOCKET sd, const std::string& name, int modelId) : api(NULL), name(name), modelId(modelId) {
        ip = api->receiveString(sd);
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
        return model->create(params);
    }

private:
    void lazyLoadApi();
    IModel* model;

    IVirtualLabAPI* api;
    std::string name;
    int modelId;
    JSONSerializer serializer;
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

        //localModels.push_back(model);
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
            int modelType;
            receiveData(socketFD, (unsigned char*)& modelType, sizeof(int));
            //std::cout << name << " " << modelId << std::endl;

            if (modelType == 1) {
                models.push_back(new ExternalModel(this, socketFD, name, modelId));
            }
            else {
                models.push_back(new ClientModel(this, socketFD, name, modelId));
            }
        }
        
        return models;
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

    std::vector<IModel*> models;
    //std::vector<IModel*> localModels;
    std::vector<IModelSample*> localModelSamples;

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

inline void ExternalModel::lazyLoadApi() {
    api = new Client(ip, port);
    std::vector<IModel*> models = api->getModels();
    for (int i = 0; i < models.size(); i++) {
        if (name == models[i]->getName()) {
            model = models[i];
            parameters = model->getParameters();
            break;
        }
    }
}

}

#endif
