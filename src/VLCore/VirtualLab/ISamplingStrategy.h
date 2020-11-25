#ifndef VIRTUALLAB_ISAMPLING_STRATEGY_H_
#define VIRTUALLAB_ISAMPLING_STRATEGY_H_

#include "VirtualLab/IDataSet.h"
#include <cstdlib>

namespace vl {

class ISamplingStrategy {
public:
    virtual ~ISamplingStrategy() {}

    virtual void setParameters(IDataSet& params, IDataSet& metaData) = 0;
};

class CompositeSamplingStrategy : public ISamplingStrategy {
public:
    CompositeSamplingStrategy() {}
    ~CompositeSamplingStrategy() {
        for (ISamplingStrategy* strategy : strategies) {
            delete strategy;
        }
    }
    void addStrategy(ISamplingStrategy* strategy) { strategies.push_back(strategy); }

    void setParameters(IDataSet& params, IDataSet& metaData) {
        for (ISamplingStrategy* strategy : strategies) {
            strategy->setParameters(params, metaData);
        }
    }

private:
    std::vector<ISamplingStrategy*> strategies;
};

template <typename T>
class SetMinMaxMetaData : public ISamplingStrategy {
public:
    SetMinMaxMetaData(const std::string& key, T min, T max) : key(key), min(min), max(max) {}

    void setParameters(IDataSet& params, IDataSet& metaData) {
        std::string minKey = getMinKey(key);
        std::string maxKey = getMaxKey(key);
        if (!metaData.containsKey(minKey)) {
            metaData.addData(minKey, new TypedData<T>());
        }
        if (!metaData.containsKey(maxKey)) {
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

template <typename T>
class RandomSampler : public ISamplingStrategy {
public:
    RandomSampler(const std::string& key) : key(key) {}

    void setParameters(IDataSet& params, IDataSet& metaData) {
        std::string minKey = SetMinMaxMetaData<T>::getMinKey(key);
        std::string maxKey = SetMinMaxMetaData<T>::getMaxKey(key);
        T min = metaData[minKey].get<T>();
        T max = metaData[maxKey].get<T>();
        double r = (double)std::rand() / (double)RAND_MAX;
        params[key].set<T>(r*(max - min) + min);
    }

private:
    T min;
    T max;
    std::string key;
};

}

#endif