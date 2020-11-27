#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/IDataSet.h"
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <queue>

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

class RandomSamplingContext {
public:
    RandomSamplingContext(IDataSet& params, DataSetStack& metaData) : params(params), metaData(metaData) {}

    template <typename T>
    T getValue(const std::string& key, const std::string& type) const {
        return metaData[key + type].get<T>(); 
    }
    template <typename T>
    void setValue(const std::string& key, const std::string& type, T value) {
        std::string metaDataKey = key + type;
        if (!metaData.top().containsKey(metaDataKey)) {
            metaData.addData(metaDataKey, new TypedData<T>());
        }
        metaData[metaDataKey].set<T>(value);
    }
    bool containsType(const std::string& key, const std::string& type) const { return metaData.containsKey(key + type); }

    template <typename T>
    T getMin(const std::string& key) const { return getValue<T>(key, getMinType()); }
    template <typename T>
    void setMin(const std::string& key, T value) { setValue<T>(key, getMinType(), value); }
    static std::string getMinType() { return "_min"; }

    template <typename T>
    T getMax(const std::string& key) const { return getValue<T>(key, getMaxType()); }
    template <typename T>
    void setMax(const std::string& key, T value) { setValue<T>(key, getMaxType(), value); }
    static std::string getMaxType() { return "_max"; }

    Scale* getScale(const std::string& key) const { return getValue<Scale*>(key, getScaleType()); }
    void setScale(const std::string& key, Scale* value) { setValue<Scale*>(key, getScaleType(), value); }
    static std::string getScaleType() { return "_scale"; }

    int getBin(const std::string& key) const {
        bool contains = containsType(key, getBinType());
        if (contains) {
            return getValue<int>(key, getBinType());
        }
        return 0;
    }
    void setBin(const std::string& key, int value) { setValue<int>(key, getBinType(), value); }
    static std::string getBinType() { return "_bin"; }

    int getNumBins(const std::string& key) const { 
        bool contains = containsType(key, getNumBinsType());
        if (contains) {
            return getValue<int>(key, getNumBinsType());
        }
        return 1;
    }
    void setNumBins(const std::string& key, int value) { setValue<int>(key, getNumBinsType(), value); }
    static std::string getNumBinsType() { return "_numBins"; }

private:
    IDataSet& params;
    DataSetStack& metaData;
};

template <typename T>
class SetSamplingRange : public ISamplingStrategy {
public:
    SetSamplingRange(const std::string& key, T min, T max) : key(key), min(min), max(max) {}

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        RandomSamplingContext context(params, metaData);
        context.setMin<T>(key, min);
        context.setMax<T>(key, max);
    }

private:
    T min;
    T max;
    std::string key;
};


class SetSamplingScale : public ISamplingStrategy {
public:
    SetSamplingScale(const std::string& key, Scale* scale) : key(key), scale(scale) {}
    ~SetSamplingScale() { delete scale; }

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        RandomSamplingContext context(params, metaData);
        context.setScale(key, scale);
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
        RandomSamplingContext context(params, metaData);

        if (!(context.containsType(key, context.getMinType()) && context.containsType(key, context.getMaxType()))) {
            return;
        }

        int bins = context.getNumBins(key);
        int bin = context.getBin(key);

        static Scale linearScale;
        Scale* scale = &linearScale;
        if (context.containsType(key, context.getScaleType())) {
            scale = context.getScale(key);
        }

        T min = context.getMin<T>(key);
        T max = context.getMax<T>(key);
        min = scale->apply(min);
        max = scale->apply(max);

        T binSize = (max-min)/(double)bins;
        min = binSize*bin + min;
        max = min + binSize;

        std::cout << key << " " << min << " " << max << " " << binSize << std::endl;

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

class LatinHypercubeSampler : public CompositeSamplingStrategy {
public:
    LatinHypercubeSampler(int bins) : CompositeSamplingStrategy(false), bins(bins) {}

    void addKey(const std::string& key) {
        keys.push_back(key);
    }

    void setParameters(IDataSet& params, DataSetStack& metaData) {
        metaData.push();

        if (sampleQueue.empty()) {

        }

        CompositeSamplingStrategy::setParameters(params, metaData);

        metaData.pop();
    }

private:
    std::queue< std::vector<int> > sampleQueue;
    std::vector<std::string> keys;
    int bins;
};

}

#endif