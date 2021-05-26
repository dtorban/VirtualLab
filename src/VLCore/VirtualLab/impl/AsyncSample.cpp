#include "VirtualLab/impl/AsyncSample.h"

namespace vl {

void AsyncSample::update(IUpdateCallback* callback) {
    std::unique_lock<std::mutex> lock(updateMutex);
    this->callback = callback;
    ready = true;
    cond.notify_all();
}

void AsyncSample::updateLoop() {
    while(running) {
        std::unique_lock<std::mutex> lock(updateMutex);
        if (running && !ready) {
            cond.wait(lock);
        }
        if (callback) {
            ModelSampleDecorator::update(callback);
            callback = NULL;
        }
        ready = false;
    }
}

}