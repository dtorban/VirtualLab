#include "VirtualLab/opt/InteractiveModel.h"
#include "VirtualLab/opt/LeastSquares.h"
#include <condition_variable>

namespace vl {

class InteractiveModelSample : public IModelSample, public IUpdateCallback {
public:

    InteractiveModelSample(IModel* model, const DataObject& params, int parameterResolution) : model(model), updateCompleted(0), parameterResolution(parameterResolution) {
        lastTime = 0.0;
        lastCommitTime = 0.0;
        running = true;
        iteration = 0;
        currentSelected = 0;
        closeness = 0.1;
        numRandomSim = 0;
        xPos = 0;
        yPos = 0;

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
        
        for (int i = 0; i < numRandomSim; i++) {
            samples.push_back(createNewSample(createNewParams(this->params)));
        }

        //this->params["samples"] = samples[0]->getParameters()["samples"];
        //this->params = samples[0]->getParameters();
        this->nav = samples[0]->getNavigation();
        this->nav["key"] = StringDataValue("");
        this->nav["val"] = DoubleDataValue(0);
        this->nav["com"] = DoubleDataValue(0);
        this->nav["interp"] = DoubleDataValue(0);
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
    
    DataObject createNewParams(const DataObject& params) {
        Eigen::VectorXd dir = Eigen::VectorXd(paramKeys.size());
        for (int i = 0; i < paramKeys.size(); i++) {
            double r = (double)std::rand() / (double)RAND_MAX;
            r = 2.0*(r-0.5);
            dir[i] = r;
        }
        dir = dir.normalized()*closeness;
        
        ParameterHelper helper(params);
        DataObject p = params;
        
        for (int i = 0; i < paramKeys.size(); i++) {
            std::string param = paramKeys[i];
            double max = helper.scale(param, helper.getMax(param));
            double min = helper.scale(param, helper.getMin(param));
            double val = helper.scale(param, params[param].get<double>());
            val = val + dir[i]*(max-min);
            if (val < min) {
                val = min;
            }
            if (val > max) {
                val = max;
            }
            val = helper.invScale(param, val);
            p[param].set<double>(val);
            std::cout << param << " " << p[param].get<double>() << " " << val << std::endl;
        }

        return p;
    }

    IModelSample* createNewSample(const DataObject& params) {
        return model->create(params);
    }

    const DataObject& getParameters() const { return params; }
    const DataObject& getData() const { return data; }
    DataObject& getNavigation() { return nav; }

    void interpolate(DataObject& result, const DataObject& a, const DataObject& b, double percent) {
        //result["m"] = a["m"];
        //result = a;
        double x = xPos;//result["x"].get<double>();
        double y = yPos;//result["y"].get<double>();
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

        const vl::Array& modulesA = a["m"].get<vl::Array>();
        const vl::Array& modulesB = b["m"].get<vl::Array>();
        DataArray modules;
        //std::cout << modulesA.size() << " " << modulesB.size() << " " << (1.0-percent)*modulesA.size() + percent*modulesB.size() << std::endl;
        int numModules = (1.0-percent)*modulesA.size() + percent*modulesB.size();
        for (int i = 0; i < numModules; i++) {
            //int moduleAIndex = i*(modulesA.size() < numModules ? modulesA.size()/numModules : numModules/modulesA.size());
            //int moduleBIndex = i*(modulesB.size() < numModules ? modulesB.size()/numModules : numModules/modulesB.size());
            int moduleAIndex = i*modulesA.size()/numModules;
            int moduleBIndex = i*modulesB.size()/numModules;

            DataObject module = modulesA[moduleAIndex].get<vl::Object>();
            for (DataObject::const_iterator it = module.begin(); it != module.end(); it++) {
                /*if (it->first == "x" || it->first == "y") {
                    continue;
                }*/
                if (module[it->first].isType<double>()) {
                    double aVal = modulesA[moduleAIndex].get<vl::Object>().find(it->first)->second.get<double>();
                    double bVal = modulesB[moduleBIndex].get<vl::Object>().find(it->first)->second.get<double>();
                    if (it->first == "x") {
                        aVal = aVal-a["x"].get<double>();
                        bVal = bVal-b["x"].get<double>();
                    }
                    else if (it->first == "y") {
                        aVal = aVal-a["y"].get<double>();
                        bVal = bVal-b["y"].get<double>();
                    }
                    double val = (1.0-percent)*aVal + percent*bVal;
                    if (it->first == "x") {
                        val += x;
                    }
                    else if (it->first == "y") {
                        val += y;
                    }
                    module[it->first].set<double>(val);
                    
                }
            }

            modules.push_back(module);            
            //result["m"].get<vl::Array>().push_back(modulesB[i]);
            /*for (DataObject::iterator it = a.begin(); it != a.end(); it++) {
            }*/
        }
        data["m"] = modules;
        data["x"].set<double>(x);
        data["y"].set<double>(y);
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

        //std::cout << nav["key"].get<std::string>() << " = " << nav["val"].get<double>() << std::endl;
        std::string key = nav["key"].get<std::string>();
        if (key.length() > 0) {
            double val = nav["val"].get<double>();
            params[key].set<double>(val);
        }

        if (nav["com"].get<double>() > 0.001) {
            lastCommitTime = lastTime;
            // Update primary
            delete samples[0];
            samples[0] = createNewSample(params);

            // Update samples
            for (int i = 0; i < paramKeys.size(); i++) {
                std::string param = paramKeys[i];
                if (key != param) {
                    for (int j = 0; j < parameterResolution; j++) {
                        DataObject p = params;
                        p[param].set<double>(paramHelper->deNormalize(param, 1.0*j/(parameterResolution - 1)));
                        int sampleIndex = 1+i*parameterResolution + j;
                        delete samples[sampleIndex];
                        samples[sampleIndex] = createNewSample(p);
                    }
                }
            }
            for (int i = 0; i < numRandomSim; i++) {
                int sampleIndex = 1+paramKeys.size()*parameterResolution + i;
                delete samples[sampleIndex];
                samples[sampleIndex] = createNewSample(createNewParams(this->params));
            }
            nav["com"].set(0.0);
        }

        iteration++;
        
        /*if (iteration % 10 == 0) {
            currentSelected++;
            currentSelected = currentSelected % samples.size();
        }*/

        std::cout << "time: " << nav["t"].get<double>() << std::endl;

        for (int i = 0; i < samples.size(); i++) {
            double sampleTime = samples[i]->getNavigation()["t"].get<double>() + dt;
            if (i == 0 || sampleTime < 60 
                || (nav["interp"].get<double>() > 0.001 && 
                (paramKeyIndexLookup[key]*parameterResolution + 1 <= i && (paramKeyIndexLookup[key]+1)*parameterResolution + 1 > i))) {
                samples[i]->getNavigation() = nav;
                samples[i]->getNavigation()["t"].set<double>(sampleTime);
                samples[i]->update(new UpdateCallbackProxy(this));
                //std::cout << sampleTime << std::endl;
            }
            else {
                onComplete();
            }
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
            double interpPercent = nav["interp"].get<double>();
            if (key.length() > 0 && interpPercent > 0.001) {
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

            DataObject result = data;
            //std::cout << interpPercent << std::endl;
            interpolate(data, result, samples[currentSelected]->getData(), 1.0 - interpPercent);

            /*std::cout << samples[currentSelected]->getData()["aflow_mean"].get<double>() << " ";
            for (int i = 1+paramKeys.size()*parameterResolution; i < samples.size(); i++) {
                std::cout << samples[i]->getData()["aflow_mean"].get<double>() << " ";
            }
            std::cout << std::endl;*/

            xPos += data["dx"].get<double>();
            yPos += data["dy"].get<double>();
            data["x"].set<double>(xPos);
            data["y"].set<double>(yPos);

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
    double lastCommitTime;
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
    double xPos;
    double yPos;
    double closeness;
    int numRandomSim;
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
