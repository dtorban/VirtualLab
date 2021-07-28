#include "VirtualLab/opt/InteractiveModel.h"
#include "VirtualLab/opt/LeastSquares.h"

namespace vl {

class InteractiveModelSample : public IModelSample, public IUpdateCallback {
public:

    InteractiveModelSample(IModel* model, const DataObject& params, int parameterResolution) : model(model), updateCompleted(0), parameterResolution(parameterResolution) {
        lastTime = 0.0;

        for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }
        
        for (int i = 0; i < parameterResolution*paramKeys.size(); i++) {
            samples.push_back(createNewSample(params));
        }
        
        this->params = params;
        //this->params["samples"] = samples[0]->getParameters()["samples"];
        //this->params = samples[0]->getParameters();
        this->nav = samples[0]->getNavigation();
    }

    virtual ~InteractiveModelSample() {
        for (int i = 0; i < samples.size(); i++) {
            delete samples[i];
        }
    }

    void update() {}
    
    IModelSample* createNewSample(const DataObject& params) {
        return model->create(params);
    }

    const DataObject& getParameters() const { return params; }
    const DataObject& getData() const { return data; }
    DataObject& getNavigation() { return nav; }

    void update(IUpdateCallback* callback) {
        // Update time by dt
        double dt = nav["t"].get<double>() - lastTime;
        lastTime += dt;
        
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCompleted = 0;
            this->callback = callback;
        }

        for (int i = 0; i < samples.size(); i++) {
            double sampleTime = samples[i]->getNavigation()["t"].get<double>() + dt;
            std::cout << "sample time: " << sampleTime << std::endl;
            samples[i]->getNavigation() = nav;
            samples[i]->getNavigation()["t"].set<double>(sampleTime);
            samples[i]->update(new UpdateCallbackProxy(this));
        }
    }
             
    void onComplete() {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCompleted++;
            if (updateCompleted < samples.size()) {
                return;    
            }
            
            data = samples[0]->getData();
            data["params"] = params;

            callback->onComplete();
            delete callback;
    }

private:
    IModel* model;
    DataObject params;
    DataObject data;
    DataObject nav;
    double lastTime;
    int updateCompleted;
    std::vector<IModelSample*> samples;
    IUpdateCallback* callback;
    std::mutex updateMutex;
    std::vector<std::string> paramKeys;
    int parameterResolution;
};

    
InteractiveModel::InteractiveModel(IModel* model, int parameterResolution) : ModelDecorator(model), parameterResolution(parameterResolution) {
        name = "Interactive " + model->getName();
}

InteractiveModel::~InteractiveModel() {
}

IModelSample* InteractiveModel::create(const DataObject& params) {
    return new InteractiveModelSample(model, params, parameterResolution);
}


    
}