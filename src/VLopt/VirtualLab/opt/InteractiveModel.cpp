#include "VirtualLab/opt/InteractiveModel.h"
#include "VirtualLab/opt/LeastSquares.h"
#include <condition_variable>

namespace vl {

class InteractiveModelSample : public IModelSample, public IUpdateCallback {
public:

    InteractiveModelSample(IModel* model, const DataObject& params, int parameterResolution) : model(model), updateCompleted(0), parameterResolution(parameterResolution) {
        lastTime = 0.0;
        running = true;
        iteration = 0;
        currentSelected = 0;

        for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }
        
        ParameterHelper helper(params);

        for (int i = 0; i < paramKeys.size(); i++) {
            for (int j = 0; j < parameterResolution; j++) {
                DataObject p = params;
                std::string param = paramKeys[i];
                if (param == "cpool") {
                    p[param].set<double>(helper.deNormalize(param, 0.5*j/(parameterResolution - 1)));
                }
                else {
                    p[param].set<double>(helper.deNormalize(param, 1.0*j/(parameterResolution - 1)));
                }
                //p[param].set<double>(helper.deNormalize(param, 1.0*j/(parameterResolution - 1)));
                samples.push_back(createNewSample(p));
            }
        }
        
        this->params = params;
        //this->params["samples"] = samples[0]->getParameters()["samples"];
        //this->params = samples[0]->getParameters();
        this->nav = samples[0]->getNavigation();
    }

    virtual ~InteractiveModelSample() {

        std::unique_lock<std::mutex> lock(updateMutex);
        running = false;

        if (updateCompleted < samples.size()) {
            cond.wait(lock);
        }
        
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
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            if (!running) {
                callback->onComplete();
                delete callback;
                return;
            }
            updateCompleted = 0;
            this->callback = callback;
        }

        // Update time by dt
        double dt = nav["t"].get<double>() - lastTime;
        lastTime += dt;

        iteration++;
        
        if (iteration % 10 == 0) {
            currentSelected++;
            currentSelected = currentSelected % samples.size();
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
            
            data = samples[currentSelected]->getData();
            //data["params"] = params;

            
            DataObject h;
            h["time"] = DoubleDataValue(nav["t"].get<double>());
            h["x"] = DoubleDataValue(data["x"].get<double>()/1000);
            h["y"] = DoubleDataValue(data["y"].get<double>()/1000);
            history.push_back(h);

            data["h"] = history;

            callback->onComplete();
            delete callback;

            cond.notify_all();
    }

private:
    int iteration;
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
    DataArray history;
    int parameterResolution;
    std::condition_variable cond;
    bool running;
    int currentSelected;
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