#include "VirtualLab/opt/OptimizedModel.h"
#include "VirtualLab/opt/LeastSquares.h"

namespace vl {

class ForceModelSample : public IModelSample, public IUpdateCallback {
public:

    ForceModelSample(IModel* model, const DataObject& params, double closeness, int numSamples) : model(model), closeness(closeness), updateCompleted(0) {
        iteration = 0;
        lastTime = 0.0;

        for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }
        
        for (int i = 0; i < numSamples; i++) {
            samples.push_back(createNewSample(params));
        }
        
        this->params = createNewParams(params);
        this->params["samples"] = samples[0]->getParameters()["samples"];
        //this->params = samples[0]->getParameters();
        this->nav = samples[0]->getNavigation();

        //x = Eigen::VectorXd::Zero(paramKeys.size());
        v = Eigen::VectorXd::Zero(paramKeys.size());

        forces.push_back(new Force(0, 90, "aflow_mean"));
        forces.push_back(new Force(4, 60, "aflow_mean"));
        forces.push_back(new Force(7, 100, "aflow_mean"));
    }

    virtual ~ForceModelSample() {
        for (int i = 0; i < samples.size(); i++) {
            delete samples[i];
        }

        for (int i = 0; i < forces.size(); i++) {
            delete forces[i];
        }
        /*
                        double dist = 0.0;
                if (forceId == 0) {
                    goalIndex = 0;
                    goal = 90;
                }
                else if (forceId == 1) {
                    goalIndex = 4;
                    goal = 60;
                }
                else if (forceId == 2) {
                    goalIndex = 7;
                    goal = 100;
                }*/
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
        DataObject p = createNewParams(params);
        return model->create(p);
    }

    const DataObject& getParameters() const { return params; }
    const DataObject& getData() const { return data; }
    DataObject& getNavigation() { return nav; }

    class Force {
    public:
        Force(int goalIndex, double goal, const std::string& dataKey) : goalIndex(goalIndex), goal(goal), dataKey(dataKey) {}

        const Eigen::VectorXd& calculateForce(const std::vector<IModelSample*>& samples, const DataObject& params, const DataObject& data, const std::vector<std::string>& paramKeys) {
            double currentDist = std::fabs(data["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.get<double>() - goal);
            
            std::vector<double> distance;
            for (int i = 0; i < samples.size(); i++) {
                double dist = std::fabs(samples[i]->getData()["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.get<double>() - goal);
                distance.push_back(dist);
            }

            // Calculate gradient
            Eigen::MatrixXd A = Eigen::MatrixXd(samples.size(), paramKeys.size());
            Eigen::VectorXd b = Eigen::VectorXd(samples.size());

            Eigen::VectorXd x = Eigen::VectorXd(paramKeys.size());

            ParameterHelper helper(params);

            for (int j = 0; j < paramKeys.size(); j++) {
                std::string key = paramKeys[j];
                x[j] = helper.normalize(key, params[key].get<double>());
            }

            int currentSample = 0;
            for (int i = 0; i < samples.size(); i++) {

                Eigen::VectorXd diff = Eigen::VectorXd(paramKeys.size());
                for (int j = 0; j < paramKeys.size(); j++) {
                    std::string key = paramKeys[j];
                    double testVal = helper.normalize(key, samples[i]->getParameters()[key].get<double>());
                    double sampleVal = helper.normalize(key, params[key].get<double>());
                    //std::cout << key << " " << sampleVal << std::endl;

                    //std::cout << testVal << " " << sampleVal << " " << testVal - sampleVal << std::endl;
                    diff[j] = testVal - sampleVal;
                }

                b[currentSample] = (distance[i] - currentDist)  / diff.norm();
                //std::cout << "100 " << distFunction->calculate(*testSamples[i]) << " " << distFunction->calculate(*sample) << " " << (distFunction->calculate(*testSamples[i]) - distFunction->calculate(*sample)) << std::endl;
                //std::cout << "diff norm " << " " << diff.norm() << std::endl;
                Eigen::VectorXd dir = diff.normalized();
                A.block(currentSample, 0, 1, paramKeys.size()) = dir.transpose();
                currentSample++;
            }

            Eigen::VectorXd sol = calculateLeastSquares(A,b);
            double res = (A*sol-b).norm();
            std::cout << "Residual " << res << std::endl;


            //double lambda = distance[selectedSampleIndex] / sol.norm();
            //std::cout << "lambda " << lambda << std::endl;

            gradient = sol;
            Eigen::VectorXd normGradient = sol.normalized();

            Eigen::VectorXd dx = x - (x-normGradient*goal);
            //Eigen::VectorXd f = -dx - 0.0*v;
            force = -dx;

            return force;
        }

        void setValue(const std::vector<IModelSample*>& samples, DataObject& data, const Eigen::VectorXd& dir) {
            double val = 0.0;
            for (int i = 0; i < samples.size(); i++) {
                val += samples[i]->getData()["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.get<double>();
            }
            val = val / samples.size();
            data["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.set<double>(val);
            /*if (gradient.size() > 0) {
                Eigen::VectorXd normDir = dir.normalized();
                double dirDeriv = gradient.dot(normDir);
                std::cout << "Directional Deriv: " << dirDeriv << std::endl;

                double val = data["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.get<double>();
                std::cout << "Directional Deriv: " << dirDeriv << " " << val;
                val += dirDeriv*dir.norm();
                std::cout << " " << val << " " << dir.norm() << " " << dir << std::endl;
                data["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find(dataKey)->second.set<double>(val);
            }*/
        }

    private:
        double goal;
        int goalIndex;
        std::string dataKey;
        Eigen::VectorXd gradient;
        Eigen::VectorXd force;
    };
             
    /*Eigen::VectorXd calculateForce(int forceId) {
        
            double goal;
            double goalIndex;
            std::vector<double> distance;
            for (int i = 0; i < samples.size(); i++) {
                double dist = 0.0;
                if (forceId == 0) {
                    goalIndex = 0;
                    goal = 90;
                }
                else if (forceId == 1) {
                    goalIndex = 4;
                    goal = 60;
                }
                else if (forceId == 2) {
                    goalIndex = 7;
                    goal = 100;
                }

                dist = std::pow((samples[i]->getData()["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find("aflow_mean")->second.get<double>() - goal),2);
               
                //
                //dist = std::sqrt(dist);
                distance.push_back(dist);
            }

            // Calculate gradient
            Eigen::MatrixXd A = Eigen::MatrixXd(samples.size()-1, paramKeys.size());
            Eigen::VectorXd b = Eigen::VectorXd(samples.size()-1);

            Eigen::VectorXd x = Eigen::VectorXd(paramKeys.size());

            ParameterHelper helper(getParameters());

            int currentSample = 0;
            for (int i = 0; i < samples.size(); i++) {
                if (i == selectedSampleIndex) {
                    for (int j = 0; j < paramKeys.size(); j++) {
                        std::string key = paramKeys[j];
                        x[j] = helper.normalize(key, sample->getParameters()[key].get<double>());
                    }
                    continue;
                }

                Eigen::VectorXd diff = Eigen::VectorXd(paramKeys.size());
                for (int j = 0; j < paramKeys.size(); j++) {
                    std::string key = paramKeys[j];
                    double testVal = helper.normalize(key, samples[i]->getParameters()[key].get<double>());
                    double sampleVal = helper.normalize(key, sample->getParameters()[key].get<double>());
                    //std::cout << key << " " << sampleVal << std::endl;

                    //std::cout << testVal << " " << sampleVal << " " << testVal - sampleVal << std::endl;
                    diff[j] = testVal - sampleVal;
                }

                b[currentSample] = (distance[i] - distance[selectedSampleIndex])  / diff.norm();
                //std::cout << "100 " << distFunction->calculate(*testSamples[i]) << " " << distFunction->calculate(*sample) << " " << (distFunction->calculate(*testSamples[i]) - distFunction->calculate(*sample)) << std::endl;
                //std::cout << "diff norm " << " " << diff.norm() << std::endl;
                Eigen::VectorXd dir = diff.normalized();
                A.block(currentSample, 0, 1, paramKeys.size()) = dir.transpose();
                currentSample++;
            }

            Eigen::VectorXd sol = calculateLeastSquares(A,b);
            double res = (A*sol-b).norm();
            std::cout << "Residual " << res << std::endl;


            //double lambda = distance[selectedSampleIndex] / sol.norm();
            //std::cout << "lambda " << lambda << std::endl;

            Eigen::VectorXd gradient = sol;
            Eigen::VectorXd normGradient = sol.normalized();

            Eigen::VectorXd dx = x - (x-gradient.normalized()*goal);
            Eigen::VectorXd f = -dx - 0.0*v;

//Vector3d dx = x.segment(nodeSize*node + positionOffset, 3) - Vector3d(anchorPoint[0], anchorPoint[1], anchorPoint[2]);
	//f.segment(nodeSize*node + positionOffset, 3) += -ks*dx - kd*v.segment(nodeSize*node + positionOffset, 3);

            return f;//*lambda;
    }*/

    void update(IUpdateCallback* callback) {
        // Update time by dt
        double dt = nav["t"].get<double>() - lastTime;
        lastTime += dt;

        if (iteration > 0 && iteration % 10 == 0) {

            Eigen::VectorXd force(paramKeys.size());

            for (int i = 0; i < forces.size(); i++) {
                force += forces[i]->calculateForce(samples, params, data, paramKeys);
            }

            for (int i = 0; i < paramKeys.size(); i++) {
                v[i] += force[i]*0.01;
            }

            ParameterHelper helper(params);
            for (int i = 0; i < paramKeys.size(); i++) {
                std::string key = paramKeys[i];
                double val = params[key].get<double>();
                double normalizedVal = helper.normalize(key, val);
                params[key].set<double>(helper.clamp(key, helper.deNormalize(key, normalizedVal + v[i]*0.01)));
                
            }

            DataObject p = params;
            p["samples"] = DataArray();

            /*Eigen::VectorXd force = calculateForce(0);
            force += calculateForce(1);
            force += calculateForce(2);
            //force = force.normalized();
            
            //v = Eigen::VectorXd::Zero(paramKeys.size());
            for (int i = 0; i < paramKeys.size(); i++) {
                v[i] += force[i]*0.01;
            }    */
            

            /*DataObject params = samples[selectedSampleIndex]->getParameters();
            ParameterHelper helper(params);
            params["samples"] = DataArray();

            for (int i = 0; i < paramKeys.size(); i++) {
                std::string key = paramKeys[i];
                double val = 0.0//sample->getParameters()[key].get<double>();
                double normalizedVal = helper.normalize(key, val);
                params[key].set<double>(helper.clamp(key, helper.deNormalize(key, normalizedVal + v[i]*0.01)));
                
            }*/


            for (int i = 0; i < samples.size(); i++) {
                /*if (i != selectedSampleIndex) {
                    delete samples[i];
                    samples[i] = createNewSample(params);
                }*/
                //else {
                    delete samples[i];
                    samples[i] = model->create(p);
                //}
            }

        }

        iteration++;
        
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

            for (int i = 0; i < forces.size(); i++) {
                forces[i]->setValue(samples, data, v*0.01);
            }

            callback->onComplete();
            delete callback;
    }

private:
    double closeness;
    IModel* model;
    DataObject params;
    DataObject data;
    DataObject nav;
    double lastTime;
    int updateCompleted;
    std::vector<IModelSample*> samples;
    IUpdateCallback* callback;
    std::mutex updateMutex;
    int iteration;
    //Eigen::VectorXd x;
    Eigen::VectorXd v;
    std::vector<std::string> paramKeys;
    std::vector<Force*> forces;
};

    
ForceModel::ForceModel(IModel* model, double closeness, int numTestSamples) : ModelDecorator(model), closeness(closeness), numTestSamples(numTestSamples) {
        name = "ForceModel " + model->getName();
}

ForceModel::~ForceModel() {
}

IModelSample* ForceModel::create(const DataObject& params) {
    return new ForceModelSample(model, params, closeness, numTestSamples);
}


class OptimizedSample2 : public ModelSampleDecorator, public IUpdateCallback {
public:
    
    OptimizedSample2(IModel* model, const DataObject& params, double closeness, int numSamples) : ModelSampleDecorator(new ModelSampleProxy(&proxy)), model(model), closeness(closeness), updateCompleted(0) {
        iteration = 0;
        lastTime = 0.0;

        for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
            if (it->second.isType<double>() && it->first != "N" && it->first != "num") {
                paramKeys.push_back(it->first);
            }
        }
        
        selectedSampleIndex = 0;
        for (int i = 0; i < numSamples; i++) {
            if (i != selectedSampleIndex) {
                samples.push_back(createNewSample(params));
            }
            else {
                samples.push_back(model->create(params));
            }
            
            sortedIndexes.push_back(i);
        }

        proxy = ModelSampleProxy(samples[selectedSampleIndex]);
        
        nav = proxy.getNavigation();

        v = Eigen::VectorXd::Zero(paramKeys.size());

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
             
    Eigen::VectorXd calculateForce(int forceId) {
        
            double goal;
            double goalIndex;
            std::vector<double> distance;
            for (int i = 0; i < samples.size(); i++) {
                double dist = 0.0;
                if (forceId == 0) {
                    goalIndex = 0;
                    goal = 90;
                }
                else if (forceId == 1) {
                    goalIndex = 4;
                    goal = 60;
                }
                else if (forceId == 2) {
                    goalIndex = 7;
                    goal = 100;
                }

                //dist = std::pow((samples[i]->getData()["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find("aflow_mean")->second.get<double>() - goal),2);
                dist = std::fabs(samples[i]->getData()["samples"].get<vl::Array>()[goalIndex].get<vl::Object>().find("aflow_mean")->second.get<double>() - goal);
               
                //
                //dist = std::sqrt(dist);
                distance.push_back(dist);
            }

            // Calculate gradient
            Eigen::MatrixXd A = Eigen::MatrixXd(samples.size()-1, paramKeys.size());
            Eigen::VectorXd b = Eigen::VectorXd(samples.size()-1);

            Eigen::VectorXd x = Eigen::VectorXd(paramKeys.size());

            ParameterHelper helper(getParameters());

            int currentSample = 0;
            for (int i = 0; i < samples.size(); i++) {
                if (i == selectedSampleIndex) {
                    for (int j = 0; j < paramKeys.size(); j++) {
                        std::string key = paramKeys[j];
                        x[j] = helper.normalize(key, sample->getParameters()[key].get<double>());
                    }
                    continue;
                }

                Eigen::VectorXd diff = Eigen::VectorXd(paramKeys.size());
                for (int j = 0; j < paramKeys.size(); j++) {
                    std::string key = paramKeys[j];
                    double testVal = helper.normalize(key, samples[i]->getParameters()[key].get<double>());
                    double sampleVal = helper.normalize(key, sample->getParameters()[key].get<double>());
                    //std::cout << key << " " << sampleVal << std::endl;

                    //std::cout << testVal << " " << sampleVal << " " << testVal - sampleVal << std::endl;
                    diff[j] = testVal - sampleVal;
                }

                b[currentSample] = (distance[i] - distance[selectedSampleIndex])  / diff.norm();
                //std::cout << "100 " << distFunction->calculate(*testSamples[i]) << " " << distFunction->calculate(*sample) << " " << (distFunction->calculate(*testSamples[i]) - distFunction->calculate(*sample)) << std::endl;
                //std::cout << "diff norm " << " " << diff.norm() << std::endl;
                Eigen::VectorXd dir = diff.normalized();
                A.block(currentSample, 0, 1, paramKeys.size()) = dir.transpose();
                currentSample++;
            }

            Eigen::VectorXd sol = calculateLeastSquares(A,b);
            double res = (A*sol-b).norm();
            std::cout << "Residual " << res << std::endl;


            //double lambda = distance[selectedSampleIndex] / sol.norm();
            //std::cout << "lambda " << lambda << std::endl;

            Eigen::VectorXd gradient = sol;
            Eigen::VectorXd normGradient = sol.normalized();

            Eigen::VectorXd dx = x - (x-gradient.normalized()*goal);
            Eigen::VectorXd f = -dx - 0.0*v;

//Vector3d dx = x.segment(nodeSize*node + positionOffset, 3) - Vector3d(anchorPoint[0], anchorPoint[1], anchorPoint[2]);
	//f.segment(nodeSize*node + positionOffset, 3) += -ks*dx - kd*v.segment(nodeSize*node + positionOffset, 3);

            return f;//*lambda;
    }

    void update(IUpdateCallback* callback) {
        // Update time by dt
        double dt = nav["t"].get<double>() - lastTime;
        lastTime += dt;

        if (iteration > 0 && iteration % 10 == 0) {

            Eigen::VectorXd force = calculateForce(0);
            force += calculateForce(1);
            force += calculateForce(2);
            //force = force.normalized();
            
            //v = Eigen::VectorXd::Zero(paramKeys.size());
            for (int i = 0; i < paramKeys.size(); i++) {
                v[i] += force[i]*0.01;
            }    

            DataObject params = samples[selectedSampleIndex]->getParameters();
            //DataObject params = samples[sortedIndexes[0]]->getParameters();
            ParameterHelper helper(params);
            params["samples"] = DataArray();

            /*for (int i = 0; i < paramKeys.size(); i++) {
                std::string key = paramKeys[i];
                double val = sample->getParameters()[key].get<double>();
                double normalizedVal = helper.normalize(key, val);
                //std::cout << "params." << key << "=" << helper.clamp(key, helper.deNormalize(key, normalizedVal - normGradient[i]*closeness)) << ";" << std::endl;
                //params[key].set<double>(helper.clamp(key, helper.deNormalize(key, normalizedVal - normGradient[i]*closeness)));
                //params[key].set<double>(helper.clamp(key, helper.deNormalize(key, normalizedVal - normGradient[i]*lambda)));
                params[key].set<double>(helper.clamp(key, helper.deNormalize(key, normalizedVal + v[i]*0.01)));
                
            }*/

            for (int i = 0; i < samples.size(); i++) {
                if (i != selectedSampleIndex) {
                    delete samples[i];
                    samples[i] = createNewSample(params);
                }
                else {
                    delete samples[i];
                    samples[i] = model->create(params);
                }
            }

            /*for (int i = sortedIndexes.size()/2; i < sortedIndexes.size(); i++) {
                delete samples[sortedIndexes[i]];
                samples[sortedIndexes[i]] = createNewSample(params);
            }*/

        }

        iteration++;
        
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
        
            int min_index = 0;
            double min_dist = 0;

            std::vector<double> distance;
            for (int i = 0; i < samples.size(); i++) {
                double dist = 0.0;
                dist += std::pow((samples[i]->getData()["samples"].get<vl::Array>()[0].get<vl::Object>().find("aflow_mean")->second.get<double>() - 90),2);
                dist += std::pow((samples[i]->getData()["samples"].get<vl::Array>()[4].get<vl::Object>().find("aflow_mean")->second.get<double>() - 60),2);
                dist += std::pow((samples[i]->getData()["samples"].get<vl::Array>()[7].get<vl::Object>().find("aflow_mean")->second.get<double>() - 100),2);
                dist = std::sqrt(dist);
                //std::cout << "dist: " << dist << std::endl;
                distance.push_back(dist);

                if (i == 0) {
                    min_dist = dist;
                }
                else {
                    if (dist < min_dist) {
                        min_dist = dist;
                        min_index = i;
                    }
                }
            }

            auto cmp = [distance](char a, char b) { return distance[a] < distance[b]; };
            std::sort(sortedIndexes.begin(), sortedIndexes.end(), cmp);
            for (int i = 0; i < sortedIndexes.size(); i++) {
                std::cout << sortedIndexes[i] << " dist: " << distance[sortedIndexes[i]] << std::endl;
            }

            //selectedSampleIndex = min_index;
            
            proxy = ModelSampleProxy(samples[selectedSampleIndex]);
            
            data = proxy.getData();
            data["index"] = DoubleDataValue(selectedSampleIndex);
                    
            
            //double diffGradient = (sol-prevGradient).norm();
            //std::cout << "Gradient Diff " << diffGradient << std::endl;
            //sumGradient += sol;
            //numSums++;
            //Eigen::VectorXd averageGradient = sumGradient/numSums;
            //double diffAverage = (sol-averageGradient).norm();
            //std::cout << "Average gradient Diff " << diffAverage << std::endl;
            //prevGradient = sol;

            //std::cout << "Gradient Mag: " << sol.norm() << std::endl;
            //Eigen::VectorXd normGradient = sol.normalized();

            /*DataObject gradient;
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
            }*/

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
    double lastTime;
    int updateCompleted;
    std::vector<IModelSample*> samples;
    IUpdateCallback* callback;
    std::mutex updateMutex;
    std::vector<std::string> paramKeys;
    int iteration;
    Eigen::VectorXd v;
    std::vector<int> sortedIndexes;
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