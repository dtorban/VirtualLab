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
        for (int i = 0; i < models.size(); i++) {
            delete models[i];
        }
    }
    virtual void registerModel(IModel* model) {
        models.push_back(model);
    }
    virtual void deregisterModel(IModel* model) {
        models.erase(std::remove(models.begin(), models.end(), model), models.end()); 
        delete model;
    }
    virtual const std::vector<IModel*>& getModels() { return models; }

protected:
    std::vector<IModel*> models;
};


class LoadBalancedModel : public IModel {
public:
    LoadBalancedModel() : index(0) {}
    virtual ~LoadBalancedModel() {}
    virtual const std::string& getName() const { return models[0]->getName(); }
    virtual const DataObject& getParameters() { return models[0]->getParameters(); }
    virtual IModelSample* create(const DataObject& params) {
        IModelSample* sample = models[index%models.size()]->create(params);
        std::cout << models.size() << std::endl;
        index++;
        return sample;
    }
    void addModel(IModel* model) {
        models.push_back(model);
    }
private:
    std::vector<IModel*> models;
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
    virtual const std::vector<IModel*>& getModels() {
        for (int i = 0; i < models.size(); i++) {
            delete models[i];
        }
        models.clear();

        std::map<std::string, LoadBalancedModel*> loadBalancedModels;
        const std::vector<IModel*>& apiModels = api->getModels();
        for (int i = 0; i < apiModels.size(); i++) {
            std::string name = apiModels[i]->getName();
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
    std::vector<IModel*> models;
};


}


#endif