#ifndef VIRTUALLAB_PCA_PCA_MODEL_H_
#define VIRTUALLAB_PCA_PCA_MODEL_H_

#include "VirtualLab/IModel.h"

namespace vl {

class PCAModel : public IModel {
public:
    PCAModel(const std::string& name, IModel* model) : name(name), model(model) {
        params = model->getParameters();
    }
    virtual ~PCAModel() {
        delete model;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params);

private:
    IModel* model;
    std::string name;
    DataObject params;
};


}

#endif