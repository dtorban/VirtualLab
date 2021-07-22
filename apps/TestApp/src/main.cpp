/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <cmath>
#include "VirtualLab/impl/VirtualLabAPI.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"
//#include "VirtualLab/pca/PCAModel.h"

using namespace vl;

/*class TestUpdateCallback : public IUpdateCallback {
public:
	TestUpdateCallback(IModelSample* sample) : sample(sample) {}
	virtual ~TestUpdateCallback() {}

	void onComplete() {
	}

private:
	IModelSample* sample;
};*/


int main(int argc, char**argv) { 

	//Client client;
	//LoadBalancedAPI api(&client);
	Client api;

	std::vector<ModelProxy> models = api.getModels();
	for (int f = 0; f < models.size(); f++) {
		std::cout << f << ": " << models[f].getName() << std::endl;
	}

	ModelProxy proxy = 	api.getModels()[4];	
	IModel* model = &proxy;
	//model = new PCAModel("Test", model);
	std::cout << model->getName() << std::endl;
		
	DataObject params = model->getParameters();
	params["N"].set<double>(10);
	IModelSample* sample = model->create(params);
	
	std::cout << "Parameters: " << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << "Navigation: " << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << "Data: " << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	sample->getNavigation()["m"].set<double>(1);

	for (int i = 0; i < 10; i++) {
		if (i == 9) {
			sample->getNavigation()["h"].set<double>(1);
		}
		sample->getNavigation()["t"].set<double>(10.0*i + 10.0);
		sample->update();
		std::cout << "t=" << sample->getNavigation()["t"].get<double>() << ": ";
		std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
		std::cout << std::endl;
	}

	delete sample;

	exit(0);

	return 0;
}


