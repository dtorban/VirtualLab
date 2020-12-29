#ifndef VIRTUALLAB_IVIRTUALLAB_API_H_
#define VIRTUALLAB_IVIRTUALLAB_API_H_

#include "VirtualLab/IModel.h"

namespace vl {

class IVirtualLabAPI {
public:
	virtual ~IVirtualLabAPI() {}
    virtual void registerModel(IModel* model) = 0;
    virtual void deregisterModel(IModel* model) = 0;
    virtual const std::vector<IModel*>& getModels() = 0;
};


}


#endif