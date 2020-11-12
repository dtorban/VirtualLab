#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/DataSet.h"
#include "VirtualLab/IModelSample.h"
#include <vector>
#include <cstdlib>

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
class RandomLinearSampler : public RangeSampler<T> {
public:
    RandomLinearSampler(const std::string& name, T min, T max) : RangeSampler<T>(name, min, max) {}
protected:
    T getValue(T min, T max) const {
        double r = (double)std::rand() / (double)RAND_MAX;
        return r * (max - min) + min;
    }
};

}

#endif