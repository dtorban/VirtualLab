/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <cmath>
#include "VirtualLab/impl/VirtualLabAPI.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

int main(int argc, char**argv) {

	Client client;
	LoadBalancedAPI api(&client);

	std::vector<IModel*> models = api.getModels();
	for (int f = 0; f < models.size(); f++) {
		std::cout << models[f]->getName() << std::endl;
	}

			
	IModel* model = api.getModels()[0];
	std::cout << model->getName() << std::endl;
		

	while (true) {
		int i;
		std::cin >> i;

		DataObject params = model->getParameters();
		params["N"].set<double>(1);
		IModelSample* sample = model->create(params);
		
		std::cout << "Parameters: " << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
		std::cout << "Navigation: " << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
		std::cout << "Data: " << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

		sample->getNavigation()["m"].set<double>(1);

		for (int i = 0; i < 10; i++) {
			sample->getNavigation()["t"].set<double>(1.0*i + 1.0);
			sample->update();
			std::cout << "t=" << sample->getNavigation()["t"].get<double>() << ": ";
			std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
			std::cout << std::endl;

		}


	}

	return 0;
}


