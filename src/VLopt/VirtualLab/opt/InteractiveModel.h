#ifndef VIRTUALLAB_OPT_INTERACTIVE_MODEL_H_
#define VIRTUALLAB_OPT_INTERACTIVE_MODEL_H_

#include "VirtualLab/IModel.h"
#include "VirtualLab/impl/VirtualLabAPI.h"
#include <cmath>
#include <mutex>

namespace vl {

class InteractiveModel : public ModelDecorator {
public:

    InteractiveModel(IModel* model, int parameterResolution);
    
    virtual ~InteractiveModel();
    
    const std::string& getName() const { return name; }

    IModelSample* create(const DataObject& params);

private:
    std::string name;
    int parameterResolution;
};

}

#endif