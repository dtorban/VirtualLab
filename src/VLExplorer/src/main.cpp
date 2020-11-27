/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
//#define _USE_MATH_DEFINES
//#include <cmath>
//#include <libwebsockets.h>
#include "WebServer.h" 
//#include "VirtualLab/Ensemble.h"
//#include "VirtualLab/impl/X2.h"
#include "VirtualLab/impl/TestModel.h"

class VLWebServerSession;
class VLWebServerCommand;

struct VLWebServerSessionState {
	std::map<std::string, VLWebServerCommand*> commands;
    bool* running;
};

class VLWebServerCommand {
public:
	virtual ~VLWebServerCommand() {}
	virtual void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) = 0;
};

class VLWebServerSession : public JSONSession {
public:
	VLWebServerSession(VLWebServerSessionState state) : state(state) {
	}
	~VLWebServerSession() {
	}
	void receiveJSON(picojson::value& val) {
		std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
		std::cout << val.get<picojson::object>()["command"].get<std::string>() << std::endl;
		std::map<std::string, VLWebServerCommand*>::iterator it = state.commands.find(cmd);
		if (it != state.commands.end()) {
			it->second->execute(this, val, &state);
		}
	}
	void update() {}

private:
	VLWebServerSessionState state;
};

class KillCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		*(state->running) = false;
	}
};

int main(int argc, char**argv) {
    using namespace vl;
	//DefaultQuery query;
	CompositeSamplingStrategy* strategy = new CompositeSamplingStrategy(false);
	strategy->addStrategy(new SetSamplingRange<double>("w", 0.1, 100));
	strategy->addStrategy(new SetSamplingRange<double>("a", 1, 1000));
	strategy->addStrategy(new SetSamplingRange<double>("c", 0, 10));
	strategy->addStrategy(new SetSamplingScale("a", new LogrithmicScale()));
	LatinHypercubeSampler* latinHyperCube = new LatinHypercubeSampler(4);
	latinHyperCube->addParameter<double>("w");
	latinHyperCube->addParameter<double>("a");
	latinHyperCube->addParameter<double>("c");
	strategy->addStrategy(latinHyperCube);
	IQuery* query = new SamplingQuery(strategy);
	IModel* model = new TestModel();
	IModelSample* sample = model->create(*query);
	for (std::string key : sample->getNavigation().getKeys()) {
		sample->getNavigation()[key].set<double>(3.14159);
		std::cout << key << " " << sample->getNavigation()[key].get<double>() << std::endl;
	}

	IDataSet& timeParm = sample->getNavigation()["time"];

	for (double time = 0.0; time < 6.0; time += 0.1) {
		timeParm.set<double>(time);
		sample->update();
		std::cout <<timeParm.get<double>() << " " << sample->getData()["y"].get<double>() << std::endl;
	}


	for (int i = 0; i < 10; i++) {
		IModelSample* s = model->create(*query);
		s->update();
		std::cout << "Sample: " << i << std::endl;
		for (std::string key : s->getData().getKeys()) {
			std::cout << "\t" << key << " " << s->getData()[key].get<double>() << std::endl;
		}

		delete s;
	}

	std::cout << "Usage: ./bin/ExampleServer 8081 path/to/web" << std::endl;

	if (argc > 2) {
		int port = std::atoi(argv[1]);
		std::string webDir = std::string(argv[2]);

        bool running = true;

		VLWebServerSessionState state;
		state.commands["kill"] = new KillCommand();
        state.running = &running;
		WebServerWithState<VLWebServerSession, VLWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


