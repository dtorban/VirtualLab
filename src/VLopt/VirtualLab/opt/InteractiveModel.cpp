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
                paramKeyIndexLookup[it->first] = paramKeys.size()-1;
            }
        }

        this->params = params;
        paramHelper = new ParameterHelper(&(this->params));
        paramHelper->set("cpool", params["cpool"].get<double>(), paramHelper->getMin("cpool"), 750, "log");

        samples.push_back(createNewSample(this->params));

        for (int i = 0; i < paramKeys.size(); i++) {
            for (int j = 0; j < parameterResolution; j++) {
                DataObject p = params;
                std::string param = paramKeys[i];
                p[param].set<double>(paramHelper->deNormalize(param, 1.0*j/(parameterResolution - 1)));
                samples.push_back(createNewSample(p));
            }
        }
        
        //this->params["samples"] = samples[0]->getParameters()["samples"];
        //this->params = samples[0]->getParameters();
        this->nav = samples[0]->getNavigation();
        this->nav["key"] = StringDataValue("");
        this->nav["val"] = DoubleDataValue(0);
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

        delete paramHelper;
    }

    void update() {}
    
    IModelSample* createNewSample(const DataObject& params) {
        return model->create(params);
    }

    const DataObject& getParameters() const { return params; }
    const DataObject& getData() const { return data; }
    DataObject& getNavigation() { return nav; }

    void interpolate(DataObject& result, const DataObject& a, const DataObject& b, double percent) {
        //result["m"] = a["m"];
        //result = a;
        double x = result["x"].get<double>();
        double y = result["y"].get<double>();
        for (DataObject::const_iterator it = a.begin(); it != a.end(); it++) {
            if (it->first == "x" || it->first == "y") {
                continue;
            }
            if (a[it->first].isType<double>()) {
                double aVal = paramHelper->scale(it->first, it->second.get<double>());
                double bVal = paramHelper->scale(it->first, b[it->first].get<double>());
                double val = paramHelper->invScale(it->first, (1.0-percent)*aVal + percent*bVal);
                result[it->first].set<double>(val);
            }
        }

        const vl::Array& modulesB = b["m"].get<vl::Array>();
        for (int i = 0; i < modulesB.size(); i++) {
            result["m"].get<vl::Array>().push_back(modulesB[i]);
            /*for (DataObject::iterator it = a.begin(); it != a.end(); it++) {
            }*/
        }
    }

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

        std::cout << nav["key"].get<std::string>() << " = " << nav["val"].get<double>() << std::endl;

        iteration++;
        
        /*if (iteration % 10 == 0) {
            currentSelected++;
            currentSelected = currentSelected % samples.size();
        }*/

        std::cout << "time: " << nav["t"].get<double>() << std::endl;

        for (int i = 0; i < samples.size(); i++) {
            double sampleTime = samples[i]->getNavigation()["t"].get<double>() + dt;
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
            std::string key = nav["key"].get<std::string>();
            if (key.length() > 0) {
                double val = paramHelper->normalize(key, nav["val"].get<double>());
                double pos = val*(parameterResolution-1);
                int indexA = std::floor(pos);
                int indexB = indexA < (parameterResolution-1) ? indexA + 1 : indexA;
                double percent = pos - indexA;
                int keyStartIndex = 1 + parameterResolution*paramKeyIndexLookup[key];
                IModelSample* sampleA = samples[keyStartIndex + indexA];
                IModelSample* sampleB = samples[keyStartIndex + indexB];
                data = samples[currentSelected]->getData();
                interpolate(data, sampleA->getData(), sampleB->getData(), percent);
            }
            
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
    std::map<std::string, int> paramKeyIndexLookup;
    DataArray history;
    int parameterResolution;
    std::condition_variable cond;
    bool running;
    int currentSelected;
    ParameterHelper* paramHelper;
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