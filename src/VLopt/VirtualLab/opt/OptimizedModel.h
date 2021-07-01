#ifndef VIRTUALLAB_OPT_OPTIMIZED_MODEL_H_
#define VIRTUALLAB_OPT_OPTIMIZED_MODEL_H_

#include "VirtualLab/IModel.h"
#include "VirtualLab/impl/VirtualLabAPI.h"
#include <cmath>

namespace vl {

class OptimizedModel : public ModelDecorator {
public:
    class DistanceFunction {
    public:
        virtual ~DistanceFunction() {}
        virtual double calculate(const IModelSample& a, const IModelSample& b) const = 0;
    };

    OptimizedModel(IModel* model, DistanceFunction* distFunction, double closeness, int numTestSamples);
    virtual ~OptimizedModel();

    IModelSample* create(const DataObject& params);

private:
    DistanceFunction* distFunction;
    int numTestSamples;
    double closeness;
};

class DoubleValueDistance : public OptimizedModel::DistanceFunction {
public:
    DoubleValueDistance(const std::string& key, double val) : key(key), val(val) {}
    double calculate(const IModelSample& a, const IModelSample& b) const {
        double x1 = a.getData()[key].get<double>();
        double x2 = b.getData()[key].get<double>();
        //return std::fabs(val-(x2-x1));
        return std::fabs(val - x2);
    }

private:
    std::string key;
    double val;
};

}

#endif