#include <iostream>
#include "simulator.h"
#include "VirtualLab/IModel.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/util/JSONSerializer.h"

using namespace vl;

class DataObjectConfig : public IConfig {
public:
	DataObjectConfig(const DataObject& obj) : obj(obj) {}
	int getInt(int id, const std::string& key, int defaultValue) { 
        return obj[key].get<double>(defaultValue);
    }
	double getDouble(int id, const std::string& key, double defaultValue) {
        return obj[key].get<double>(defaultValue);
    }
private:
	const DataObject& obj;
};

class CellSample : public IModelSample {
public:
    CellSample(DataObject params) : params(params) {
        std::cout << JSONSerializer::instance().serialize(params) << std::endl;
        data["x"] = DoubleDataValue();
        data["y"] = DoubleDataValue();
        data["nm"] = DoubleDataValue();
        data["en"] = DoubleDataValue();
        data["fx"] = DoubleDataValue();
        data["fy"] = DoubleDataValue();
        data["actin"] = DoubleDataValue();
        data["free_actin"] = DoubleDataValue();
        data["aflow"] = DoubleDataValue();
        data["m"] = DataArray();
        data["ev"] = DataObject();

        posx = &data["x"].get<double>();
        posy = &data["y"].get<double>();
        numModules = &data["nm"].get<double>();
        engageNum = &data["en"].get<double>();
        fx = &data["fx"].get<double>();
        fy = &data["fy"].get<double>();
        actin = &data["actin"].get<double>();
        free_actin = &data["free_actin"].get<double>();
        aflow = &data["aflow"].get<double>();
        modules = &data["m"].get<vl::Array>();
        events = &data["ev"].get<Object>();

        nav["t"] = DoubleDataValue();
        nav["m"] = DoubleDataValue(1);

        DataObjectConfig config(params);
        int simulationNumber = params["num"].get<double>();
        int prefix = 0;
        std::cout << "sim num" << simulationNumber << std::endl;
        s = new Simulator("", prefix, simulationNumber, config);
    }
    virtual ~CellSample() {
        std::cout << "Delete cell" << std::endl;
        delete s;
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update() {
        double currentTime = s->GetTime();
        s->Update(nav["t"].get<double>() - currentTime);
        *posx = s->GetCell().GetPosition().x;
        *posy = s->GetCell().GetPosition().y;
        *numModules = s->GetCell().GetNumModules();
        int eng = 0;

        vl::Array mods;
        for (int i = 0; i < s->GetCell().GetNumModules(); i++) {
            eng += s->GetCell().GetModule(i).GetEngageNum();
            DataObject mod;
            mod["id"] = DoubleDataValue(s->GetCell().GetModule(i).GetId());
            mod["en"] = DoubleDataValue(s->GetCell().GetModule(i).GetEngageNum());
            mod["fx"] = DoubleDataValue(s->GetCell().GetModule(i).GetForce().x);
            mod["fy"] = DoubleDataValue(s->GetCell().GetModule(i).GetForce().y);
            mod["x"] = DoubleDataValue(s->GetCell().GetModule(i).GetRef().x);
            mod["y"] = DoubleDataValue(s->GetCell().GetModule(i).GetRef().y);
            if (nav["m"].get<double>()) {
                mods.push_back(mod);
            }
        }
        *engageNum = eng; 
        *modules = mods;

        const std::map<std::string, int>& eventCounts = s->GetCell().GetEventCounts();
        for (std::map<std::string, int>::const_iterator it = eventCounts.begin(); it != eventCounts.end(); it++) {
            (*events)[it->first] = DoubleDataValue(it->second);
        }

        *fx = s->GetCell().GetForce().x;
        *fy = s->GetCell().GetForce().y;
        *actin = s->GetCell().GetFActin();
        *free_actin = s->GetCell().GetFreeActin();
        *aflow = s->GetCell().GetAFlow();
        nav["t"].set<double>(s->GetTime());
    }

private:
    Simulator* s;
    DataObject params;
    DataObject nav;
    DataObject data;
    double* posx;
    double* posy;
    double* numModules;
    double* engageNum;
    double* fx;
    double* fy;
    double* actin;
    double* free_actin; 
    double* aflow;
    vl::Array* modules;
    Object* events;
};

class CellModel : public IModel {
public:
    CellModel(const std::string& name = "CellModel") : name(name) {
        params["substrate_k"] = DoubleDataValue(1.0);
        params["length"] = DoubleDataValue(5000.0);
        params["cell_k"] = DoubleDataValue(10000);
        params["totact"] = DoubleDataValue(100000);
        params["mpool"] = DoubleDataValue(1000);
        params["cpool"] = DoubleDataValue(750);
        params["cell_nclutch"] = DoubleDataValue(10);
        params["kon"] = DoubleDataValue(1);
        params["koff"] = DoubleDataValue(0.1);
        params["fb"] = DoubleDataValue(2);
        params["clutch_k"] = DoubleDataValue(0.8);
        params["fm"] = DoubleDataValue(2);
        params["vu"] = DoubleDataValue(120);
        params["maxpoly"] = DoubleDataValue(200);
        params["kbirth"] = DoubleDataValue(1);
        params["cap_k"] = DoubleDataValue(0.001);
        params["id"] = DoubleDataValue(0.0);
        params["num"] = DoubleDataValue(0.0);
    }
    virtual ~CellModel() {}

    const std::string& getName() const { return name; }

    const DataObject& getParameters() const { return params; }
    IModelSample* create(const DataObject& params) const {
        return new CellSample(params);
    }

private:
    std::string name;
    DataObject params;
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

	IModelSample* sample = model->create(model->getParameters());
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
