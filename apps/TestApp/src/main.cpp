/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include "VirtualLab/net/Server.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

int main(int argc, char**argv) {
	VirtualLabAPI api;
	api.registerModel(new TestModel());
	IModel* model = api.getModels()[0];
	
	DefaultQuery query;
	IModelSample* sample = model->create(query);
	std::cout << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	for (int i = 0; i < 10; i++) {
		sample->getNavigation()["time"].set<double>(0.1*i);
		sample->update();
		std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
		std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
	}

	return 0;
}


