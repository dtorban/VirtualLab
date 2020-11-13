#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/DataSet.h"
#include "VirtualLab/IModelSample.h"
#include <vector>
#include <cstdlib>
#include <cmath>

namespace vl {

class ISamplingStrategy {
public:
    virtual ~ISamplingStrategy() {}

    virtual void setParams(DataSet& params) const = 0;
    virtual bool isValidSample(const IModelSample& sample) const { return true; }
};

class NoSamplingStrategy : public ISamplingStrategy {
public:
    void setParams(DataSet& params) const {}
};

class CompositeSampler : public ISamplingStrategy {
public:
    virtual ~CompositeSampler() {
        for (int i = 0; i < strategies.size(); i++) {
            delete strategies[i];
        }
    }

    void addStrategy(ISamplingStrategy* strategy) { strategies.push_back(strategy); }

    void setParams(DataSet& params) const {
        for (int i = 0; i < strategies.size(); i++) {
            strategies[i]->setParams(params);
        }
    }
    bool isValidSample(const IModelSample& sample) const {
        for (int i = 0; i < strategies.size(); i++) {
            if (!strategies[i]->isValidSample(sample)) {
                return false;
            }
        }

        return true;
    }

private:
    std::vector<ISamplingStrategy*> strategies;
};

template <typename T>
class Scale {
public:
    virtual ~Scale() {}
    virtual T scale(T val) const = 0;
    virtual T invert(T val) const = 0;
};

template <typename T>
class LinearScale : public Scale<T> {
public:
    virtual ~LinearScale() {}
    virtual T scale(T val) const { return val; }
    virtual T invert(T val) const { return val; }
    static const Scale<T>& instance() {
        static LinearScale<T> inst;
        return inst;
    } 
};

template <typename T>
class LogScale : public Scale<T> {
public:
    virtual ~LogScale() {}
    virtual T scale(T val) const { return std::log(val); }
    virtual T invert(T val) const { return std::exp(val); }
    static const Scale<T>& instance() {
        static LogScale<T> inst;
        return inst;
    } 
};

template <typename T>
class RangeSampler : public ISamplingStrategy {
public:
    RangeSampler(const std::string& name, T min, T max) : name(name), min(min), max(max) {}
    void setParams(DataSet& params) const {
        params[name].set<T>(getValue(min, max));
    }

protected:
    virtual T getValue(T min, T max) const = 0;

private:
    std::string name;
    T min, max;
};

template <typename T>
class RandomSampler : public RangeSampler<T> {
public:
    RandomSampler(const std::string& name, T min, T max, const Scale<T>& scale) : RangeSampler<T>(name, scale.scale(min), scale.scale(max)), scale(scale) {}
protected:
    T getValue(T min, T max) const {
        double r = (double)std::rand() / (double)RAND_MAX;
        return scale.invert(r * (max - min) + min);
    }

    const Scale<T>& scale;
};


}

#endif