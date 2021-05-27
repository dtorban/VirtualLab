#ifndef VIRTUALLAB_IMPL_VIRTUALLAB_API_H_
#define VIRTUALLAB_IMPL_VIRTUALLAB_API_H_

#include "VirtualLab/IVirtualLabAPI.h"
#include "VirtualLab/impl/TestModel.h"
#include <algorithm>

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


class ProducerSample : public ModelSampleDecorator {
public:
    ProducerSample(IModelSample* sample, IModel* model, IDataProducer* producer) : ModelSampleDecorator(sample), model(model), producer(producer) {}

    void update() {
        ModelSampleDecorator::update();
        producer->produce(*model, *sample);
    }

    void update(IUpdateCallback* callback) {
        ModelSampleDecorator::update(callback);
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


/*class LoadBalancedModel : public IModel {
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
        models.clear();

        std::map<std::string, LoadBalancedModel*> loadBalancedModels;
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
};*/


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

}


#endif