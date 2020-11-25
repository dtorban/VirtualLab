#include "VirtualLab/impl/TestModel.h"

namespace vl {

IModelSample* TestModel::create(const IQuery& query) {
    CompositeDataSet* params = new CompositeDataSet();
    params->addData("w", new TypedData<double>(1.0));
    params->addData("a", new TypedData<double>(1.0));
    params->addData("b", new TypedData<double>(0.0));
    params->addData("c", new TypedData<double>(0.0));
    query.setParameters(*params);
    return new TestModelSample(params);
}

}