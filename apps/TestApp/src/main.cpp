/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include "VirtualLab/net/Client.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

int main(int argc, char**argv) {
	Client api;
	IModel* model = api.getModels()[0];
	
	DataObject params = model->getParameters();
	IModelSample* sample = model->create(params);
	std::cout << "Parameters: " << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << "Navigation: " << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << "Data: " << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	sample->getNavigation()["m"].set<double>(1);

	for (int i = 0; i < 100; i++) {
		sample->getNavigation()["t"].set<double>(0.1*i);
		sample->update();
		std::cout << "t=" << sample->getNavigation()["t"].get<double>() << ": ";
		std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
	}

	return 0;
}


