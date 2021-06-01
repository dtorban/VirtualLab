#include "VirtualLab/pca/PCAModel.h"

#ifdef USE_MLPACK
#include <mlpack/prereqs.hpp>
#include <mlpack/methods/pca/pca.hpp>
#include <mlpack/methods/pca/decomposition_policies/exact_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/quic_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/randomized_svd_method.hpp>
using namespace mlpack;
using namespace mlpack::pca;
using namespace mlpack::util;

#include <mlpack/methods/kmeans/kmeans.hpp>
using namespace mlpack::kmeans;

namespace vl {

template<typename DecompositionPolicy>
void RunPCA(arma::mat& dataset,
            const size_t newDimension,
            const bool scale,
            const double varToRetain)
{
  PCA<DecompositionPolicy> p(scale);
  double varRetained;

    varRetained = p.Apply(dataset, newDimension);
}

}

#endif

namespace vl {

class PCAModelSample : public IModelSample, public IDataConsumer {
public:
	PCAModelSample(const DataObject& params, PCAModel::SampleInfo& info) : params(params), callback(NULL), kmeans_calc(-1), prevColumnSize(0), info(info) {
        data["pca"] = DataArray();
        data["bounds"] = DataArray();
        pca = &data["pca"].get<vl::Array>();
        bound = &data["bounds"].get<vl::Array>();
    }

	virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }
    virtual void update();
	void update(IUpdateCallback* callback);
	virtual void consume(IModel& model, IModelSample& sample);

private:
    DataObject params;
    DataObject nav;
    DataObject data;
    vl::Array* pca;
    vl::Array* bound;
	IUpdateCallback* callback;
    int kmeans_calc;
    DataObject keys;
    std::map<std::string, int> keyType;
    int prevColumnSize;
    PCAModel::SampleInfo& info;
#ifdef USE_MLPACK
    arma::Row<size_t> assignments;
    arma::mat centroids;
#endif
};


IModelSample* PCAModel::create(const DataObject& params) {
		PCAModelSample* sample = new PCAModelSample(params, info);
		consumers.push_back(sample);
		return sample;
}

void PCAModelSample::update() {
    if (callback) {
        //std::cout << "update PCA" << std::endl;

        nav["keys"] = keys;
        DataObject zoom = nav["zoom"].get<vl::Object>();
        double zoomK = zoom["k"].get<double>();
        double zoomX = zoom["x"].get<double>();
        double zoomY = zoom["y"].get<double>();
        //std::cout << zoom["k"].get<double>() << std::endl;
        int params = 0;
        int other = 0;

        std::vector<std::string> cols;
        for (DataObject::const_iterator it = keys.begin(); it != keys.end(); it++) {
            if (it->second.get<double>() > 0.0001) {
                cols.push_back(it->first);
                int type = keyType[it->first];
                if (type == 0) {
                    params++;
                }
                else {
                    other++;
                }
            }
        }
        //std::cout << numCols << std::endl;

        if (prevColumnSize != cols.size()) {
            kmeans_calc = -1;
        }
        prevColumnSize = cols.size();

        pca->clear();
        bound->clear();

#ifdef USE_MLPACK
        if (cols.size() > 1 && (info.dataRows.size() > 2 || info.samplePtr.size() > 2)) {
                int numRows = info.dataRows.size();
                if (params > 0 && other == 0) {
                    numRows = info.paramRows.size();
                }

                arma::mat A(numRows, cols.size());
                
                if (params > 0 && other ==0) {
                    int i = 0;
                    for (std::map<IModelSample*, DataObject>::iterator it = info.paramRows.begin(); it != info.paramRows.end(); it++) {
                        ParameterHelper helper(it->second);
                        for (int f = 0; f < cols.size(); f++) {
                            A(i,f) = helper.scale(cols[f], it->second[cols[f]].get<double>());
                        }
                        i++;
                    }
                }
                else {
                    //B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
                    for (int i = 0; i < info.dataRows.size(); i++) {

                        DataObject& ps = info.paramRows[info.samplePtr[i]];
                        ParameterHelper helper(ps);
                        for (int f = 0; f < cols.size(); f++) {
                            double val;
                            const std::string& key = cols[f];
                            int type = keyType[key];
                            switch (type)
                            {
                            case 0:
                                val = helper.scale(key, ps[key].get<double>());
                                break;
                            case 1:
                                val = info.navRows[i][key].get<double>();
                                break;
                            case 2:
                                val = info.dataRows[i][key].get<double>();
                                break;
                            }
                            A(i,f) = val;
                        }
                    }
                }

                A = A.t();
                
                if (cols.size() != 2) { 
                    RunPCA<ExactSVDPolicy>(A, 2, true, 1.0);
                }

                int clusterNum = this->params["clusters"].get<double>();

                if (clusterNum > 0) {
                    int blah = numRows/100;
                    if (kmeans_calc < blah) {
                        //std::cout << "Calc KMeans" << std::endl;
                        kmeans_calc++;
                        // The dataset we are clustering.
                        //extern arma::mat data;
                        // The number of clusters we are getting.
                        //extern size_t clusters = 5;
                        // The assignments will be stored in this vector.
                        //arma::Row<size_t> assignments;
                        // The centroids will be stored in this matrix.
                        //arma::mat centroids;
                        // Initialize with the default arguments.
                        KMeans<> k;
                        k.Cluster(A, clusterNum, assignments, centroids);
                    }
                }
                
                double bounds[4];

                for (int f = 0; f < numRows; f++) {
                    if (f == 0) {
                        // x,y min
                        bounds[0] = A(0,f);
                        bounds[1] = A(1,f);
                        // x,y max
                        bounds[2] = A(0,f);
                        bounds[3] = A(1,f);
                    }
                    else {
                        if (bounds[0] > A(0,f)) { bounds[0] = A(0,f); }
                        if (bounds[1] > A(1,f)) { bounds[1] = A(1,f); }
                        if (bounds[2] < A(0,f)) { bounds[2] = A(0,f); }
                        if (bounds[3] < A(1,f)) { bounds[3] = A(1,f); } 
                    }
                }

                for (int f = 0; f < numRows; f++) {
                    if (f % 1 == 0) {
                        
                        vl::DataObject obj;
                        double x = A(0,f);
                        double y = A(1,f);
                        obj["x"] = DoubleDataValue(x);
                        obj["y"] = DoubleDataValue(y);
                        if (assignments.size() <= f) {
                            obj["cluster"] = DoubleDataValue(1);
                        }
                        else {
                            obj["cluster"] = DoubleDataValue(1);//assignments[f]+1);
                        }
                        double radius = std::sqrt(std::pow(x - (bounds[2]+bounds[0])/2.0 + zoomX*(bounds[2]-bounds[0])/zoomK ,2) + std::pow(y - (bounds[3]+bounds[1])/2.0 - zoomY*(bounds[3]-bounds[1])/zoomK,2));
                        //std::cout << radius << std::endl;
                        if (false) {//radius < 1000.0) {
                            obj["cluster"] = DoubleDataValue(0);
                        }
                        pca->push_back(obj);
                    }
                }

                /*if (centroids.size() == clusterNum*2) {                   
                    for (int f = 0; f < clusterNum; f++) {
                            vl::DataObject obj;
                            obj["x"] = DoubleDataValue(centroids(0,f));
                            obj["y"] = DoubleDataValue(centroids(1,f));
                            obj["cluster"] = DoubleDataValue(0);
                            pca->push_back(obj);
                    }
                }*/

                for (int i = 0; i < 4; i++) {
                    bound->push_back(DoubleDataValue(bounds[i]));
                }
        }
#endif

        callback->onComplete();
        delete callback;
    }
    callback = NULL;
}

void PCAModelSample::update(IUpdateCallback* callback) {
    keys = this->nav["keys"];
    this->callback = callback;
}

void PCAModelSample::consume(IModel& model, IModelSample& sample) {

    const DataObject& obj = sample.getData();
    const DataObject& nav = sample.getNavigation();
    const DataObject& params = sample.getParameters();

    /*navRows.push_back(nav);
    dataRows.push_back(obj);
    samplePtr.push_back(&sample);
    if (paramRows.find(&sample) == paramRows.end()) {
        paramRows[&sample] = sample.getParameters();
    }*/

    bool paramsEnabled = this->params["params"].get<double>() > 0.0001 ? 1 : 0;
 
    for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
        if (it->second.isType<double>()) {   
            if (keys.find(it->first) == keys.end()) {
                std::cout << it->first << " " << paramsEnabled << std::endl;
                keys[it->first] = DoubleDataValue(paramsEnabled);
                keyType[it->first] = 0;
            }
        }
    }
    for (DataObject::const_iterator it = nav.begin(); it != nav.end(); it++) {
        if (it->second.isType<double>()) { 
            if (keys.find(it->first) == keys.end()) {
                std::cout << it->first << std::endl;
                keys[it->first] = DoubleDataValue(0);
                keyType[it->first] = 1;
            }
        }
    }
    for (DataObject::const_iterator it = obj.begin(); it != obj.end(); it++) {
        if (it->second.isType<double>()) { 
            if (keys.find(it->first) == keys.end()) {
                std::cout << it->first << std::endl;
                keys[it->first] = DoubleDataValue(0);
                keyType[it->first] = 2;
            }
        }
    }

    this->nav["keys"] = keys;

    const Object& dataSetObj = obj.get<Object>();

	update();
}

}