#include "VirtualLab/ws/WebSocketAPI.h"
#include "VirtualLab/util/JSONSerializer.h"

namespace vl {

void VLWebSocketsSession::receiveJSON(picojson::value& val) {
    std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
    std::map<std::string, VLWebSocketsCommand*>::iterator it = state.commands.find(cmd);
    if (it != state.commands.end()) {
        it->second->execute(this, val, &state);
    }
}

class MyKillCommand : public VLWebSocketsCommand {
public:
	void execute(VLWebSocketsSession* session, picojson::value& command, VLWebSocketsSessionState* state) {
		*(state->running) = false;
	}
};

class MyGetModelsCommand : public VLWebSocketsCommand {
public:
	void execute(VLWebSocketsSession* session, picojson::value& command, VLWebSocketsSessionState* state) {
		const std::vector<IModel*>& models = state->api->getModels();
        picojson::object data;
        picojson::array a;
        for (int i = 0; i < models.size(); i++) {
            picojson::object obj;
            obj["name"] = picojson::value(models[i]->getName());
            obj["id"] = picojson::value((double)i);
            a.push_back(picojson::value(obj));
        }
		data["command"] = picojson::value("updateModels");
		data["models"] = picojson::value(a);
        picojson::value ret(data);
		session->sendJSON(ret);
	}
};

class MyCreateModelSampleCommand : public VLWebSocketsCommand {
public:
	void execute(VLWebSocketsSession* session, picojson::value& command, VLWebSocketsSessionState* state) {
		const std::vector<IModel*>& models = state->api->getModels();
        picojson::object obj = command.get<picojson::object>();
        IModel* model = models[obj["modelId"].get<double>()];
        DefaultQuery q;
        IModelSample* sample = model->create(q);
        std::cout << "sample created?" << std::endl;

        picojson::object data;
		data["command"] = picojson::value("modelSampleCreated");
		data["data"] = JSONSerializer::instance().serializeJSON(sample->getData());
		data["navigation"] = JSONSerializer::instance().serializeJSON(sample->getNavigation());
        picojson::value ret(data);
		session->sendJSON(ret);

        delete sample;
        
	}
};

WebSocketAPI::WebSocketAPI(IVirtualLabAPI& api, int port) : api(api), running(true) {
    VLWebSocketsSessionState state;
    state.commands["kill"] = new MyKillCommand();
    state.commands["getModels"] = new MyGetModelsCommand();
    state.commands["createModelSample"] = new MyCreateModelSampleCommand(); 
    state.running = &running;
    state.api = this;
    server = new WebServerWithState<VLWebSocketsSession, VLWebSocketsSessionState>(state, port);
}

WebSocketAPI::~WebSocketAPI() {
    delete server;
}

void WebSocketAPI::registerModel(IModel* model) {
    api.registerModel(model);
}

void WebSocketAPI::deregisterModel(IModel* model) {
    api.deregisterModel(model);
}

const std::vector<IModel*>& WebSocketAPI::getModels() {
    return api.getModels();
}

bool WebSocketAPI::service() {
    if (running) {
        server->service();
    }

    return running;
}

}