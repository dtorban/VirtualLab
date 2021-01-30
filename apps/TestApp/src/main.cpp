/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

int main(int argc, char**argv) {

	int pid = fork();
	if (pid != 0) {
		Server server;
		server.registerModel(new TestModel("ModelA"));
		server.registerModel(new TestModel("ModelB"));
		while(true) {
			server.service();
		}
		return 0;
	}


	Client api;
	//VirtualLabAPI api;
	//api.registerModel(new TestModel());
	IModel* model = api.getModels()[0];
	
	DefaultQuery query;
	IModelSample* sample = model->create(query);
	std::cout << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	std::string str = JSONSerializer::instance().serialize(sample->getParameters());
	DataValue d;
	JSONSerializer::instance().deserialize(str, d);
	std::cout << str << " " << JSONSerializer::instance().serialize(d) << std::endl;

	for (int i = 1; i < 1000; i++) {
		sample->getNavigation()["time"].set<double>(0.1*i);
		sample->update();
		//std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
		std::cout << i << " " <<  JSONSerializer::instance().serialize(sample->getData()) << std::endl;
	}

	return 0;
}


