#ifndef VIRTUALLAB_OPT_OPTIMIZED_MODEL_H_
#define VIRTUALLAB_OPT_OPTIMIZED_MODEL_H_

#include "VirtualLab/IModel.h"
#include "VirtualLab/impl/VirtualLabAPI.h"
#include <cmath>
#include <mutex>

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
    
    
class OptimizedModel2 : public ModelDecorator {
public:

    OptimizedModel2(IModel* model, double closeness, int numSamples);
    
    virtual ~OptimizedModel2();
    
    const std::string& getName() const { return name; }

    IModelSample* create(const DataObject& params);

private:
    std::string name;
    int numTestSamples;
    double closeness;
};

class ForceModel : public ModelDecorator {
public:

    ForceModel(IModel* model, double closeness, int numSamples);
    
    virtual ~ForceModel();
    
    const std::string& getName() const { return name; }

    IModelSample* create(const DataObject& params);

private:
    std::string name;
    int numTestSamples;
    double closeness;
};

}

#endif