#include "VirtualLab/impl/TestModel.h"

namespace vl {

IModelSample* TestModel::create(const IQuery& query) {
    CompositeDataSet* params = new CompositeDataSet();
    params->addData("w", new TypedData<double>(1.0));
    query.setParameters(*params);
    return new TestModelSample(params);
}

}