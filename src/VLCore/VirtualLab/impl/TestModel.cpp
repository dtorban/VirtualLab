#include "VirtualLab/impl/TestModel.h"

namespace vl {

IModelSample* TestModel::create(const IQuery& query) const {
    DataObject parameters;
    parameters.get<Object>()["w"] = DoubleDataValue(1.0);
    parameters.get<Object>()["a"] = DoubleDataValue(1.0);
    parameters.get<Object>()["b"] = DoubleDataValue(0.0);
    parameters.get<Object>()["c"] = DoubleDataValue(0.0);
    query.setParameters(parameters);

    return new TestModelSample(parameters);
}

}