#ifndef VIRTUALLAB_IMODEL_H_
#define VIRTUALLAB_IMODEL_H_

#include "VirtualLab/IQuery.h"
#include "VirtualLab/IModelSample.h"

namespace vl {

class IModelSampler {
public:
    virtual ~IModelSampler() {}
    virtual void sample(DataObject& params) = 0;
    virtual void reset() = 0;
    virtual bool hasNext() = 0;
    virtual void next() = 0;
};


class IModel {
public:
    virtual ~IModel() {}

    virtual const std::string& getName() const = 0;
    virtual const DataObject& getParameters() = 0;
    virtual IModelSample* create(const DataObject& params) = 0;
    IModelSample* create(const DataObject& params, IModelSampler& sampler) {
        DataObject p = params;
        if (!sampler.hasNext()) {
            sampler.reset();
        }
        sampler.sample(p);
        sampler.next();
        return create(p);
    }
    IModelSample* create(IModelSampler& sampler) {
        return create(getParameters(), sampler);
    }
};

class ModelProxy : public IModel {
public:
    ModelProxy() : model(NULL) {}
    ModelProxy(IModel* model) : model(model) {}
    ModelProxy(const ModelProxy& proxy) : model(proxy.model) {}
    void operator=(const ModelProxy& proxy) { model = proxy.model; }
    virtual ~ModelProxy() {}
    const std::string& getName() const { return model->getName(); }
    const DataObject& getParameters() { return model->getParameters(); }
    IModelSample* create(const DataObject& params) { return model->create(params); }

private:
    IModel* model;
};

class ModelDecorator : public IModel {
public:
    ModelDecorator(IModel* model) : model(model) {}
    virtual ~ModelDecorator() { delete model; }
    virtual const std::string& getName() const { return model->getName(); }
    virtual const DataObject& getParameters() { return model->getParameters(); }
    virtual IModelSample* create(const DataObject& params) { return model->create(params); }

protected:
    IModel* model;
};

template<typename T>
class TypedModelDecorator : public ModelDecorator {
public:
    TypedModelDecorator(const std::string& name, IModel* model) : ModelDecorator(model), name(name) {}
    const std::string& getName() const { return name; }
    virtual IModelSample* create(const DataObject& params) { return new T(model->create(params)); }

private:
    std::string name;
};


}

#endif