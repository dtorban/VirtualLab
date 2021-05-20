#ifndef VIRTUALLAB_NET_SERVER_H_
#define VIRTUALLAB_NET_SERVER_H_

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "VirtualLab/net/NetInterface.h"
#include "VirtualLab/impl/VirtualLabAPI.h"

namespace vl {

class Server : public NetInterface, public IVirtualLabAPI {
public:
	Server(int listenPort = 3457, int numExpectedClients = 0);
	~Server();

    virtual void registerModel(IModel* model) {
        impl.registerModel(model);
        models.push_back(model);
    }

    virtual void deregisterModel(IModel* model) {
        impl.deregisterModel(model);
        //TODO: deregister model
    }

    virtual const std::vector<ModelProxy> getModels() {
        return impl.getModels();
    }

	void service();

private:

	SOCKET serverSocketFD;
	std::vector<SOCKET> clientSocketFDs;
	std::vector<std::string> clientSocketIPs;
	std::vector< std::vector<IModel*> > clientModels;
	VirtualLabAPI impl;
	fd_set readfds;
    std::vector<IModel*> models;
    std::map<int, IModelSample*> serverModelSamples;
};

}

#endif