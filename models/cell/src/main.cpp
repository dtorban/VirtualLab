#include <iostream>
#include "simulator.h"
#include "VirtualLab/IModel.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

class CellSample : public IModelSample {
public:
    CellSample(DataObject params) : params(params) {
        std::cout << JSONSerializer::instance().serialize(params) << std::endl;
        data["x"] = DoubleDataValue();
        data["y"] = DoubleDataValue();
        data["nm"] = DoubleDataValue();
        data["en"] = DoubleDataValue();
        posx = &data["x"].get<double>();
        posy = &data["y"].get<double>();
        std::cout << posx << " " << posy << std::endl;
        numModules = &data["nm"].get<double>();
        engageNum = &data["en"].get<double>();
        time["time"] = DoubleDataValue();
        timeParam = &time["time"].get<double>();

        Config config("");
        int simulationNumber = 0;
        int prefix = 0;
        s = new Simulator("", prefix, simulationNumber, config);
    }
    virtual ~CellSample() {
        std::cout << "Delete cell" << std::endl;
        delete s;
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return time; }
    virtual const DataObject& getData() const { return data; }

    virtual void update() {
        double currentTime = s->GetTime();
        s->Update(time["time"].get<double>() - currentTime);
        *posx = s->GetCell().GetPosition().x;
        *posy = s->GetCell().GetPosition().y;
        *numModules = s->GetCell().GetNumModules();
        int eng = 0;
        for (int i = 0; i < s->GetCell().GetNumModules(); i++) {
            eng += s->GetCell().GetModule(i).GetEngageNum();
        }
        *engageNum = eng;
        time["time"].set<double>(s->GetTime());
    }

private:
    Simulator* s;
    DataObject params;
    DataObject time;
    DataObject data;
    double* timeParam;
    double* posx;
    double* posy;
    double* numModules;
    double* engageNum;
};

class CellModel : public IModel {
public:
    CellModel(const std::string& name = "CellModel") : name(name) {}
    virtual ~CellModel() {}

    const std::string& getName() const { return name; }
    virtual IModelSample* create(const IQuery& query) const {
        DataObject params;
        params["id"] = DoubleDataValue(0.0);
        params["num"] = DoubleDataValue(0.0);
        query.setParameters(params);
        return new CellSample(params);
    }

private:
    std::string name;
};

int main(int argc, char* argv[]) {
    Server server;
    server.registerModel(new CellModel("ModelA"));
    while(true) {
        server.service();
    }

    /*VirtualLabAPI api;
    api.registerModel(new CellModel("ModelA"));
	//VirtualLabAPI api;
	//api.registerModel(new TestModel());
	IModel* model = api.getModels()[0];
	
	DefaultQuery query;
	IModelSample* sample = model->create(query);
	std::cout << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	std::string str = JSONSerializer::instance().serialize(sample->getParameters());
	DataValue d;
	JSONSerializer::instance().deserialize(str, d);
	std::cout << str << " " << JSONSerializer::instance().serialize(d) << std::endl;

    DataObject time = sample->getNavigation();

	for (int i = 1; i < 1000; i++) {
		time["time"].set<double>(0.1*i);
        std::string sTime = JSONSerializer::instance().serialize(time);
        JSONSerializer::instance().deserialize(sTime, sample->getNavigation());
		sample->update();
		std::cout << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
		std::cout << i << " " <<  JSONSerializer::instance().serialize(sample->getData()) << std::endl;
	}*/

    return 0;
}
