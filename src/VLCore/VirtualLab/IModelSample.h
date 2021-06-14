#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/DataValue.h"
#include "util/ByteBuffer.h"

namespace vl {

class IUpdateCallback {
public:
    virtual ~IUpdateCallback() {}
    virtual void onComplete() = 0;
};

class IModelSample {
public:
    virtual ~IModelSample() {}

    virtual const DataObject& getParameters() const = 0;
    virtual DataObject& getNavigation() = 0;
    virtual const DataObject& getData() const = 0;
    virtual void update() = 0;
    virtual void update(IUpdateCallback* callback) {
        update();
        callback->onComplete();
        delete callback;
    }
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
    virtual void update(IUpdateCallback* callback) { return sample->update(callback); }

protected:
    IModelSample* sample;
};


class AsyncModelSampleDecorator : public ModelSampleDecorator {
private:
    class UpdateCallback : public IUpdateCallback {
	public:
		UpdateCallback(AsyncModelSampleDecorator* sample, IUpdateCallback* callback) : sample(sample), callback(callback) {}
        virtual ~UpdateCallback() {
            delete callback;
        }

		void onComplete() {
			sample->asyncUpdate();
            callback->onComplete();
		}

	private:
		AsyncModelSampleDecorator* sample;
        IUpdateCallback* callback;
	};
public:
    AsyncModelSampleDecorator(IModelSample* sample) : ModelSampleDecorator(sample) {}
    virtual ~AsyncModelSampleDecorator() {}

    void update() {
        ModelSampleDecorator::update();
        asyncUpdate();
    }

    void update(IUpdateCallback* callback) {
        return ModelSampleDecorator::update(new UpdateCallback(this, callback));
    }

protected:
    virtual void asyncUpdate() {}
};



}

#endif
