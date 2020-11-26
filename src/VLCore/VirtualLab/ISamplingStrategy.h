#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/IDataSet.h"
#include <cstdlib>
#include <cmath>
#include <iostream>

namespace vl {

class ISamplingStrategy {
public:
    virtual ~ISamplingStrategy() {}

    virtual void setParameters(IDataSet& params, DataSetStack& metaData) = 0;
};

class CompositeSamplingStrategy : public ISamplingStrategy {
public:
    CompositeSamplingStrategy(bool changeScope = true) : changeScope(changeScope) {}
    ~CompositeSamplingStrategy() {
        for (ISamplingStrategy* strategy : strategies) {
            delete strategy;
        }
    }
    void addStrategy(ISamplingStrategy* strategy) { strategies.push_back(strategy); }

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        if (changeScope) {
            metaData.push();
        }
        for (ISamplingStrategy* strategy : strategies) {
            strategy->setParameters(params, metaData);
        }
        if (changeScope) {
            metaData.pop();
        }
    }

private:
    std::vector<ISamplingStrategy*> strategies;
    bool changeScope;
};

template <typename T>
class SetSamplingRange : public ISamplingStrategy {
public:
    SetSamplingRange(const std::string& key, T min, T max) : key(key), min(min), max(max) {}

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        std::string minKey = getMinKey(key);
        std::string maxKey = getMaxKey(key);
        if (!metaData.top().containsKey(minKey)) {
            metaData.addData(minKey, new TypedData<T>());
        }
        if (!metaData.top().containsKey(maxKey)) {
            metaData.addData(maxKey, new TypedData<T>());
        }
        metaData[minKey].set<T>(min);
        metaData[maxKey].set<T>(max);
    }

    static std::string getMinKey(const std::string& key) {
        return key + "_min";
    }

    static std::string getMaxKey(const std::string& key) {
        return key + "_max";
    }

private:
    T min;
    T max;
    std::string key;
};

class Scale {
public:
    virtual double apply(double val) { return val; }
    virtual double invert(double val) { return val; }
};

class LogrithmicScale : public Scale {
public:
    double apply(double val) { return std::log(val); }
    double invert(double val) { return std::exp(val); }
};

class SetSamplingScale : public ISamplingStrategy {
public:
    SetSamplingScale(const std::string& key, Scale* scale) : key(key), scale(scale) {}
    ~SetSamplingScale() { delete scale; }

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        std::string scaleKey = getScaleKey(key);

        if (!metaData.containsKey(scaleKey)) {
            metaData.addData(scaleKey, new TypedData<Scale*>());
        }

        metaData[scaleKey].set<Scale*>(scale);
    }

    static std::string getScaleKey(const std::string& key) {
        return key + "_scale";
    }

private:
    Scale* scale;
    std::string key;
};

template <typename T>
class RandomSampler : public ISamplingStrategy {
public:
    RandomSampler(const std::string& key) : key(key) {}

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        std::string minKey = SetSamplingRange<T>::getMinKey(key);
        std::string maxKey = SetSamplingRange<T>::getMaxKey(key);
        std::string scaleKey = SetSamplingScale::getScaleKey(key);

        if (!(metaData.containsKey(minKey) && metaData.containsKey(maxKey))) {
            return;
        }

        static Scale linearScale;
        Scale* scale = &linearScale;
        if (metaData.containsKey(scaleKey)) {
            scale = metaData[scaleKey].get<Scale*>();
            std::cout << "found scale" << std::endl;
        }

        T min = metaData[minKey].get<T>();
        T max = metaData[maxKey].get<T>();
        min = scale->apply(min);
        max = scale->apply(max);

        double r = (double)std::rand() / (double)RAND_MAX;
        T value = r*(max - min) + min;
        value = scale->invert(value);

        params[key].set<T>(value);
    }

private:
    T min;
    T max;
    std::string key;
};

}

#endif