#ifndef VIRTUALLAB_IMPL_TEST_MODEL_H_
#define VIRTUALLAB_IMPL_TEST_MODEL_H_

#include "VirtualLab/IModel.h"
#include <cmath>
#include <iostream>

namespace vl {

class TestModel : public IModel {
public:
    TestModel(const std::string& name = "TestModel") : name(name) {}
    virtual ~TestModel() {}

    const std::string& getName() const { return name; }
    virtual IModelSample* create(const IQuery& query) const;

private:
    std::string name;
};

class TestModelSample : public IModelSample {
public:
    TestModelSample(const DataObject& object) {
        data["y"] = DoubleDataValue(0.0);
        yData = &(data["y"].get<double>());

        nav["time"] = DoubleDataValue(0.0);
        timeParam = &(nav["time"].get<double>());

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
    }

private:
    DataObject parameters;
    DataObject nav;
    DataObject data;
    double* timeParam;
    double* yData;
    double w;
    double a;
    double b;
    double c;
};

}

#endif