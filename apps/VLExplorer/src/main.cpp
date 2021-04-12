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
	VLWebServerSession(VLWebServerSessionState state) : state(state) {
	}
	~VLWebServerSession() {
	}
	void receiveJSON(picojson::value& val) {
		std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
		std::map<std::string, VLWebServerCommand*>::iterator it = state.commands.find(cmd);
		if (it != state.commands.end()) {
			it->second->execute(this, val, &state);
		}
	}
	void update() {}

private:
	VLWebServerSessionState state;
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
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {\
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
		state.api = &api;
		state.models = api.getModels();
		WebServerWithState<VLWebServerSession, VLWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


