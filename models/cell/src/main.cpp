#include <iostream>
#include "simulator.h"
#include "VirtualLab/IModel.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/util/JSONSerializer.h"
#include "VirtualLab/impl/ExtendedModel.h"
#include <gsl/gsl_fit.h>
#include "VirtualLab/opt/OptimizedModel.h"

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
        data["f"] = DoubleDataValue();
        data["actin"] = DoubleDataValue();
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
        force = &data["f"].get<double>();
        actin = &data["actin"].get<double>();
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
        *aflow = s->GetCell().GetAFlow();
        *motors = s->GetCell().GetActiveMotors();
        *force = s->GetCell().GetForceTotal();

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
    double* force;
    double* actin;
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

        addParameter("substrate_k", 1.0, 0.01, 1000.0, "log");
        addParameter("length", 5000.0, 100, 10000, "log");
        addParameter("cell_k", 10000.0, 100, 100000, "log");
        addParameter("totact", 100000.0, 100, 1000000, "log");
        addParameter("mpool", 1000.0, 100, 10000, "log");
        addParameter("cpool", 750, 75, 7500, "log");
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
    }
    virtual ~CellModel() {}

    void addParameter(const std::string& key, double defaultValue) {
        addParameter(key, defaultValue, defaultValue, defaultValue, "linear");
    }

    void addParameter(const std::string& key, double defaultValue, double min, double max, const std::string& scale) {
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

        #pragma omp parallel for 
        for (int i = 0; i < samples.size(); i++) {
            samples[i]->getNavigation() = nav;
            samples[i]->update();
        }

        int simIndex = nav["sim"].get<double>();
        data.set<Object>(samples[simIndex]->getData().get<Object>());

        DataArray sampleData;
        for (int i = 0; i < samples.size(); i++) {
            sampleData.push_back(samples[i]->getData());
        }
        data["samples"] = sampleData;
    }

    const std::vector<IModelSample*>& getSamples() { return samples; }

private:
    std::vector<IModelSample*> samples;
    DataObject params;
    DataObject nav;
    DataObject data;
};

class NModel : public IModel {
public:
    NModel(const std::string& name, IModel* model, IModelSampler* sampler = NULL) : name(name), model(model), sampler(sampler) {
        params = model->getParameters();
        params["N"] = DoubleDataValue(10);
    }
    virtual ~NModel() {
        delete model;
        delete sampler;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params) {
        std::vector<IModelSample*> samples;

        int numSamples = params["N"].get<double>();
        std::cout << "num Samples = " << numSamples << std::endl;

        if (sampler) {
            sampler->reset();
        }

        DataArray sampleParams;
        for (int i = 0; i < numSamples; i++) {
            DataObject parameters = params;
            //parameters["num"].set<double>(parameters["num"].get<double>()+i);
            if (sampler && sampler->hasNext()) {
                sampler->sample(parameters);
                sampler->next();
            }
            IModelSample* sample = model->create(parameters);
            sampleParams.push_back(sample->getParameters());
            samples.push_back(sample);
        }

        DataObject p = params;
        p["samples"] = sampleParams;

        return new NSample(p, samples);
    }

private:
    IModel* model;
    std::string name;
    DataObject params;
    IModelSampler* sampler;
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

class RandomMotilityCoefficentValue : public ICalculatedValue {
public:
    RandomMotilityCoefficentValue(IDoubleCalculation* xCalc, IDoubleCalculation* yCalc, IDoubleCalculation* timeCalc, const std::string& output, int size = 100) : xCalc(xCalc), yCalc(yCalc), timeCalc(timeCalc), output(output), size(size) {}
    virtual ~RandomMotilityCoefficentValue() {
        delete xCalc;
        delete yCalc;
        delete timeCalc;
    }

    struct State : public ICalculatedState {
        std::vector<double> time;
        std::vector<double> x;
        std::vector<double> y;
    };

    ICalculatedState* createState() {
        return new State();
    }

    virtual void update(IModelSample& sample, DataObject& data, ICalculatedState* state) const  {
        State& updateState = *(static_cast<State*>(state));
        //std::cout << updateState.time.size() << std::endl;
        if (updateState.time.size() >= size) {
            std::vector<double> time;
            std::vector<double> x;
            std::vector<double> y;
            for (int i = 0; i < updateState.time.size(); i += 2) {
                time.push_back(updateState.time[i]);
                x.push_back(updateState.x[i]);
                y.push_back(updateState.y[i]);
            }
            updateState.time = time;
            updateState.x = x;
            updateState.y = y;
        }

        double t = timeCalc->calculate(sample, sample.getNavigation())/60.0;
        double lastTime = updateState.time.size() > 0 ? updateState.time[updateState.time.size()-1] : -1;
        double averageDt = updateState.time.size() > 0 ? (lastTime-updateState.time[0])/updateState.time.size() : 0.0;
        //std::cout << averageDt << std::endl;
        
        if (averageDt < t - lastTime) {
            double x = xCalc->calculate(sample, data)/1000;
            double y = yCalc->calculate(sample, data)/1000;
            updateState.time.push_back(t);
            updateState.x.push_back(x);
            updateState.y.push_back(y);
        }

        if (updateState.time.size()>1) {
            //data_dt[i] = data_time[0+i+1]-data_time[0];
            double dt = t-updateState.time[0];

            std::vector<double> data_dt;
            std::vector<double> data_msd;
            std::vector<double> data_msd_sem;
            int analysis_count = updateState.time.size();

            for (int i=0; i<analysis_count-2; i++) {
                int num_diff = analysis_count-1-i;
                data_dt.push_back(updateState.time[0+i+1]-updateState.time[0]);
                data_msd.push_back(0);
                std::vector<double> sqr_diff;
                for (int j=0; j<num_diff; j++) {
                    sqr_diff.push_back(std::pow(updateState.x[j+i+1]-updateState.x[j],2)+std::pow(updateState.y[j+i+1]-updateState.y[j],2));
                    data_msd[i] += sqr_diff[j];
                }
                data_msd[i] = data_msd[i]/num_diff;
                double std = 0;
                for (int j=0; j<num_diff; j++) {
                    std += std::pow(sqr_diff[j]-data_msd[i],2);
                }
                std = sqrt(std/(num_diff-1));
                data_msd_sem.push_back(std/sqrt(num_diff));
            }

            double cov11, chisq;
            int rmc_count = int((analysis_count-2)/2);
            double data_rmc;
            
            gsl_fit_wmul (&data_dt[0], 1, &data_msd_sem[0], 1, &data_msd[0], 1, rmc_count,&data_rmc, &cov11,
                        &chisq);
            
            data_rmc = data_rmc/4.0;

            data[output] = DoubleDataValue(data_rmc);
        }
        else {
            data[output] = DoubleDataValue(0);
        }
    }

private:
    IDoubleCalculation* xCalc;
    IDoubleCalculation* yCalc;
    IDoubleCalculation* timeCalc;
    std::string output;
    int size;
};

class NSampleMeanValue : public ICalculatedValue {
public:
    struct State : public ICalculatedState {
        State() : sample(NULL), init(false) {}
        NSample* sample;
        bool init;
    };

    NSampleMeanValue(IDoubleCalculation* valCalc, const std::string& output) : valCalc(valCalc), output(output) {}
    virtual ~NSampleMeanValue() {
        delete valCalc;
    }

    ICalculatedState* createState() {
        return new State();
    }

    virtual void update(IModelSample& sample, DataObject& data, ICalculatedState* state) const  {
        State& updateState = *(static_cast<State*>(state));
        if (!updateState.init) {
            for (IModelSample* s = &sample; s != NULL; s = s->getInnerSample()) {
                updateState.sample = dynamic_cast<NSample*>(s);
                if (updateState.sample) {
                    break;
                }
            }
            updateState.init = true;
        }

        if (updateState.sample) {
            const std::vector<IModelSample*>& samples = updateState.sample->getSamples();
            double val = 0.0;
            for (int i = 0; i < samples.size(); i++) {
                val += valCalc->calculate(*samples[i], samples[i]->getData());
            }

            data[output] = DoubleDataValue(val/samples.size());
        }
    }

private:
    IDoubleCalculation* valCalc;
    std::string output;
};

/*double sqr_diff[analysis_count];
    int num_diff;
    double std;
    
    for (int i=0; i<analysis_count-2; i++) {
        num_diff = analysis_count-1-i;
        data_dt[i] = data_time[0+i+1]-data_time[0];
        data_msd[i]=0;
        for (int j=0; j<num_diff; j++) {
            sqr_diff[j] = SQR(data_cell_pos[j+i+1][0]-data_cell_pos[j][0])+SQR(data_cell_pos[j+i+1][1]-data_cell_pos[j][1]);
            data_msd[i] += sqr_diff[j];
        }
        data_msd[i] = data_msd[i]/num_diff;
        std = 0;
        for (int j=0; j<num_diff; j++) {
            std += SQR(sqr_diff[j]-data_msd[i]);
        }
        std = sqrt(std/(num_diff-1));
        data_msd_sem[i] = std/sqrt(num_diff);
    }
    
    double cov11, chisq;
    int rmc_count = int((analysis_count-2)/2);
    
    gsl_fit_wmul (data_dt, 1, data_msd_sem, 1, data_msd, 1, rmc_count,&data_rmc, &cov11,
                  &chisq);
    
    data_rmc = data_rmc/4.0;
    //end//
*/

ExtendedModel* createExtendedModel() {
    ExtendedModel* extendedModel = new ExtendedModel("Extended Cell", new CellModel("Cell"));
    extendedModel->addCalculatedValue(new MeanValue(new KeyCalculation("aflow"), "mean_aflow"));
    extendedModel->addCalculatedValue(new MeanValue(new KeyCalculation("nm"), "mean_nm"));
    extendedModel->addCalculatedValue(new SimpleCalculatedValue(new MagnitudeCalculation(new KeyCalculation("fx"), new KeyCalculation("fx")), "fmag"));
    extendedModel->addCalculatedValue(new MeanValue(new KeyCalculation("f"), "mean_traction"));
    extendedModel->addCalculatedValue(new RandomMotilityCoefficentValue(new KeyCalculation("x"),new KeyCalculation("y"),new KeyCalculation("t"), "rmc"));
    return extendedModel;
}

int main(int argc, char* argv[]) {
    std::cout << "Usage: CellModel <port>" << std::endl;

	if (argc > 1) {
		int port = std::atoi(argv[1]);

        Server server(port);
        VLApiConnector api(&server, port);
        api.registerModel(new CellModel("Cell"));
        CompositeSampler* sampler = new CompositeSampler();
        sampler->addSampler(new AddParameterValueToSample("N", 5));
        sampler->addSampler(new RandomSampler("num"));
        sampler->addSampler(new RandomBinSampler("substrate_k", "N"));
        api.registerModel(new SampledModel(createExtendedModel(), sampler));

        ExtendedModel* extendedNCell = new ExtendedModel("Extended N-Cell", new NModel("N-Cell", createExtendedModel()));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("rmc"), "mean_rmc"));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_aflow"), "mean_aflow"));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_traction"), "mean_traction"));
        api.registerModel(extendedNCell);

        sampler = new CompositeSampler();
        sampler->addSampler(new RandomSampler("num"));
        sampler->addSampler(new RandomBinSampler("substrate_k", "N"));
        extendedNCell = new ExtendedModel("Cell Substrate", new NModel("N-Cell", createExtendedModel(), sampler));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("rmc"), "mean_rmc"));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_aflow"), "mean_aflow"));
        extendedNCell->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_traction"), "mean_traction"));

        CompositeSampler* localSampler = new CompositeSampler();
        for (DataObject::const_iterator it = extendedNCell->getParameters().begin(); it != extendedNCell->getParameters().end(); it++) {
            if (it->first != "N") {
                localSampler->addSampler(new LocalRandomSampler(it->first, 0.01));
            }
        }

        LatinHypercubeSampler* latinSampler = new LatinHypercubeSampler(5);
        for (DataObject::const_iterator it = extendedNCell->getParameters().begin(); it != extendedNCell->getParameters().end(); it++) {
            if (it->first != "N") {
                latinSampler->addParameter(it->first);
            }
        }
        //latinSampler->addParameter("cpool");
        //latinSampler->addParameter("mpool");
        
        //api.registerModel(new SampledModel(extendedNCell, latinSampler));
        api.registerModel(new SampledModel(extendedNCell, latinSampler));

        // Experiment Model
        IModel* extendedModel = createExtendedModel();
        /*extendedModel = new NModel("N-Cell", extendedModel, new IterationSampler("num","N"));
        ExtendedModel* ext = new ExtendedModel("Experiment", extendedModel);
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("rmc"), "mean_rmc"));
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_aflow"), "mean_aflow"));
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_traction"), "mean_traction"));
        extendedModel = ext;*/
        extendedModel = new NModel("Substate Model", extendedModel, new RandomBinSampler("substrate_k", "N"));
        ExtendedModel* ext = new ExtendedModel("Experiment", extendedModel);
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("rmc"), "mean_rmc"));
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_aflow"), "mean_aflow"));
        ext->addCalculatedValue(new NSampleMeanValue(new KeyCalculation("mean_traction"), "mean_traction"));
        extendedModel = ext;
        //extendedModel = new SampledModel(ext, new RandomSampler("num"));
        api.registerModel(extendedModel);

        //api.registerModel(new SampledModel(new OptimizedModel(extendedNCell,new DoubleValueDistance("mean_traction",100), 0.01, 20), localSampler));
        

        while(true) {
            server.service();
        }
    }

    return 0;
}
