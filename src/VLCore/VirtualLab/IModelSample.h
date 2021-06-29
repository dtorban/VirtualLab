#ifndef VIRTUALLAB_IMODEL_SAMPLE_H_
#define VIRTUALLAB_IMODEL_SAMPLE_H_

#include "VirtualLab/IDataSet.h"
#include "VirtualLab/DataValue.h"
#include "util/ByteBuffer.h"
#include <mutex>

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

	virtual IModelSample* getInnerSample() { return NULL; }
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
	virtual IModelSample* getInnerSample() { return sample; }

protected:
    IModelSample* sample;
};

template <class T> class CountedPtr {
public:
	CountedPtr(T* p = 0)
	{
		if (p != 0)
		{
			refCounter = new counter(p);
		}
		else
		{
			refCounter = 0;
		}
	}

	CountedPtr(const CountedPtr& cp)
	{
		setPointer(cp.refCounter);
	}

	virtual ~CountedPtr()
	{
		if (refCounter != 0)
		{
			releasePointer();
		}
	}

	CountedPtr<T>& operator=(const CountedPtr& cp) {
		if (refCounter != 0)
		{
			releasePointer();
		}
		setPointer(cp.refCounter);
	    return *this;
	}

	T& operator*() const throw() {return *refCounter->pointer;}
	T* operator->() const throw() {return refCounter->pointer;}
	bool operator==(const CountedPtr<T>& rhs) {return refCounter == rhs.refCounter;}
	bool operator!=(const CountedPtr<T>& rhs) {return !(*this == rhs);}

private:
	struct counter
	{
		T* pointer;
		int count;
		counter(T* p) : pointer(p), count(1) {}
		~counter() { delete pointer; }
	}* refCounter;

	void setPointer(counter* c)
	{
		refCounter = c;
		refCounter->count++;
	}

	void releasePointer()
	{
		refCounter->count--;
		if (refCounter->count == 0)
		{
            std::cout << "deleted ref" << std::endl;
			delete refCounter;
		}
	}
};

class AsyncModelSampleDecorator : public ModelSampleDecorator {
private:
    class UpdateCallback : public IUpdateCallback {
	public:
		UpdateCallback(AsyncModelSampleDecorator* sample, IUpdateCallback* callback, CountedPtr<bool>& sampleAvailable) : sample(sample), callback(callback), sampleAvailable(sampleAvailable) {}
        virtual ~UpdateCallback() {
            delete callback;
        }

		void onComplete() {
            //std::cout << "async " << sample << std::endl;
            if (*sampleAvailable) {
			    sample->asyncUpdate();
            }
            callback->onComplete();
		}

	private:
		AsyncModelSampleDecorator* sample;
        IUpdateCallback* callback;
        CountedPtr<bool>& sampleAvailable;
	};
public:
    AsyncModelSampleDecorator(IModelSample* sample) : ModelSampleDecorator(sample) , sampleAvailable(new bool()) {
        (*sampleAvailable) = true;
    }
    virtual ~AsyncModelSampleDecorator() {
        std::cout << "delete " << this << std::endl;
        (*sampleAvailable) = false;
    }

    void update() {
        ModelSampleDecorator::update();
        asyncUpdate();
    }

    void update(IUpdateCallback* callback) {
        return ModelSampleDecorator::update(new UpdateCallback(this, callback, sampleAvailable));
    }

protected:
    virtual void asyncUpdate() {}

private:
    CountedPtr<bool> sampleAvailable;
};



}

#endif
