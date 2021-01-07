#ifndef VIRTUALLAB_WS_WEBSOCKET_API_H_
#define VIRTUALLAB_WS_WEBSOCKET_API_H_

/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include "WebServer.h" 
#include "VirtualLab/IVirtualLabAPI.h"

namespace vl {

class VLWebSocketsSession;
class VLWebSocketsCommand;
class WebSocketAPI;

struct VLWebSocketsSessionState {
	std::map<std::string, VLWebSocketsCommand*> commands;
    WebSocketAPI* api;
    bool* running;
};

class VLWebSocketsCommand {
public:
	virtual ~VLWebSocketsCommand() {}
	virtual void execute(VLWebSocketsSession* session, picojson::value& command, VLWebSocketsSessionState* state) = 0;
};

class VLWebSocketsSession : public JSONSession {
public:
	VLWebSocketsSession(VLWebSocketsSessionState state) : state(state) {}
	~VLWebSocketsSession() {}
	void receiveJSON(picojson::value& val);
	void update() {}

private:
	VLWebSocketsSessionState state;
};

class WebSocketAPI : public IVirtualLabAPI {
public:
	WebSocketAPI(IVirtualLabAPI& api, int port);
	~WebSocketAPI();

    virtual void registerModel(IModel* model);
    virtual void deregisterModel(IModel* model);
    virtual const std::vector<IModel*>& getModels();

    bool service();

private:
	IVirtualLabAPI& api;
    WebServerWithState<VLWebSocketsSession, VLWebSocketsSessionState>* server;
    bool running;
};

}

#endif