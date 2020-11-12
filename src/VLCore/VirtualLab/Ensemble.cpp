#include "VirtualLab/Ensemble.h"

namespace vl {

void Ensemble::sampleModel() {
    DataSet* params = model->createParameterSet();
    samplingStrategy->setParams(*params);
    IModelSample* sample = model->sample(params);
    if (samplingStrategy->isValidSample(*sample)) {
        samples.push_back(sample);
    }
    else {
        delete sample;
    }
}

}