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


}


#endif