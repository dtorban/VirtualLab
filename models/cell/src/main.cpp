#include <iostream>
#include "simulator.h"
#include "VirtualLab/IModel.h"
#include "VirtualLab/net/Server.h"
#include <thread>
#include "VirtualLab/ws/WebSocketAPI.h"

using namespace vl;

class CellSample : public IModelSample {
public:
    CellSample(CompositeDataSet* data) : data(data) {
        posx = new TypedData<double>(0.0);
        posy = new TypedData<double>(0.0);
        numModules = new TypedData<double>(0);
        engageNum = new TypedData<double>(0);
        data->addData("x", posx);
        data->addData("y", posy);
        data->addData("nm", numModules);
        data->addData("en", engageNum);
        timeParam = new TypedData<double>();
        time.addData("time", timeParam);

        Config config("");
        int simulationNumber = 0;
        int prefix = 0;
        s = new Simulator("", prefix, simulationNumber, config);
    }
    virtual ~CellSample() {
        delete data;
        delete s;
    }

    virtual IDataSet& getNavigation() { return time; }
    virtual const IDataSet& getData() const { return *data; }

    virtual void startUpdate() {
        double currentTime = s->GetTime();
        s->Update(timeParam->get<double>() - currentTime);
        posx->set<double>(s->GetCell().GetPosition().x);
        posy->set<double>(s->GetCell().GetPosition().y);
        numModules->set<double>(s->GetCell().GetNumModules());
        int eng = 0;
        for (int i = 0; i < s->GetCell().GetNumModules(); i++) {
            eng += s->GetCell().GetModule(i).GetEngageNum();
        }
        engageNum->set<double>(eng);
        timeParam->set<double>(s->GetTime());
    }

private:
    Simulator* s;
    CompositeDataSet time;
    CompositeDataSet* data;
    IDataSet* timeParam;
    IDataSet* posx;
    IDataSet* posy;
    IDataSet* numModules;
    IDataSet* engageNum;
};

class CellModel : public IModel {
public:
    CellModel(const std::string& name = "CellModel") : name(name) {}
    virtual ~CellModel() {}

    const std::string& getName() const { return name; }
    virtual IModelSample* create(const IQuery& query) const {
        CompositeDataSet* params = new CompositeDataSet();
        params->addData("id", new TypedData<double>(0));
        params->addData("num", new TypedData<double>(0));
        query.setParameters(*params);
        return new CellSample(params);
    }

private:
    std::string name;
};

class ServerRunner {
public:
    ServerRunner(Server& server) : server(server), serverThread(NULL) {}   

    ~ServerRunner() {
        if (serverThread) {
            serverThread->join();
            delete serverThread;
        }
    }

    void start() {
        serverThread = new std::thread(&ServerRunner::runServer, this);
    }
 
private:
    void runServer() {
        while (true) {
            server.service();
        }
    }
    Server& server;
    std::thread* serverThread;
};

int main(int argc, char* argv[]) {
	Config config("");
    int simulationNumber = 0;
    int prefix = 0;
    Simulator* s = new Simulator("", prefix, simulationNumber, config);

    /*while (true) {
        s->Update(1.0);
        //std::cout << s->GetCell().GetPosition() << std::endl;
    }*/
    //s->Simulate();

    Server server;
    server.registerModel(new CellModel("ModelA"));

	ServerRunner runner(server);
    runner.start();

    WebSocketAPI api(server, 8081);
    while (api.service()) {}

    delete s;

    return 0;
}
