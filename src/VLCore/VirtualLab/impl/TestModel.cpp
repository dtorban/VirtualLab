#include "VirtualLab/impl/TestModel.h"

namespace vl {

IModelSample* TestModel::create(const DataObject& params) const {
    return new TestModelSample(params);
}

}