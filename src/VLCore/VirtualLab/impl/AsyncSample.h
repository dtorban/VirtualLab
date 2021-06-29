#ifndef VIRTUALLAB_IMPL_ASYNC_SAMPLE_H_
#define VIRTUALLAB_IMPL_ASYNC_SAMPLE_H_

#include "VirtualLab/IModelSample.h"

#include <thread>
#include <mutex>
#include <condition_variable>

namespace vl {

class AsyncSample : public ModelSampleDecorator {
public:
    AsyncSample(IModelSample* sample) : ModelSampleDecorator(sample), running(true), ready(false), callback(NULL) {
        updateThread = new std::thread(&AsyncSample::updateLoop, this);
    }

    virtual ~AsyncSample() {
        std::unique_lock<std::mutex> lock(updateMutex);
        running = false;
        ready = true;
        cond.notify_all();
        lock.unlock();
        updateThread->join();
        delete updateThread;
    }

    void update(IUpdateCallback* callback);
    void updateLoop();

private:
    std::thread* updateThread;
    std::mutex updateMutex;
    std::condition_variable cond;
    IUpdateCallback* callback;
    bool running;
    bool ready;
};

}

#endif