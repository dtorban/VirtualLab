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
        virtual double calculate(const IModelSample& a) const = 0;
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
    double calculate(const IModelSample& a) const {
        //vl::Array arr = a.getData()["samples"].get<vl::Array>();
        //double x = arr[0].get<vl::Object>()[key].get<double>();
        double x = a.getData()[key].get<double>();
        //return std::fabs(val-(x2-x1));
        return std::fabs(val-x);
    }

private:
    std::string key;
    double val;
};

}

#endif