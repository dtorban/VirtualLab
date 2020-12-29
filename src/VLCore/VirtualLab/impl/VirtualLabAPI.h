#ifndef VIRTUALLAB_IMPL_VIRTUALLAB_API_H_
#define VIRTUALLAB_IMPL_VIRTUALLAB_API_H_

#include "VirtualLab/IVirtualLabAPI.h"
#include "VirtualLab/impl/TestModel.h"

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
    virtual void registerModel(IModel* model) {}
    virtual void deregisterModel(IModel* model) {}
    virtual const std::vector<IModel*>& getModels() const { return models; }

protected:
    std::vector<IModel*> models;
};


}


#endif