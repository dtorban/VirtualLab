/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include "WebServer.h" 
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/impl/TestModel.h"
#include "VirtualLab/util/JSONSerializer.h"

class VLWebServerSession;
class VLWebServerCommand;

using namespace vl;

struct VLWebServerSessionState {
	std::map<std::string, VLWebServerCommand*> commands;
	IVirtualLabAPI* api;
	std::vector<IModel*> models;
};

class VLWebServerCommand {
public:
	virtual ~VLWebServerCommand() {}
	virtual void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) = 0;
};

class VLWebServerSession : public JSONSession {
public:
	VLWebServerSession(VLWebServerSessionState state) : state(state), currentSampleIndex(0) {
	}
	~VLWebServerSession() {
		for (std::map<int, IModelSample*>::iterator it = samples.begin(); it != samples.end(); it++) {
			delete it->second;
		}
	}
	void receiveJSON(picojson::value& val) {
		std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
		std::map<std::string, VLWebServerCommand*>::iterator it = state.commands.find(cmd);
		if (it != state.commands.end()) {
			it->second->execute(this, val, &state);
		}
	}

	void update() {}

	IModelSample* getSample(int id) {
		return samples[id];
	}

	int addSample(IModelSample* sample) {
		samples[currentSampleIndex] = sample;
		currentSampleIndex++;
		return currentSampleIndex-1;
	}

	void deleteSample(int id) {
		std::map<int, IModelSample*>::iterator it = samples.find(id);
		delete it->second;
		samples.erase(it);
	}

private:
	VLWebServerSessionState state;
	std::map<int, IModelSample*> samples;
	int currentSampleIndex;
};

class DeleteSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double sampleId = command.get<picojson::object>()["sampleId"].get<double>();

		session->deleteSample(sampleId);

		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class UpdateSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double sampleId = command.get<picojson::object>()["sampleId"].get<double>();

		IModelSample* sample = session->getSample(sampleId);
		JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["nav"], sample->getNavigation());
		std::cout << sample->getNavigation()["t"].get<double>() << std::endl;
		sample->update();

		data["nav"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getNavigation()));
		data["data"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getData()));

		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class CreateSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double modelIndex = command.get<picojson::object>()["index"].get<double>();
		DataObject obj;
		JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["params"], obj);

		IModelSample* sample = state->models[modelIndex]->create(obj);
		int id = session->addSample(sample);

		data["sampleId"] = picojson::value((double)id);
		data["nav"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getNavigation()));
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class GetParametersCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double modelIndex = command.get<picojson::object>()["index"].get<double>();

		data["params"] = picojson::value(JSONSerializer::instance().serializeJSON(state->models[modelIndex]->getParameters()));
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class InitCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		picojson::array modelNames;
		for (int i = 0; i < state->models.size(); i++) {
			modelNames.push_back(picojson::value(state->models[i]->getName()));
		}
		data["models"] = picojson::value(modelNames);
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

int main(int argc, char**argv) {
	std::cout << "Usage: ./bin/ExampleServer 8081 path/to/web" << std::endl;

	Client api;

	if (argc > 2) {
		int port = std::atoi(argv[1]);
		std::string webDir = std::string(argv[2]);

        bool running = true;

		VLWebServerSessionState state;
		state.commands["init"] = new InitCommand();
		state.commands["getParameters"] = new GetParametersCommand();
		state.commands["createSample"] = new CreateSampleCommand();
		state.commands["updateSample"] = new UpdateSampleCommand();
		state.commands["deleteSample"] = new DeleteSampleCommand();
		state.api = &api;
		state.models = api.getModels();
		WebServerWithState<VLWebServerSession, VLWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


