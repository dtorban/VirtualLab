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
//#include "VirtualLab/impl/TestModel.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/impl/TestModel.h"
#include "VirtualLab/util/JSONSerializer.h"

class VLWebServerSession;
class VLWebServerCommand;

using namespace vl;

class ModelNavigator {
public:
	ModelNavigator(const IModel& model) : model(model), currentSample(NULL) {
		DefaultQuery query;
		createSample(query);
	}

	~ModelNavigator() {
		delete currentSample;
	}

	void createSample(const IQuery& query) {
		delete currentSample;
		currentSample = model.create(query);
		currentSample->update();
	}

	IModelSample* getSample() {
		return currentSample;
	}

private:
	const IModel& model;
	IModelSample* currentSample;
};

struct VLWebServerSessionState {
	std::map<std::string, VLWebServerCommand*> commands;
    bool* running;
	ModelNavigator* navigator;
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
		//std::cout << val.get<picojson::object>()["command"].get<std::string>() << std::endl;
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

class JSONDataSetSerializer {
public:
	picojson::value serialize(const DataObject& dataSet) {
		return JSONSerializer::instance().serializeJSON(dataSet);
	}
};

class JSONSerializedQuery : public IQuery {
public:
	JSONSerializedQuery(picojson::value* serializedParams) : serializedParams(serializedParams) {}

	void setParameters(DataObject& params, DataObjectStack& context) const {
		JSONDataSetSerializer serializer;
		*serializedParams = serializer.serialize(params);
	}

	picojson::value* serializedParams;
};

class CreateSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::value serializedParams;
		JSONSerializedQuery query(&serializedParams);
		state->navigator->createSample(query);
		IModelSample* sample = state->navigator->getSample();
		picojson::object data;
		data["command"] = picojson::value("updateSample");
		data["query"] = serializedParams;

		JSONDataSetSerializer serializer;
		picojson::object jsonSample;
		jsonSample["navigation"] = serializer.serialize(sample->getNavigation());
		jsonSample["data"] = serializer.serialize(sample->getData());

		data["sample"] = picojson::value(jsonSample);
		picojson::value ret(data);
		session->sendJSON(ret);
	}
};

class JSONQuery : public IQuery {
public:
	JSONQuery(const picojson::object& jsonParams) : jsonParams(jsonParams) {}

	void setParameters(DataObject& params, DataObjectStack& context) const {
		for (picojson::object::const_iterator it = jsonParams.begin(); it != jsonParams.end(); it++) {
			params[it->first].set<double>(it->second.get<double>());
		}
	}

	picojson::object jsonParams;
};

class UpdateQueryCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object jsonQuery = command.get<picojson::object>()["query"].get<picojson::object>();

		JSONQuery query(jsonQuery);
		state->navigator->createSample(query);

		picojson::object navigation = command.get<picojson::object>()["navigation"].get<picojson::object>();
		IModelSample* sample = state->navigator->getSample();
		for (picojson::object::iterator it = navigation.begin(); it != navigation.end(); it++) {
			sample->getNavigation()[it->first].set<double>(it->second.get<double>());
		}
		sample->update();

		picojson::object data;
		data["command"] = picojson::value("updateSample");
		data["query"] = picojson::value(jsonQuery);

		JSONDataSetSerializer serializer;
		picojson::object jsonSample;
		jsonSample["navigation"] = serializer.serialize(sample->getNavigation());
		jsonSample["data"] = serializer.serialize(sample->getData());

		data["sample"] = picojson::value(jsonSample);
		picojson::value ret(data);
		session->sendJSON(ret);
	}
};

class UpdateNavigationCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object navigation = command.get<picojson::object>()["navigation"].get<picojson::object>();
		IModelSample* sample = state->navigator->getSample();
		for (picojson::object::iterator it = navigation.begin(); it != navigation.end(); it++) {
			sample->getNavigation()[it->first].set<double>(it->second.get<double>());
		}

		//CompositeDataSet ds;
		//JSONSerializer::instance().deserialize(picojson::value(navigation).serialize(), ds);
		sample->update();
		//JSONSerializer::instance().serialize(sample->getData());

		picojson::object data;
		data["command"] = picojson::value("updateSample");

		JSONDataSetSerializer serializer;
		picojson::object jsonSample;
		jsonSample["navigation"] = serializer.serialize(sample->getNavigation());
		jsonSample["data"] = serializer.serialize(sample->getData());

		data["sample"] = picojson::value(jsonSample);
		picojson::value ret(data);
		session->sendJSON(ret);
	}
};

int main(int argc, char**argv) {
    /*using namespace vl;
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
	*/

	int pid = 0;
#ifdef WIN32
	pid = 1;
	if (false) {
#else
	if ((pid = fork()) < 0) {
#endif
		std::cout << "fork failed" << std::endl;
		return 1;
	}

	if (pid != 0) {
		//pid = fork();
		//if (pid != 0) {
			Server server;
			server.registerModel(new TestModel("ModelA"));
			server.registerModel(new TestModel("ModelB"));
			while(true) {
				server.service();
			}
			return 0;
		//}

		/*Client client;
		client.registerModel(new TestModel("ModelA"));
		client.registerModel(new TestModel("ModelB"));
		while(true) {
			std::cout << "Waiting..." << std::endl;
			client.service();
		}*/

		return 0;
		
	}

	std::cout << "Usage: ./bin/ExampleServer 8081 path/to/web" << std::endl;

	Client api;
	for (int i = 0; i < 10; i++) {
		std::vector<IModel*> models = api.getModels();
		if (models.size() > 0) {
			for (int f = 0; f < models.size(); f++) {
				std::cout << "Here: " <<  models[f]->getName() << std::endl;
			}
			break;
		}
	}

	DefaultQuery query;
	std::vector<IModel*> models = api.getModels();
	//IModelSample* sample = models[0]->create(query);
	//sample->getNavigation()["time"].set<double>(0.1);
	//sample->update();
	//api.registerModel(new TestModel());
	//ModelNavigator navigator(*api.getModels()[0]);
	//ModelNavigator navigator(*new TestModel());
	ModelNavigator navigator(*models[0]);

	if (argc > 2) {
		int port = std::atoi(argv[1]);
		std::string webDir = std::string(argv[2]);

        bool running = true;

		VLWebServerSessionState state;
		state.commands["kill"] = new KillCommand();
		state.commands["createSample"] = new CreateSampleCommand();
		state.commands["updateQuery"] = new UpdateQueryCommand();
		state.commands["updateNavigation"] = new UpdateNavigationCommand();
        state.running = &running;
		state.navigator = &navigator;
		WebServerWithState<VLWebServerSession, VLWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


