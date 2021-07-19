#include "VirtualLab/opt/OptimizedModel.h"
#include "VirtualLab/opt/LeastSquares.h"

namespace vl {

class OptimizedSample2 : public ModelSampleDecorator, public IUpdateCallback {
public:
    
    OptimizedSample2(IModel* model, const DataObject& params, double closeness, int numSamples) : ModelSampleDecorator(new ModelSampleProxy(&proxy)), model(model), closeness(closeness), updateCompleted(0) {
        
        for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }
        
        for (int i = 0; i < numSamples; i++) {
            samples.push_back(createNewSample(params));
        }
        selectedSampleIndex = 0;
        proxy = ModelSampleProxy(samples[selectedSampleIndex]);
        
        nav = proxy.getNavigation();

    }

    virtual ~OptimizedSample2() {
        for (int i = 0; i < samples.size(); i++) {
            delete samples[i];
        }
    }
    
    IModelSample* createNewSample(const DataObject& params) {
        model->create(params);
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
        
        return model->create(p);
    }

    const DataObject& getData() const { return data; }
    DataObject& getNavigation() { return nav; }
             
    void update(IUpdateCallback* callback) {
        // Update time by dt
        double dt = nav["t"].get<double>() - proxy.getNavigation()["t"].get<double>();
        
        updateCompleted = 0;
        this->callback = callback;
        for (int i = 0; i < samples.size(); i++) {
            double sampleTime = samples[i]->getNavigation()["t"].get<double>() + dt;
            samples[i]->getNavigation() = nav;
            samples[i]->getNavigation()["t"].set<double>(sampleTime);
            samples[i]->update(new UpdateCallbackProxy(this));
        }
    }
             
    void onComplete() {
        {
            std::unique_lock<std::mutex> lock(updateMutex);
            updateCompleted++;
            if (updateCompleted < samples.size()) {
                return;    
            }
        }
        
        int min_index = 0;
        double min_aflow = 0;
        
        for (int i = 0; i < samples.size(); i++) {
            double aflow_mean = samples[i]->getData()["aflow_mean"].get<double>();
            if (aflow_mean < min_aflow) {
                min_aflow = aflow_mean;
                min_index = i;
            }
        }
        selectedSampleIndex = min_index;
        
        proxy = ModelSampleProxy(samples[selectedSampleIndex]);
        
        data = proxy.getData();
        data["index"] = DoubleDataValue(selectedSampleIndex);
                
        callback->onComplete();
        delete callback;
    }

private:
    int selectedSampleIndex;
    double closeness;
    ModelSampleProxy proxy;
    IModel* model;
    DataObject data;
    DataObject nav;
    int updateCompleted;
    std::vector<IModelSample*> samples;
    IUpdateCallback* callback;
    std::mutex updateMutex;
    std::vector<std::string> paramKeys;
};

    
OptimizedModel2::OptimizedModel2(IModel* model, double closeness, int numTestSamples) : ModelDecorator(model), closeness(closeness), numTestSamples(numTestSamples) {
        name = "Optimized " + model->getName();
}

OptimizedModel2::~OptimizedModel2() {
}

IModelSample* OptimizedModel2::create(const DataObject& params) {
    return new OptimizedSample2(model, params, closeness, numTestSamples);
}
    
    
class OptimizedSample : public ModelSampleDecorator {
public:
    OptimizedSample(IModelSample* sample, double closeness, int numTestSamples, IModel* model, OptimizedModel::DistanceFunction* distFunction) : ModelSampleDecorator(sample), numTestSamples(numTestSamples), distFunction(distFunction) {
        CompositeSampler* localSampler = new CompositeSampler();
        for (DataObject::const_iterator it = sample->getParameters().begin(); it != sample->getParameters().end(); it++) {
            if (it->first != "N") {
                localSampler->addSampler(new LocalRandomSampler(it->first, closeness));
            }
        }

        this->model = new SampledModel(new ModelProxy(model), localSampler);
        for (int i = 0; i < numTestSamples; i++) {
            testSamples.push_back(this->model->create(getParameters()));
        }

        for (DataObject::const_iterator it = getParameters().begin(); it != getParameters().end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }

        prevGradient = Eigen::VectorXd(paramKeys.size());
        sumGradient = Eigen::VectorXd(paramKeys.size());
        numSums = 0;
    }

    virtual ~OptimizedSample() {
        delete model;
        for (int i = 0; i < testSamples.size(); i++) {
            delete testSamples[i];
        }
    }

    const DataObject& getData() const { return data; }

    void update() {
        #pragma omp parallel for 
        for (int i = 0; i < testSamples.size()+1; i++) {
            if (i == 0) {
                ModelSampleDecorator::update();
            }
            else {
                testSamples[i-1]->getNavigation() = sample->getNavigation();
                testSamples[i-1]->update();
            }
        }

        data = sample->getData();
        data["opt"] = DoubleDataValue(3.14159);

        Eigen::MatrixXd A = Eigen::MatrixXd(testSamples.size(), paramKeys.size());
        Eigen::VectorXd b = Eigen::VectorXd(testSamples.size());

        ParameterHelper helper(getParameters());

        for (int i = 0; i < testSamples.size(); i++) {
            Eigen::VectorXd diff = Eigen::VectorXd(paramKeys.size());
            for (int j = 0; j < paramKeys.size(); j++) {
                std::string key = paramKeys[j];
                double testVal = helper.normalize(key, testSamples[i]->getParameters()[key].get<double>());
                double sampleVal = helper.normalize(key, sample->getParameters()[key].get<double>());
                //std::cout << key << " " << sampleVal << std::endl;

                //std::cout << testVal << " " << sampleVal << " " << testVal - sampleVal << std::endl;
                diff[j] = testVal - sampleVal;
            }

            b[i] = (distFunction->calculate(*testSamples[i]) - distFunction->calculate(*sample))  / diff.norm();
            std::cout << "100 " << distFunction->calculate(*testSamples[i]) << " " << distFunction->calculate(*sample) << " " << (distFunction->calculate(*testSamples[i]) - distFunction->calculate(*sample)) << std::endl;
            std::cout << "diff norm " << " " << diff.norm() << std::endl;
            Eigen::VectorXd dir = diff.normalized();
            A.block(i, 0, 1, paramKeys.size()) = dir.transpose();
        }

        Eigen::VectorXd sol = calculateLeastSquares(A,b);
        double res = (A*sol-b).norm();
        std::cout << "Residual " << res << std::endl;
        double diffGradient = (sol-prevGradient).norm();
        std::cout << "Gradient Diff " << diffGradient << std::endl;
        sumGradient += sol;
        numSums++;
        Eigen::VectorXd averageGradient = sumGradient/numSums;
        double diffAverage = (sol-averageGradient).norm();
        std::cout << "Average gradient Diff " << diffAverage << std::endl;
        prevGradient = sol;

        std::cout << "Gradient Mag: " << sol.norm() << std::endl;
        Eigen::VectorXd normGradient = sol.normalized();

        DataObject gradient;
        for (int i = 0; i < paramKeys.size(); i++) {
            gradient[paramKeys[i]] = DoubleDataValue(sol[i]);
        }
        data["gradient"] = gradient;

        DataArray dist;
        for (int i = 0; i < testSamples.size(); i++) {
            dist.push_back(DoubleDataValue(b[i]));
        }
        data["dist"] = dist;

        double lambda = distFunction->calculate(*sample) / sol.norm();
        std::cout << "lambda " << lambda << std::endl;

        for (int i = 0; i < paramKeys.size(); i++) {
            std::string key = paramKeys[i];
            double val = sample->getParameters()[key].get<double>();
            double normalizedVal = helper.normalize(key, val);
            std::cout << "params." << key << "=" << helper.clamp(key, helper.deNormalize(key, normalizedVal - normGradient[i]*lambda))
               << "; //" << val << " " << helper.normalize(key, val) << " " << normGradient[i] << " " << sol[i]*lambda << std::endl;
        }
    }

    void update(IUpdateCallback* callback) {
        update();
        callback->onComplete();
        delete callback;
    }

private:
    IModel* model;
    DataObject data;
    std::vector<IModelSample*> testSamples;
    int numTestSamples;
    std::vector<std::string> paramKeys;
    OptimizedModel::DistanceFunction* distFunction;
    Eigen::VectorXd prevGradient;
    Eigen::VectorXd sumGradient;
    int numSums;
};

OptimizedModel::OptimizedModel(IModel* model, DistanceFunction* distFunction, double closeness, int numTestSamples) : ModelDecorator(model), distFunction(distFunction), closeness(closeness), numTestSamples(numTestSamples) {
}

OptimizedModel::~OptimizedModel() {
    delete distFunction;
}

IModelSample* OptimizedModel::create(const DataObject& params) {
    return new OptimizedSample(ModelDecorator::create(params), closeness, numTestSamples, model, distFunction);
}

    


    
}