#ifndef VIRTUALLAB_IMPL_EXTENDED_MODEL_H_
#define VIRTUALLAB_IMPL_EXTENDED_MODEL_H_

#include "VirtualLab/IModel.h"
#include <cmath>
#include <iostream>

namespace vl {

class ExtendedModelSample;

class ICalculatedState {
public:
    virtual ~ICalculatedState() {}
};

class ICalculatedValue {
public:
    virtual ~ICalculatedValue() {}
    virtual ICalculatedState* createState() = 0;
    virtual void update(IModelSample& sample, DataObject& data, ICalculatedState* state) const = 0;
};

class ExtendedModel : public ModelDecorator {
public:
    ExtendedModel(const std::string& name, IModel* model) : ModelDecorator(model), name(name) {}
    virtual ~ExtendedModel() {
        for (int i = 0; i < calculatedValues.size(); i++) {
            delete calculatedValues[i];
        }
    }
    const std::string& getName() const { return name; }
    virtual IModelSample* create(const DataObject& params);

    void addCalculatedValue(ICalculatedValue* calcValue) {
        calculatedValues.push_back(calcValue);
    }
    const std::vector<ICalculatedValue*>& getCalculatedValues() const { return calculatedValues; }

private:
    std::string name;
    std::vector<ICalculatedValue*> calculatedValues;
};

class ExtendedModelSample : public AsyncModelSampleDecorator {
public:
    ExtendedModelSample(ExtendedModel* model, IModelSample* sample) : AsyncModelSampleDecorator(sample), model(model), sample(sample) {}
    virtual ~ExtendedModelSample() {
        for (std::map<ICalculatedValue*, ICalculatedState*>::iterator it = states.begin(); it != states.end(); it++) {
            if (it->second) {
                delete it->second;
            }
        }
    }

    virtual const DataObject& getData() const { return data; }

    void asyncUpdate() {
        data = sample->getData();
        const std::vector<ICalculatedValue*>& calculatedValues = model->getCalculatedValues();
        for (int i = 0; i < calculatedValues.size(); i++) {
            std::map<ICalculatedValue*, ICalculatedState*>::iterator it = states.find(calculatedValues[i]);
            ICalculatedState* state = NULL;
            if (it != states.end()) {
                state = it->second;
            }
            else {
                state = calculatedValues[i]->createState();
                states[calculatedValues[i]] = state;
            }
            calculatedValues[i]->update(*this, data, state);
        }
    }

private:
    DataObject data;
    ExtendedModel* model;
    IModelSample* sample;
    std::map<ICalculatedValue*, ICalculatedState*> states;
};


IModelSample* ExtendedModel::create(const DataObject& params) { return new ExtendedModelSample(this, model->create(params)); }

class IDoubleCalculation {
public:
    virtual ~IDoubleCalculation() {}
    virtual double calculate(const IModelSample& sample, const DataObject& data) const = 0;
};

class KeyCalculation : public IDoubleCalculation {
public:
    KeyCalculation(const std::string& key) : key(key) {}
    virtual ~KeyCalculation() {}
    virtual double calculate(const IModelSample& sample, const DataObject& data) const {
        return data[key].get<double>();
    }

private:
    std::string key;
};

class ParamCalculation : public IDoubleCalculation {
public:
    ParamCalculation(const std::string& key) : key(key) {}
    virtual ~ParamCalculation() {}
    virtual double calculate(const IModelSample& sample, const DataObject& data) const {
        return sample.getParameters()[key].get<double>();
    }

private:
    std::string key;
};

class MagnitudeCalculation : public IDoubleCalculation {
public:
    MagnitudeCalculation(IDoubleCalculation* a, IDoubleCalculation* b) : a(a), b(b) {}
    virtual ~MagnitudeCalculation() {
        delete a;
        delete b;
    }
    virtual double calculate(const IModelSample& sample, const DataObject& data) const {
        return std::sqrt(std::pow(a->calculate(sample, data), 2) + std::pow(b->calculate(sample, data), 2));
    }

private:
    IDoubleCalculation* a, *b;
};


class MeanValue : public ICalculatedValue {
public:
    MeanValue(IDoubleCalculation* calc, const std::string& output) : calc(calc), output(output) {}
    virtual ~MeanValue() {
        delete calc;
    }

    struct State : public ICalculatedState {
        State() : total(0.0), numSamples(0.0) {}
        double total;
        int numSamples;
    };

    ICalculatedState* createState() {
        return new State();
    }

    virtual void update(IModelSample& sample, DataObject& data, ICalculatedState* state) const  {
        State& updateState = *(static_cast<State*>(state));
        updateState.numSamples++;
        updateState.total += calc->calculate(sample, data);
        data[output] = DoubleDataValue(updateState.total/updateState.numSamples);
    }

private:
    IDoubleCalculation* calc;
    std::string output;
};

class SimpleCalculatedValue : public ICalculatedValue {
public:
    SimpleCalculatedValue(IDoubleCalculation* calc, const std::string& output) : calc(calc), output(output) {}
    virtual ~SimpleCalculatedValue() {
        delete calc;
    }

    ICalculatedState* createState() {
        return NULL;
    }

    virtual void update(IModelSample& sample, DataObject& data, ICalculatedState* state) const  {
        data[output] = DoubleDataValue(calc->calculate(sample, data));
    }

private:
    IDoubleCalculation* calc;
    std::string output;
};

}

#endif