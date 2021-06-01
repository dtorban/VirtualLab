#include <iostream>
#include "simulator.h"
#include "VirtualLab/IModel.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/util/JSONSerializer.h"

/*#include <mlpack/prereqs.hpp>
#include <mlpack/methods/pca/pca.hpp>
#include <mlpack/methods/pca/decomposition_policies/exact_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/quic_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/randomized_svd_method.hpp>
using namespace mlpack;
using namespace mlpack::pca;
using namespace mlpack::util;*/

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

class CellSimStateWriter : public SimState {
public:
    CellSimStateWriter(ByteBufferWriter& writer) : writer(writer) {}
    void WriteBytes(const unsigned char* bytes, int len) {
        writer.addData(bytes, len);
    }
    void ReadBytes(unsigned char* bytes, int len) const {}

private:
    ByteBufferWriter& writer;
};

class CellSimStateReader : public SimState {
public:
    CellSimStateReader(ByteBufferReader& reader) : reader(reader) {}
    void WriteBytes(const unsigned char* bytes, int len) {
    }
    void ReadBytes(unsigned char* bytes, int len) const {
        reader.readData(bytes, len);
    }

private:
    ByteBufferReader& reader;
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
        data["motors"] = DoubleDataValue();

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
        motors = &data["motors"].get<double>();

        nav["t"] = DoubleDataValue();
        nav["m"] = DoubleDataValue(1);

        DataObjectConfig config(params);
        int simulationNumber = params["num"].get<double>();
        int prefix = 0;
        std::cout << "sim num" << simulationNumber << std::endl;
        s = new Simulator("", prefix, simulationNumber, config);

        std::cout << "Start" << std::endl;
        #pragma omp parallel for 
        for (int f = 0; f < 10; f++) {
            std::cout << f << std::endl;
        }
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
            mod["nm"] = DoubleDataValue(s->GetCell().GetModule(i).GetNumMotors());
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
        *motors = s->GetCell().GetActiveMotors();

        /*if (30.0 == nav["t"].get<double>()) {
            //ByteBufferWriter writer;
            saveState(writer);
            std::cout << writer.getSize() << std::endl;
        }
        if (100.0 == nav["t"].get<double>()) {
            ByteBufferReader reader(writer.getBytes());
            loadState(reader);
        }*/

        nav["t"].set<double>(s->GetTime());
    }

    bool saveState(ByteBufferWriter& writer) {
        CellSimStateWriter w(writer);
        s->saveState(w);
        return true;
    }

    bool loadState(ByteBufferReader& reader) {
        CellSimStateReader r(reader);
        s->loadState(r);
        return true;
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
    double* motors;
    vl::Array* modules;
    Object* events;
    ByteBufferWriter writer;
};

class CellModel : public IModel {
public:
    CellModel(const std::string& name = "CellModel") : name(name), paramHelper(&params) {
        params["max"] = DataObject();
        params["min"] = DataObject();
        params["scale"] = DataObject();

        addParameter("substrate_k", 1.0, 0.1, 300.0, "log");
        addParameter("length", 5000.0, 100, 10000, "log");
        addParameter("cell_k", 10000.0, 100, 100000, "log");
        addParameter("totact", 100000.0, 100, 1000000, "log");
        addParameter("mpool", 1000.0, 100, 10000, "log");
        addParameter("cpool", 750, 75, 750, "log");
        addParameter("cell_nclutch", 10, 1, 100, "log");
        addParameter("kon", 1, 0.1, 10, "log");
        addParameter("koff", 0.1, 0.01, 1, "log");
        addParameter("fb", 2, 0.2, 20, "log");
        addParameter("clutch_k", 0.8, 0.08, 8, "log");
        addParameter("fm", 2, 0.2, 20, "log");
        addParameter("vu", 120, 1.2, 1200, "log");
        addParameter("maxpoly", 200, 100, 400, "log");
        addParameter("kbirth", 1, 0.1, 10, "log");
        addParameter("cap_k", 0.001, 0.0001, 0.01, "log");
        addParameter("num", 0.0, 0, 100000, "linear");

        //names = ['substrate_k', 'mpool', 'cpool', 'maxpoly','clutch_k', 'kon', 'koff', 'fb', 'fm', 'kbirth', 'cap_k']
        //min = [0.1, 100, 75, 100, 0.08, 0.1, 0.01, 0.2, 0.2, 0.1, 0.0001]
        //max = [300, 10000, 750, 400, 8, 10, 1, 20, 20, 10, 0.01]
    }
    virtual ~CellModel() {}

    void addParameter(const std::string& key, double defaultValue) {
        addParameter(key, defaultValue, defaultValue, defaultValue, "linear");
    }

    void addParameter(const std::string& key, double defaultValue, double min, double max, const std::string& scale) {
        //params[key] = DoubleDataValue(defaultValue);
        //params["min"].get<vl::Object>()[key] = DoubleDataValue(min);
        //params["max"].get<vl::Object>()[key] = DoubleDataValue(max);
        //params["scale"].get<vl::Object>()[key] = StringDataValue(scale);
        paramHelper.set(key, defaultValue, min, max, scale);
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }
    IModelSample* create(const DataObject& params) {
        return new CellSample(params);
    }

private:
    std::string name;
    DataObject params;
    ParameterHelper paramHelper;
};

class NSample : public IModelSample {
public:
    NSample(DataObject params, const std::vector<IModelSample*>& samples) : params(params), samples(samples) {
        //data["data"] = DataArray();
        //d = &data["data"].get<vl::Array>();

        nav = samples[0]->getNavigation();
        nav["sim"] = DoubleDataValue(0);
    }

    virtual ~NSample() {
        for (int i = 0; i < samples.size(); i++) {
            delete samples[i];
        }
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update() {
        vl::Array array;

        #pragma omp parallel for 
        for (int i = 0; i < samples.size(); i++) {
            samples[i]->getNavigation() = nav;
            samples[i]->update();
            //array.push_back(samples[i]->getData());
        }

        int simIndex = nav["sim"].get<double>();
        data.set<Object>(samples[simIndex]->getData().get<Object>());
        
        double meanRMC = 0.0;
        for (int i = 0; i < samples.size(); i++) {
            array.push_back(samples[i]->getData());
            meanRMC += samples[i]->getData()["rmc"].get<double>();
        }
        data["meanRMC"] = DoubleDataValue(meanRMC/samples.size());

        //*d = array;
    }

private:
    std::vector<IModelSample*> samples;
    DataObject params;
    DataObject nav;
    DataObject data;
    //vl::Array* d;
};

class NModel : public IModel {
public:
    NModel(const std::string& name, IModel* model) : name(name), model(model) {
        params = model->getParameters();
        params["N"] = DoubleDataValue(10);
    }
    virtual ~NModel() {
        delete model;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params) {
        std::vector<IModelSample*> samples;

        int numSamples = params["N"].get<double>();

        for (int i = 0; i < numSamples; i++) {
            DataObject parameters = params;
            parameters["num"].set<double>(i);
            //parameters["substrate_k"].set<double>(std::exp(std::log(0.1) + ((1.0*i)/(numSamples-1))*(std::log(300)-std::log(0.1))));
            //parameters["cpool"].set<double>(std::exp(std::log(75) + ((1.0*i)/(numSamples-1))*(std::log(750)-std::log(75))));
            //parameters["mpool"].set<double>(std::exp(std::log(100) + ((1.0*i)/(numSamples-1))*(std::log(10000)-std::log(100))));
            IModelSample* sample = model->create(parameters);
            samples.push_back(sample);
        }

        return new NSample(params, samples);
    }

private:
    IModel* model;
    std::string name;
    DataObject params;
};

/*
// Run RunPCA on the specified dataset with the given decomposition method.
template<typename DecompositionPolicy>
void RunPCA(arma::mat& dataset,
            const size_t newDimension,
            const bool scale,
            const double varToRetain)
{
  PCA<DecompositionPolicy> p(scale);

  //std::cout << "Performing PCA on dataset..." << std::endl;
  double varRetained;

    varRetained = p.Apply(dataset, newDimension);

  //std::cout << (varRetained * 100) << "% of variance retained (" <<
      //dataset.n_rows << " dimensions)." << std::endl;
}

class PCASample : public IModelSample {
public:
    PCASample(DataObject params, IModelSample* sample) : params(params), sample(sample) {
        data["data"] = DataArray();
        data["pca"] = DataArray();
        data["pca2"] = DataArray();
        arr = &data["data"].get<vl::Array>();
        d = &data["pca"].get<vl::Array>();
        d2 = &data["pca2"].get<vl::Array>();

        nav = sample->getNavigation();
    }

    virtual ~PCASample() {
        delete sample;
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update(int id, Object& prev, Object& obj) {

        double x = prev["x"].get<double>();
        double y = prev["y"].get<double>();
        double dx = obj["x"].get<double>() - x;
        double dy = obj["y"].get<double>() - y;
        double dist = std::sqrt(std::pow(dx,2.0)+std::pow(dy,2.0)) ;

        arma::rowvec r;
		r << obj["actin"].get<double>()
			<< obj["aflow"].get<double>() 
			<< obj["en"].get<double>() 
			<< obj["free_actin"].get<double>() 
			<< obj["nm"].get<double>()
			<< std::sqrt(std::pow(obj["fx"].get<double>(),2.0)+std::pow(obj["fy"].get<double>(),2.0)) 
            << dist
			<< sample->getData()["rmc"].get<double>()
			<< arma::endr;
        if (nav["t"].get<double>() > 50.0) {
		    rows.push_back(r);
        }



    }

    virtual void update() {

        std::vector<Object> prev;
        //std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
        for (int i = 0; i < sample->getData()["data"].get<vl::Array>().size(); i++) {
            prev.push_back(sample->getData()["data"].get<vl::Array>()[i].get<Object>());
        }

        sample->getNavigation() = nav;
        sample->update();
        
        vl::Array array;
        vl::Array array2;
        if (prev.size() > 0) {
            for (int i = 0; i < sample->getData()["data"].get<vl::Array>().size(); i++) {
                Object obj = sample->getData()["data"].get<vl::Array>()[i].get<Object>();
                update(i, prev[i], obj);
            }

            if (rows.size() > 2) {
                arma::mat A(rows.size(), rows[0].n_cols);
            
                //B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
                for (int i = 0; i < rows.size(); i++) {
                    A.row(i) = rows[i];
                }
                A = A.t();
                RunPCA<ExactSVDPolicy>(A, 2, true, 1.0);
                A = A.t();


                for (int f = 0; f < rows.size(); f+=10*prev.size()) {
                    for (int i = 0; i < prev.size(); i++) {
                        vl::DataObject obj;
                        //std::cout << A.row(f);
                        obj["x"] = DoubleDataValue(A(f+i,0));
                        obj["y"] = DoubleDataValue(A(f+i,1));
                        obj["id"] = DoubleDataValue(i);
                        obj["t"] = DoubleDataValue(f/prev.size());
                        array2.push_back(obj);
                    }
                }

                for (int f = rows.size() - 20*prev.size(); f < rows.size(); f+=1*prev.size()) {
                    for (int i = 0; i < prev.size(); i++) {
                        vl::DataObject obj;
                        //std::cout << A.row(f);
                        obj["x"] = DoubleDataValue(A(f+i,0));
                        obj["y"] = DoubleDataValue(A(f+i,1));
                        obj["id"] = DoubleDataValue(i);
                        obj["t"] = DoubleDataValue(f/prev.size());
                        array.push_back(obj);
                    }
                }
            }
        }
        *d = array;
        *d2 = array2;
        *arr = sample->getData()["data"].get<vl::Array>();
    }

private:
    IModelSample* sample;
    DataObject params;
    DataObject nav;
    DataObject data;
    vl::Array* d;
    vl::Array* d2;
    vl::Array* arr;
	std::vector<arma::rowvec> rows;
};

class PCAModel : public IModel {
public:
    PCAModel(const std::string& name, IModel* model) : name(name), model(model) {
        params = model->getParameters();
        params["N"] = DoubleDataValue(10);
    }
    virtual ~PCAModel() {
        delete model;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params) {
        return new PCASample(params, model->create(params));
    }

private:
    IModel* model;
    std::string name;
    DataObject params;
};*/

class MovingAverageSample : public IModelSample {
public:
    MovingAverageSample(DataObject params, IModelSample* sample) : params(params), sample(sample) {
        radius = params["radius"].get<double>();
        nav = sample->getNavigation();
    }

    virtual ~MovingAverageSample() {
        delete sample;
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update() {
        sample->getNavigation() = nav;
        sample->update();
        
        window.push_back(sample->getData());

        int dataIndex = window.size() > radius ? window.size()-radius-1 : 0;
        //data = sample->getData();
        data.set<Object>(window[dataIndex].get<Object>());

        if (window.size() > radius*2+1) {
            window.erase(window.begin());
            double x = 0;
            double y = 0;
            double fx = 0;
            double fy = 0;
            double rmc = 0;
            for (int i = 0; i < window.size(); i++) {
                x += window[i].get<Object>().find("x")->second.get<double>();
                y += window[i].get<Object>().find("y")->second.get<double>();
                fx += window[i].get<Object>().find("fx")->second.get<double>();
                fy += window[i].get<Object>().find("fy")->second.get<double>();
                if (i > 0) {
                    double prevX = window[i-1].get<Object>().find("x")->second.get<double>();
                    double prevY = window[i-1].get<Object>().find("x")->second.get<double>();
                    rmc += std::sqrt(std::pow(x-prevX,2) + std::pow(x-prevY,2));
                }
            }
            data["x"].set<double>(x/window.size());
            data["y"].set<double>(y/window.size());
            data["fx"].set<double>(x/window.size());
            data["fy"].set<double>(y/window.size());
            data["rmc"] = DoubleDataValue(rmc/(window.size()-1));

            vl::DataArray path;
            for (int i = 0; i < window.size(); i++) {
                DataObject obj;
                obj["x"] = window[i].get<Object>().find("x")->second;
                obj["y"] = window[i].get<Object>().find("y")->second;
                path.get<vl::Array>().push_back(obj);
                //path.push_back(Da)
            }
            data["path"] = path;
        }
        else {
            data["rmc"] = DoubleDataValue(0);
        }
    }

private:
    IModelSample* sample;
    DataObject params;
    DataObject nav;
    DataObject data;
    int radius;
    std::vector<DataValue> window;
};

class MovingAverageModel : public IModel {
public:
    MovingAverageModel(const std::string& name, IModel* model) : name(name), model(model) {
        params = model->getParameters();
        params["radius"] = DoubleDataValue(5);
    }
    virtual ~MovingAverageModel() {
        delete model;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params) {
        return new MovingAverageSample(params, model->create(params));
    }

private:
    IModel* model;
    std::string name;
    DataObject params;
};

class VLApiConnector : public IVirtualLabAPI {
public:
    VLApiConnector(Server* server, int port) : server(server), port(port) {
	std::cout << "Before Client." << std::endl; 
     	    client = new Client();
    }
    ~VLApiConnector() {
        delete client;
    }
    void registerModel(IModel* model) {
        server->registerModel(model);
        RemoteModel remoteModel(port, model);
        client->registerModel(&remoteModel);
    }
    void deregisterModel(IModel* model) {
        server->deregisterModel(model);
        client->deregisterModel(model);
    }
    const std::vector<ModelProxy> getModels() {
        return client->getModels();
    }

private:
    Client* client;
    Server* server;
    int port;
};

int main(int argc, char* argv[]) {
    std::cout << "Usage: CellModel <port>" << std::endl;

	if (argc > 1) {
		int port = std::atoi(argv[1]);

        Server server(port);
        VLApiConnector api(&server, port);
        api.registerModel(new CellModel("Cell"));
        api.registerModel(new MovingAverageModel("Smooth Cell", new CellModel("Cell")));
        //api.registerModel(new MovingAverageModel("Cell", new CellModel("Cell")));
        api.registerModel(new NModel("N-Cell", new CellModel("Cell")));
        //api.registerModel(new PCAModel("PCA-Cell", new CellModel("Cell")));
        api.registerModel(new NModel("N-Cell-2", new MovingAverageModel("Moving-Average", new CellModel("Cell"))));
        while(true) {
            server.service();
        }
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
