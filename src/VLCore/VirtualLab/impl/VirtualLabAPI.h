#ifndef VIRTUALLAB_IMPL_VIRTUALLAB_API_H_
#define VIRTUALLAB_IMPL_VIRTUALLAB_API_H_

#include "VirtualLab/IVirtualLabAPI.h"
#include "VirtualLab/impl/TestModel.h"
#include <algorithm>
#include <mutex>

namespace vl {

class VirtualLabAPI : public IVirtualLabAPI {
public:
    VirtualLabAPI() {
    }
	virtual ~VirtualLabAPI() {
        for (int i = 0; i < realModels.size(); i++) {
            std::cout << realModels[i]->getName() << ": delete" << std::endl;
            delete realModels[i];
        }
    }
    virtual void registerModel(IModel* model) {
        models.push_back(ModelProxy(model));
        realModels.push_back(model);
    }
    virtual void deregisterModel(IModel* model) {
        realModels.erase(std::remove(realModels.begin(), realModels.end(), model), realModels.end());
        models.clear();
        for (int i = 0; i < realModels.size(); i++) {
            models.push_back(ModelProxy(realModels[i]));
        }
        delete model;
    }
    virtual const std::vector<ModelProxy> getModels() { return models; }

protected:
    std::vector<ModelProxy> models;
    std::vector<IModel*> realModels;
};

class IDataConsumer {
public:
    virtual ~IDataConsumer() {}
    virtual void consume(IModel& model, IModelSample& sample) = 0;
    //TODO: "const" 
    //virtual void consume(const IModel& model, const IModelSample& sample) = 0;
};

class IDataProducer {
public:
    virtual ~IDataProducer() {}
    virtual void produce(IModel& model, IModelSample& sample) = 0;
    virtual void addConsumer(IDataConsumer* consumer) = 0;
};


class ProducerSample : public AsyncModelSampleDecorator {
public:
    ProducerSample(IModelSample* sample, IModel* model, IDataProducer* producer) : AsyncModelSampleDecorator(sample), model(model), producer(producer) {}

    void asyncUpdate() {
        producer->produce(*model, *sample);
    }

private:
    IModel* model;
    IDataProducer* producer;
};

class ProducerModel : public ModelDecorator {
public:
    ProducerModel(IModel* model, IDataProducer* producer) : ModelDecorator(model), producer(producer) {}

    IModelSample* create(const DataObject& params) {
        return new ProducerSample(model->create(params), this, producer);
    }

private:
    IDataProducer* producer;
};

class ProducerAPI : public IVirtualLabAPI, public IDataProducer {
public:
    ProducerAPI(IVirtualLabAPI* api) : api(api), deleteApi(true) {}
    ProducerAPI(IVirtualLabAPI& api) : api(&api), deleteApi(false) {}

	virtual ~ProducerAPI() {
        if (deleteApi) {
            delete api;
        }
    }

    virtual void registerModel(IModel* model) {
        //api->registerModel(new ProducerModel(model, this));
        api->registerModel(model);
    }

    virtual void deregisterModel(IModel* model) {
        api->deregisterModel(model);
    }

    virtual const std::vector<ModelProxy> getModels() { 
        if (models.size() == 0) {
            std::vector<ModelProxy> apiModels = api->getModels();
            for (int i = 0; i < apiModels.size(); i++) {
                models.push_back(new ProducerModel(new ModelProxy(apiModels[i]), this));
            }
        }

        // TODO: Fix to be dynamic rather than loading the first time.

        std::vector<ModelProxy> modelProxies;
        for (int i = 0; i < models.size(); i++) {
            modelProxies.push_back(ModelProxy(models[i]));
        }

        return modelProxies;
    }

    void produce(IModel& model, IModelSample& sample) {
        //std::cout << sample.getData()["x"].get<double>() << " " << sample.getData()["y"].get<double>() << std::endl;
        for (int i = 0; i < consumers.size(); i++) {
            consumers[i]->consume(model, sample);
        }
    }

    void addConsumer(IDataConsumer* consumer) {
        consumers.push_back(consumer);
    }

protected:
    std::vector<IDataConsumer*> consumers;
    std::vector<IModel*> models;
    IVirtualLabAPI* api;
    bool deleteApi;
};


class LoadBalancedModel : public IModel {
public:
    LoadBalancedModel() : index(0) {}
    virtual ~LoadBalancedModel() {}
    virtual const std::string& getName() const { return models[0].getName(); }
    virtual const DataObject& getParameters() { return models[0].getParameters(); }
    virtual IModelSample* create(const DataObject& params) {
        IModelSample* sample = models[index%models.size()].create(params);
        std::cout << models.size() << std::endl;
        index++;
        return sample;
    }
    void addModel(ModelProxy model) {
        models.push_back(model);
    }
private:
    std::vector<ModelProxy> models;
    int index;
};

class LoadBalancedAPI : public IVirtualLabAPI {
public:
    LoadBalancedAPI(IVirtualLabAPI* api) : api(api) {}
	virtual ~LoadBalancedAPI() {}
    virtual void registerModel(IModel* model) {
        api->registerModel(model);
    }
    virtual void deregisterModel(IModel* model) {
        api->deregisterModel(model);
    }
    virtual const std::vector<ModelProxy> getModels() {
        if (models.size() > 0) {
            return models;
        }

        std::vector<ModelProxy> apiModels = api->getModels();
        for (int i = 0; i < apiModels.size(); i++) {
            std::string name = apiModels[i].getName();
            std::map<std::string, LoadBalancedModel*>::iterator it = loadBalancedModels.find(name);
            if (it == loadBalancedModels.end()) {
                LoadBalancedModel* model = new LoadBalancedModel();
                loadBalancedModels[name] = model;
                models.push_back(model);
            }
            loadBalancedModels[name]->addModel(apiModels[i]);
        }
        
        return models;
    }

protected:
    IVirtualLabAPI* api;
    std::vector<ModelProxy> models;
    std::map<std::string, LoadBalancedModel*> loadBalancedModels;
};


class CompositeAPI : public IVirtualLabAPI {
public:
    CompositeAPI() {
        apis.push_back(&impl);
    }
	virtual ~CompositeAPI() {}
    virtual void registerModel(IModel* model) {
        impl.registerModel(model);
    }
    virtual void deregisterModel(IModel* model) {
        impl.deregisterModel(model);
    }
    virtual const std::vector<ModelProxy> getModels() {
        std::vector<ModelProxy> models;

        for (int i = 0; i < apis.size(); i++) {
            std::vector<ModelProxy> apiModels = apis[i]->getModels();
            for (int j = 0; j < apiModels.size(); j++) {
                models.push_back(apiModels[j]);
            }
        }

        return models;
    }

    void addApi(IVirtualLabAPI& api) {
        apis.push_back(&api);
    }

protected:
    VirtualLabAPI impl;
    std::vector<IVirtualLabAPI*> apis;
};

class ICalculatedField {
public:
    ICalculatedField() {}
    virtual ~ICalculatedField() {}

    virtual DataValue calculate(const DataObject& data) = 0;
};

class ForceMagnitude : public ICalculatedField {
public:
    virtual DataValue calculate(const DataObject& data) {
        return DoubleDataValue(std::sqrt(std::pow(data["fx"].get<double>(),2) + std::pow(data["fy"].get<double>(),2)));
    }
};

class ModifiedSample : public ModelSampleDecorator {
private:
    class UpdateCallback : public IUpdateCallback {
	public:
		UpdateCallback(ModifiedSample* sample, IUpdateCallback* callback) : sample(sample), callback(callback) {}
        virtual ~UpdateCallback() {
            delete callback;
        }

		void onComplete() {
			sample->internalUpdate();
            callback->onComplete();
		}

	private:
		ModifiedSample* sample;
        IUpdateCallback* callback;
	};

public:
    ModifiedSample(IModelSample* sample) : ModelSampleDecorator(sample) {
        enableKey("x",false);
        enableKey("y",false);
        enableKey("fx",false);
        enableKey("fy",false);
        enableKey("free_actin",false);
        enableKey("ev",false);
        enableKey("m",false);
        addCalculatedField("f_mag", new ForceMagnitude());
        nav = sample->getNavigation();
        nav["keys"] = keys;
        data = sample->getData();
    }

    virtual ~ModifiedSample() {
        for (std::map<std::string,ICalculatedField*>::iterator it = calculatedFields.begin(); it != calculatedFields.end(); it++) {
            delete it->second;
        }
    }

    DataObject& getNavigation() { return nav; }

    const DataObject& getData() const { return data; }

    void update() {
        sample->getNavigation() = nav;
        keys = nav["keys"];
        ModelSampleDecorator::update();
        internalUpdate();
    }

    void update(IUpdateCallback* callback) {
        sample->getNavigation() = nav;
        keys = nav["keys"];
        return ModelSampleDecorator::update(new UpdateCallback(this, callback));
    }

    void addCalculatedField(const std::string& key, ICalculatedField* field) {
        calculatedFields[key] = field;
        keys[key] = DoubleDataValue(1);
    }

    void enableKey(const std::string& key, bool enabled) {
        keys[key] = DoubleDataValue(enabled);
    }

private:
    void internalUpdate() {
        sample->getData();
        for (DataObject::const_iterator it = sample->getData().begin(); it != sample->getData().end(); it++) {
            if (keys.find(it->first) == keys.end()) {
                keys[it->first] = DoubleDataValue(1);
            }
        }
        nav = sample->getNavigation();
        nav["keys"] = keys;
        //std::cout << "Modified" << std::endl;

        data = DataObject();

        for (DataObject::const_iterator it = keys.begin(); it != keys.end(); it++) {
            if (std::abs(it->second.get<double>()) > 0.00001) {
                std::map<std::string,ICalculatedField*>::iterator field = calculatedFields.find(it->first);
                if (field != calculatedFields.end()) {
                    data[it->first] = field->second->calculate(sample->getData());
                }
                else {
                    data[it->first] = sample->getData()[it->first];
                }
            }
        }

    }

    DataObject nav;
    DataObject data;
    DataObject keys;
    std::map<std::string,ICalculatedField*> calculatedFields;
};

class ParameterHelper {
public:
    ParameterHelper(DataObject* params) : params(params), const_params(params) {
        vl::Object::iterator it = params->find(".metadata");
        if (it == params->end()) {
            (*params)[".metadata"] = DataObject();
            metadata = &((*params)[".metadata"].get<vl::Object>());
        }
        else {
            metadata = &(it->second.get<vl::Object>());
        }
        const_metadata = metadata;
    }

    ParameterHelper(const DataObject& params) : params(NULL), const_params(&params), metadata(NULL) {
        vl::Object::const_iterator it = const_params->find(".metadata");
        if (it != const_params->end()) {
            const_metadata = &(it->second.get<vl::Object>());
        }
        else {
            const_metadata = NULL;
        }
    }

    void set(const std::string& param, double val, double min, double max, const std::string& scale) {
        if (params && metadata) {
            (*params)[param] = DoubleDataValue(val);
            DataObject obj;
            obj["min"] = DoubleDataValue(min);
            obj["max"] = DoubleDataValue(max);
            obj["scale"] = StringDataValue(scale);
            (*metadata)[param] = obj;
        }
    }

    void set(const std::string& param, double val, double min, double max) {
        set(param, val, min, max, "linear");
    }

    void set(const std::string& param, double val) {
        set(param, val, val, val);
    }

    double getMin(const std::string& param) const {
        return getMetaData<double>(param, "min");
    }

    double getMax(const std::string& param) const {
        return getMetaData<double>(param, "max");
    }

    std::string getScale(const std::string& param) const {
        return getMetaData<std::string>(param, "scale");
    }

    double scale(const std::string& param, double val) {
        if (getScale(param) == "log") {
            return std::log(val);
        }
        return val;
    }
    double invScale(const std::string& param, double val) {
        if (getScale(param) == "log") {
            return std::exp(val);
        }
        return val;
    }

    template<typename T>
    const T& getMetaData(const std::string& param, const std::string& key) const {
        if (const_metadata) {
            vl::Object::const_iterator it = const_metadata->find(param);
            if (it != const_metadata->end()) {
                return it->second.get<vl::Object>().find(key)->second.get<T>();
            }
        }
        static T defaultVal;
        return defaultVal;
    }

private:
    DataObject* params;
    vl::Object* metadata;
    const DataObject* const_params;
    const vl::Object* const_metadata;
};

class SamplingModelSample : public IModelSample {
public:
    class UpdateCallback : public IUpdateCallback {
	public:
		UpdateCallback(SamplingModelSample* samplingSample, IModelSample* sample, int index) : samplingSample(samplingSample), sample(sample), index(index) {}
		virtual ~UpdateCallback() {}

		void onComplete() {
            /*int t = sample->getNavigation()["t"].get<double>() + samplingSample->getParameters()["dt"].get<double>();
            sample->getNavigation()["t"].set<double>(t);
            sample->update(new UpdateCallback(samplingSample, sample));*/

            std::unique_lock<std::mutex> lock(samplingSample->updateMutex);
            //std::cout << sample << " Update callbacks" << std::endl;
            double t = sample->getNavigation()["t"].get<double>();
            double progress = (t-samplingSample->start)/(samplingSample->end-samplingSample->start);
            vl::Object& sampleData = (*samplingSample->sampleInfoArray)[index].get<vl::Object>();
            sampleData["progress"].set<double>(progress);
            int status = std::floor(sampleData["status"].get<double>());
            if (status < 2) {
                sampleData["status"].set<double>(2);
            }
            if (progress >= 1.0) {
                sampleData["status"].set<double>(3);
                //TODO: delete sample
                //delete samplingSample->samples[index];
                samplingSample->createSample();
            }
            else {
                samplingSample->updateQueue.push_back(std::pair<IModelSample*, int>(sample, index));
            }
            lock.unlock();
            samplingSample->update();
		}

	private:
		SamplingModelSample* samplingSample;
        IModelSample* sample;
        int index;
	};

public:
    SamplingModelSample(ModelProxy model, const DataObject& params) : model(model), params(params), callback(NULL) {
        data["samples"] = DataArray();
        sampleInfoArray = &(data["samples"].get<vl::Array>());
        start = params["start"].get<double>();
        end = params["end"].get<double>();
        dt = params["dt"].get<double>();
        numSamples = params["samples"].get<double>();
        nav["p"] = DoubleDataValue(0);
    }
    virtual ~SamplingModelSample() {
        for (int i = 0; i < samples.size(); i++) {
            delete samples[i];
        }
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }
    virtual void update() {
        if (callback) {
            callback->onComplete();
            delete callback;
            callback = NULL;
        }
    }
    virtual DataObject calculateParams(const DataObject& params) {
        DataObject p = params;
        if (samples.size() % (numSamples/2) == 0) {
        //if (true) {
            ParameterHelper helper(&p);
            for (vl::Object::iterator it = p.begin(); it != p.end(); it++) {
                if (it->second.isType<double>()) {
                    std::string param = it->first;
                    double max = helper.scale(param, helper.getMax(param));
                    double min = helper.scale(param, helper.getMin(param));
                    double r = (double)std::rand() / (double)RAND_MAX;
                    double value = r*(max - min) + min;
                    value = helper.invScale(param, value);
                    p[param].set<double>(value);
                }
            }
        }
        else {
            p = samples[samples.size()-1]->getParameters();
            ParameterHelper helper(&p);
            {
                std::string param = "substrate_k";
                double max = helper.scale(param, helper.getMax(param));
                double min = helper.scale(param, helper.getMin(param));
                double r = (double)std::rand() / (double)RAND_MAX;
                double value = r*(max - min) + min;
                value = helper.invScale(param, value);
                p[param].set<double>(value);
            }
            /*{
                std::string param = "num";
                double max = helper.scale(param, helper.getMax(param));
                double min = helper.scale(param, helper.getMin(param));
                double r = (double)std::rand() / (double)RAND_MAX;
                double value = r*(max - min) + min;
                value = helper.invScale(param, value);
                p[param].set<double>(value);
            }*/
        }

        return p;
    }
    virtual void createSample() {
        int i = samples.size();
        IModelSample* sample = model.create(calculateParams(model.getParameters()));
        DataObject obj;
        obj["status"] = DoubleDataValue(0);
        DataObject details;
        details["params"] = sample->getParameters();
        obj["details"] = details;
        obj["progress"] = DoubleDataValue(0);
        sampleInfoArray->push_back(obj);
        samples.push_back(sample);
        sample->getNavigation()["t"].set<double>(start-dt);
        //std::cout << sample << " Call update start" << std::endl;
        //sample->update(new UpdateCallback(this, sample, i));
        updateQueue.push_back(std::pair<IModelSample*, int>(sample, i));
    }
    virtual void update(IUpdateCallback* callback) {
        this->callback = callback;

        if (samples.size() == 0) {
            numSamples = params["samples"].get<double>();
            for (int i = 0; i < numSamples; i++) {
                createSample();
            }

            callback->onComplete();
            delete callback;
        }
        else {
            std::unique_lock<std::mutex> lock(updateMutex);
            for (int i = 0; i < samples.size(); i++) {
                vl::Object& sampleData = (*sampleInfoArray)[i].get<vl::Object>();
                int status = std::floor(sampleData["status"].get<double>());
                if (status == 0) {
                    sampleData["status"].set<double>(1);
                    sampleData["details"] = DataObject();
                }
            }
            std::vector< std::pair<IModelSample*, int> > currentQueue = updateQueue;
            updateQueue.clear();
            //std::cout << "updateQueue cleared" << std::endl;

            for (int i = 0; i < currentQueue.size(); i++) {
                IModelSample* sample = currentQueue[i].first;
                int index = currentQueue[i].second;
                int t = sample->getNavigation()["t"].get<double>() + dt;
                sample->getNavigation()["t"].set<double>(t);
            }

            lock.unlock();

            for (int i = 0; i < currentQueue.size(); i++) {
                IModelSample* sample = currentQueue[i].first;
                //std::cout << sample << " Call update" << std::endl;
                sample->update(new UpdateCallback(this, sample, currentQueue[i].second));
                //std::cout << sample << " Update called" << std::endl;
            }

        }
    }

private:
    ModelProxy model;
    DataObject params;
    DataObject nav;
    DataObject data;
    vl::Array* sampleInfoArray;
    IUpdateCallback* callback;
    std::vector<IModelSample*> samples;
    std::vector< std::pair<IModelSample*, int> > updateQueue;
    std::mutex updateMutex;
    double start;
    double end;
    double dt;
    int numSamples;
};

class SamplingModel : public IModel {
public:
    SamplingModel(const std::string& name, ModelProxy model) : name(name), model(model) {
        params["model"] = model.getParameters();
        ParameterHelper helper(&params);
        helper.set("start", 10, 0, 6*3600);
        helper.set("end", 600, 0, 10*3600);
        //helper.set("end", 6*3600, 0, 10*3600);
        helper.set("dt", 10, 1, 3600);
        helper.set("samples", 8, 1, 20);
    }

    virtual ~SamplingModel() {}

    const std::string& getName() const { return name; }
    const DataObject& getParameters() { return params; }
    IModelSample* create(const DataObject& params) {
        return new SamplingModelSample(model, params);
    }

private:
    DataObject params;
    std::string name;
    ModelProxy model;
};

class CompositeSampler : public IModelSampler {
public:
    CompositeSampler() {}
    virtual ~CompositeSampler() {
        for (int i = 0; i < samplers.size(); i++) {
            delete samplers[i];
        }
    }

    void addSampler(IModelSampler* sampler) {
        samplers.push_back(sampler);
    }

    void sample(DataObject& params) {
        for (int i = 0; i < samplers.size(); i++) {
            samplers[i]->sample(params);
        }
    }
private:
    std::vector<IModelSampler*> samplers;
};

class StructuredSampler : public IModelSampler {
public:
    StructuredSampler(const std::string& param, int resolution) : param(param), resolution(resolution), count(0) {}
    void sample(DataObject& params) {
        ParameterHelper helper(params);
        double max = helper.scale(param, helper.getMax(param));
        double min = helper.scale(param, helper.getMin(param));
        double binSize = (max-min)/resolution;
        double val = min + binSize*(count%resolution) + binSize/2.0;
        val = helper.invScale(param, val);
        params[param].set<double>(val);
        count++;
    }
private:
    std::string param;
    int resolution;
    int count;
};

class RandomSampler : public IModelSampler {
public:
    RandomSampler(const std::string& param) : param(param) {}
    void sample(DataObject& params) {
        ParameterHelper helper(params);
        double max = helper.scale(param, helper.getMax(param));
        double min = helper.scale(param, helper.getMin(param));
        double r = (double)std::rand() / (double)RAND_MAX;
        double val = min + r*(max-min);
        val = helper.invScale(param, val);
        params[param].set<double>(val);
    }
private:
    std::string param;
};

class SampledModel : public ModelDecorator {
public:
    SampledModel(IModel* model, IModelSampler* sampler) : ModelDecorator(model), sampler(sampler) {}
    virtual ~SampledModel() {
        delete sampler;
    }

    IModelSample* create(const DataObject& params) {
        DataObject p = params;
        sampler->sample(p);
        return ModelDecorator::create(p);
    }

private:
    IModelSampler* sampler;
};

}


#endif