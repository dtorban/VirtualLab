#ifndef VIRTUALLAB_IMODEL_H_
#define VIRTUALLAB_IMODEL_H_

#include "VirtualLab/IQuery.h"
#include "VirtualLab/IModelSample.h"

namespace vl {

class IModel {
public:
    virtual ~IModel() {}

    virtual const std::string& getName() const = 0;
    virtual const DataObject& getParameters() = 0;
    virtual IModelSample* create(const DataObject& params) = 0;
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

}

#endif