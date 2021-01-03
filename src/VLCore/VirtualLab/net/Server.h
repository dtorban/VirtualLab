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
    }

    virtual void deregisterModel(IModel* model) {
        impl.deregisterModel(model);
    }

    virtual const std::vector<IModel*>& getModels() {
        return impl.getModels();
    }

	void service();

private:

	SOCKET serverSocketFD;
	std::vector<SOCKET> clientSocketFDs;
	VirtualLabAPI impl;
	fd_set readfds;
    std::vector<IModelSample*> serverModelSamples;
};

}

#endif