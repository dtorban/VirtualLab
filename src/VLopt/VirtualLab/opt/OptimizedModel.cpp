#include "VirtualLab/opt/OptimizedModel.h"
#include "VirtualLab/opt/LeastSquares.h"

namespace vl {

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
            if (it->second.isType<double>()) {
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


        for (int i = 0; i < testSamples.size(); i++) {
            Eigen::VectorXd diff = Eigen::VectorXd(paramKeys.size());
            for (int j = 0; j < paramKeys.size(); j++) {
                diff[j] = testSamples[i]->getParameters()[paramKeys[j]].get<double>() - sample->getParameters()[paramKeys[j]].get<double>();
            }

            b[i] = distFunction->calculate(*sample, *testSamples[i]) / diff.norm();
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

        ParameterHelper helper(getParameters());

        for (int i = 0; i < paramKeys.size(); i++) {
            std::string key = paramKeys[i];
            double val = sample->getParameters()[key].get<double>();
            std::cout << "params." << key << "=" << helper.clamp(key, val-sol[i]*2.0) << "; //" << val << "," << sol[i] << std::endl;
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