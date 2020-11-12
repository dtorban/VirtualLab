#ifndef VIRTUALLAB_ENSEMBLE_H_
#define VIRTUALLAB_ENSEMBLE_H_

#include "VirtualLab/IComputationalModel.h"
#include "VirtualLab/ISamplingStrategy.h"
#include <vector>

namespace vl {

class Ensemble {
public:
    Ensemble(IComputationalModel* model, ISamplingStrategy* samplingStrategy) : model(model), samplingStrategy(samplingStrategy) {}
    virtual ~Ensemble() {
        delete model;
        delete samplingStrategy;
    }

    virtual void sampleModel();

    std::vector<IModelSample*> getSamples() { return samples; }

protected:
    IComputationalModel* model;
    ISamplingStrategy* samplingStrategy;
    std::vector<IModelSample*> samples;
};

}

#endif