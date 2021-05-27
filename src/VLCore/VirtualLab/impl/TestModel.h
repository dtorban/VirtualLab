#ifndef VIRTUALLAB_IMPL_TEST_MODEL_H_
#define VIRTUALLAB_IMPL_TEST_MODEL_H_

#include "VirtualLab/IModel.h"
#include <cmath>
#include <iostream>

namespace vl {

class TestModel : public IModel {
public:
    TestModel(const std::string& name = "TestModel") : name(name) {
        defaultParams["w"] = DoubleDataValue(1.0);
        defaultParams["a"] = DoubleDataValue(1.0);
        defaultParams["b"] = DoubleDataValue(0.0);
        defaultParams["c"] = DoubleDataValue(0.0);
    }
    virtual ~TestModel() {}

    const std::string& getName() const { return name; }
    const DataObject& getParameters() { return defaultParams; }
    IModelSample* create(const DataObject& params);

private:
    std::string name;
    DataObject defaultParams;
};

class TestModelSample : public IModelSample {
public:
    TestModelSample(const DataObject& object) {
        data["y"] = DoubleDataValue(0.0);
        yData = &(data["y"].get<double>());
        data["y2"] = DoubleDataValue(0.0);
        y2Data = &(data["y2"].get<double>());

        nav["t"] = DoubleDataValue(0.0);
        timeParam = &(nav["t"].get<double>());

        parameters = object;
        w = parameters["w"].get<double>();
        a = parameters["a"].get<double>();
        b = parameters["b"].get<double>();
        c = parameters["c"].get<double>();
    }
    virtual ~TestModelSample() {
        //delete data;
    }

    const DataObject& getParameters() const {
        return parameters;
    }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update() {
        // calculate a*cos(wt + b) + c
        (*yData) = (a * std::cos(w*(*timeParam) + b) + c);
        (*y2Data) = (*yData)*(*yData);
    }

private:
    DataObject parameters;
    DataObject nav;
    DataObject data;
    double* timeParam;
    double* yData;
    double* y2Data;
    double w;
    double a;
    double b;
    double c;
};

}

#endif