#include "VirtualLab/impl/TestModel.h"

namespace vl {

IModelSample* TestModel::create(const DataObject& params) {
    return new TestModelSample(params);
}

}