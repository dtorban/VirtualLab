#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/DataValue.h"
#include "util/ByteBuffer.h"

namespace vl {

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual const DataObject& getParameters() const = 0;
    virtual DataObject& getNavigation() = 0;
    virtual const DataObject& getData() const = 0;
    virtual void update() = 0;
    //virtual bool saveState(ByteBufferWriter& writer) { return false; }
    //virtual bool loadState(ByteBufferReader& reader) { return false; }
};

class ModelSampleDecorator : public IModelSample {
public:
    ModelSampleDecorator(IModelSample* sample) : sample(sample) {}
    virtual ~ModelSampleDecorator() { delete sample; }

    virtual const DataObject& getParameters() const { return sample->getParameters(); }
    virtual DataObject& getNavigation() { return sample->getNavigation(); }
    virtual const DataObject& getData() const { return sample->getData(); }
    virtual void update() { return sample->update(); }

protected:
    IModelSample* sample;
};

}

#endif