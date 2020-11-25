#ifndef VIRTUALLAB_IMPL_TEST_MODEL_H_
#define VIRTUALLAB_IMPL_TEST_MODEL_H_

#include "VirtualLab/IModel.h"
#include <cmath>
#include <iostream>

namespace vl {

class TestModel : public IModel {
public:
    virtual ~TestModel() {}

    virtual IModelSample* create(const IQuery& query);
};

class TestModelSample : public IModelSample {
public:
    TestModelSample(CompositeDataSet* data) : data(data) {
        w = (*data)["w"].get<double>();
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
        // calculate cos(wt)
        (*yData).set<double>(std::cos(w*(*timeParam).get<double>()));
    }

private:
    CompositeDataSet time;
    CompositeDataSet* data;
    IDataSet* timeParam;
    IDataSet* yData;
    double w;
};

}

#endif