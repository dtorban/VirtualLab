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
    RangeSampler(const std::string& name, T min, T max, const Scale<T>& scale) : name(name), min(scale.scale(min)), max(scale.scale(max)), scale(scale) {}
    void setParams(DataSet& params) const {
        params[name].set<T>(scale.invert(getValue(min, max)));
    }

protected:
    virtual T getValue(T min, T max) const = 0;
    const Scale<T>& scale;
private:
    std::string name;
    T min, max;
};

template <typename T>
class RandomSampler : public RangeSampler<T> {
public:
    RandomSampler(const std::string& name, T min, T max, const Scale<T>& scale) : RangeSampler<T>(name, min, max, scale) {}
protected:
    T getValue(T min, T max) const {
        double r = (double)std::rand() / (double)RAND_MAX;
        return r * (max - min) + min;
    }
};

template <typename T>
class BinnedRandomSampler : public ISamplingStrategy {
public:
    BinnedRandomSampler(const std::string& name, T min, T max, const Scale<T>& scale, int numBins, int bin) 
        : sampler(name, calcBinPos(min, max, scale, numBins, bin), calcBinPos(min, max, scale, numBins, bin+1), scale) {}
    void setParams(DataSet& params) const {
        sampler.setParams(params);
    }

private:
    T calcBinPos(T min, T max, const Scale<T>& scale, int numBins, int bin) {
        T binSize = (scale.scale(max) - scale.scale(min))/(1.0*numBins);
        return scale.invert(binSize*bin + scale.scale(min));
    }

    RandomSampler<T> sampler;
};

template <typename T>
class FairSampler : public ISamplingStrategy {
public:
    FairSampler(const std::string& name, T min, T max, const Scale<T>& scale, int numBins) {
        for (int i = 0; i < numBins; i++) {
            samplers.push_back(new BinnedRandomSampler<T>(name, min, max, scale, numBins, i));
        }
    }
    virtual ~FairSampler() {
        for (int i = 0; i < samplers.size(); i++) {
            delete samplers[i];
        }
    }
    void setParams(DataSet& params) const {
        static int curSampler = 0;
        samplers[curSampler%samplers.size()]->setParams(params);
        curSampler++;
    }

private:
    std::vector<BinnedRandomSampler<T>*> samplers;
};

}

#endif