#ifndef VIRTUALLAB_IMPL_TEST_MODEL_H_
#define VIRTUALLAB_IMPL_TEST_MODEL_H_

#include "VirtualLab/IModel.h"
#include <cmath>
#include <iostream>

namespace vl {

class TestModel : public IModel {
public:
    virtual ~TestModel() {}

    const std::string& getName() const { static std::string name = "TestModel"; return name; }
    virtual IModelSample* create(const IQuery& query) const;
};

class TestModelSample : public IModelSample {
public:
    TestModelSample(CompositeDataSet* data) : data(data) {
        w = (*data)["w"].get<double>();
        a = (*data)["a"].get<double>();
        b = (*data)["b"].get<double>();
        c = (*data)["c"].get<double>();
        yData = new TypedData<double>(0.0);
        data->addData("y", yData);
        timeParam = new TypedData<double>();
        time.addData("time", timeParam);
    }
    virtual ~TestModelSample() {
        delete data;
    }

    virtual IDataSet& getNavigation() { return time; }
    virtual const IDataSet& getData() const { return *data; }

    virtual void update() {
        // calculate a*cos(wt + b) + c
        (*yData).set<double>(a * std::cos(w*(*timeParam).get<double>() + b) + c);
    }

private:
    CompositeDataSet time;
    CompositeDataSet* data;
    IDataSet* timeParam;
    IDataSet* yData;
    double w;
    double a;
    double b;
    double c;
};

}

#endif