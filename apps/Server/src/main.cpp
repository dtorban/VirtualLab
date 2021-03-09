/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include "VirtualLab/net/Server.h"

using namespace vl;

int main(int argc, char**argv) {
	Server server;
	while(true) {
		std::cout << server.getModels().size() << std::endl;
        server.service();
    }

	return 0;
}


