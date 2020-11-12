/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <map>
//#define _USE_MATH_DEFINES
//#include <cmath>
//#include <libwebsockets.h>
#include "WebServer.h" 

class MyWebServerSession;
class MyWebServerCommand;

struct MyWebServerSessionState {
	std::map<std::string, MyWebServerCommand*> commands;
    bool* running;
};

class MyWebServerCommand {
public:
	virtual ~MyWebServerCommand() {}
	virtual void execute(MyWebServerSession* session, picojson::value& command, MyWebServerSessionState* state) = 0;
};

class MyWebServerSession : public JSONSession {
public:
	MyWebServerSession(MyWebServerSessionState state) : state(state) {
	}
	~MyWebServerSession() {
	}
	void receiveJSON(picojson::value& val) {
		std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
		std::cout << val.get<picojson::object>()["command"].get<std::string>() << std::endl;
		std::map<std::string, MyWebServerCommand*>::iterator it = state.commands.find(cmd);
		if (it != state.commands.end()) {
			it->second->execute(this, val, &state);
		}
	}
	void update() {}

private:
	MyWebServerSessionState state;
};

class KillCommand : public MyWebServerCommand {
public:
	void execute(MyWebServerSession* session, picojson::value& command, MyWebServerSessionState* state) {
		*(state->running) = false;
	}
};

int main(int argc, char**argv) {
	std::cout << "Usage: ./bin/ExampleServer 8081 path/to/web" << std::endl;

	if (argc > 2) {
		int port = std::atoi(argv[1]);
		std::string webDir = std::string(argv[2]);

        bool running = true;

		MyWebServerSessionState state;
		state.commands["kill"] = new KillCommand();
        state.running = &running;
		WebServerWithState<MyWebServerSession, MyWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


