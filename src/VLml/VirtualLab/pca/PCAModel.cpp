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

#ifdef USE_MLPACK2

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

class PCASample : public IModelSample {
public:
    PCASample(DataObject params, IModelSample* sample) : params(params), sample(sample) {
        data["data"] = DataObject();
        data["pca"] = DataArray();
        data["pca2"] = DataArray();
        da = &data["data"].get<vl::Object>();
        d = &data["pca"].get<vl::Array>();
        d2 = &data["pca2"].get<vl::Array>();

        nav = sample->getNavigation();
    }

    virtual ~PCASample() {
        delete sample;
    }

    virtual const DataObject& getParameters() const { return params; }
    virtual DataObject& getNavigation() { return nav; }
    virtual const DataObject& getData() const { return data; }

    virtual void update(int id, Object& prev, Object& obj) {

        double x = prev["x"].get<double>();
        double y = prev["y"].get<double>();
        double dx = obj["x"].get<double>() - x;
        double dy = obj["y"].get<double>() - y;
        double dist = std::sqrt(std::pow(dx,2.0)+std::pow(dy,2.0)) ;

        arma::rowvec r;
		r << obj["actin"].get<double>()
			<< obj["aflow"].get<double>() 
			<< obj["en"].get<double>() 
			<< obj["free_actin"].get<double>() 
			<< obj["nm"].get<double>()
			<< std::sqrt(std::pow(obj["fx"].get<double>(),2.0)+std::pow(obj["fy"].get<double>(),2.0)) 
            << dist
			<< sample->getData()["rmc"].get<double>()
			<< arma::endr;
        if (nav["t"].get<double>() > 0.0) {
		    rows.push_back(r);
        }



    }

    virtual void update() {

        std::vector<Object> prev;
        //std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
        /*for (int i = 0; i < sample->getData()["data"].get<vl::Array>().size(); i++) {
            prev.push_back(sample->getData()["data"].get<vl::Array>()[i].get<Object>());
        }*/
        prev.push_back(sample->getData().get<Object>());

        sample->getNavigation() = nav;
        sample->update();
        
        vl::Array array;
        vl::Array array2;
        if (prev.size() > 0) {
            /*for (int i = 0; i < sample->getData()["data"].get<vl::Array>().size(); i++) {
                Object obj = sample->getData()["data"].get<vl::Array>()[i].get<Object>();
                update(i, prev[i], obj);
            }*/
            Object obj = sample->getData().get<Object>();
            update(0, prev[0], obj);

            if (rows.size() > 2) {
                arma::mat A(rows.size(), rows[0].n_cols);
            
                //B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
                for (int i = 0; i < rows.size(); i++) {
                    A.row(i) = rows[i];
                }
                A = A.t();
                RunPCA<ExactSVDPolicy>(A, 2, true, 1.0);
                A = A.t();


                for (int f = 0; f < rows.size(); f+=10*prev.size()) {
                    for (int i = 0; i < prev.size(); i++) {
                        vl::DataObject obj;
                        //std::cout << A.row(f);
                        obj["x"] = DoubleDataValue(A(f+i,0));
                        obj["y"] = DoubleDataValue(A(f+i,1));
                        obj["id"] = DoubleDataValue(i);
                        obj["t"] = DoubleDataValue(f/prev.size());
                        array2.push_back(obj);
                    }
                }

                for (int f = rows.size() - 20*prev.size(); f < rows.size(); f+=1*prev.size()) {
                    for (int i = 0; i < prev.size(); i++) {
                        vl::DataObject obj;
                        //std::cout << A.row(f);
                        obj["x"] = DoubleDataValue(A(f+i,0));
                        obj["y"] = DoubleDataValue(A(f+i,1));
                        obj["id"] = DoubleDataValue(i);
                        obj["t"] = DoubleDataValue(f/prev.size());
                        array.push_back(obj);
                    }
                }
            }
        }
        *d = array;
        *d2 = array2;
        *da = sample->getData().get<vl::Object>();
    }

private:
    IModelSample* sample;
    DataObject params;
    DataObject nav;
    DataObject data;
    vl::Array* d;
    vl::Array* d2;
    vl::Object* da;
	std::vector<arma::rowvec> rows;
};

IModelSample* PCAModel::create(const DataObject& params) {
    return new PCASample(params, model->create(params));
}

}

#else

#endif

namespace vl {

class PCAModelSample : public IModelSample, public IDataConsumer {
public:
	PCAModelSample(const DataObject& params) : params(params), callback(NULL), kmeans_calc(-1) {
        data["pca"] = DataArray();
        pca = &data["pca"].get<vl::Array>();
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
	IUpdateCallback* callback;
    int kmeans_calc;
    std::vector<std::string> columns;
    std::vector<DataObject> dataRows;
    DataObject keys;
#ifdef USE_MLPACK
	std::vector<arma::rowvec> rows;
    //arma::mat prevCentroids;
    arma::Row<size_t> assignments;
    arma::mat centroids;
#endif
};


IModelSample* PCAModel::create(const DataObject& params) {
		PCAModelSample* sample = new PCAModelSample(params);
		consumers.push_back(sample);
		return sample;
}

void PCAModelSample::update() {
    if (callback) {
        //std::cout << "update PCA" << std::endl;

        nav["keys"] = keys;

        int numCols = 0;
        for (DataObject::const_iterator it = keys.begin(); it != keys.end(); it++) {
            if (it->second.get<double>() > 0.0001) {
                numCols++;
            }
        }
        //std::cout << numCols << std::endl;

        pca->clear();

#ifdef USE_MLPACK
        if (numCols > 1 && rows.size() > 2) {
                arma::mat A(rows.size(), rows[0].n_cols);
            
                //B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
                for (int i = 0; i < rows.size(); i++) {
                    A.row(i) = rows[i];
                }

                A = A.t();
                
                
                RunPCA<ExactSVDPolicy>(A, 2, true, 1.0);

                int clusterNum = 10;

                //std::cout << kmeans_calc << " " << rows.size()/100 << std::endl;
                int blah = rows.size()/100;
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
                

                /*std::vector<int> oldIndices;
                if (prevCentroids.size() == centroids.size()) {
                    // Figure out reordering
                    //std::cout << prevCentroids << " " << prevCentroids.size() << std::endl;
                    //std::cout << centroids << " " << centroids.size() << std::endl;
                    

                    for (int i = 0; i < 5; i++)  {
                        double min;
                        int oldIndex = 0;
                        for (int f = 0; f < 5; f++)  {
                            double val = std::pow(prevCentroids(0,i)-centroids(0,f),2) + std::pow(prevCentroids(1,i)-centroids(1,f),2);
                            if (f == 0 || val < min) {
                                min = val;
                                oldIndex = f;
                            }
                        }
                        oldIndices.push_back(oldIndex);
                    }
                }
                else {
                    for (int i = 0; i < 5; i++)  {
                        oldIndices.push_back(i);
                    }
                }
                prevCentroids = centroids;*/

                //A = A.t();

                //pca->clear();

                for (int f = 0; f < rows.size(); f++) {
                        vl::DataObject obj;
                        //std::cout << A.row(f);
                        obj["x"] = DoubleDataValue(A(0,f));
                        obj["y"] = DoubleDataValue(A(1,f));
                        //obj["id"] = DoubleDataValue(i);
                        //obj["cluster"] = DoubleDataValue(oldIndices[assignments[f]]+1);
                        if (assignments.size() <= f) {
                            obj["cluster"] = DoubleDataValue(1);
                        }
                        else {
                            obj["cluster"] = DoubleDataValue(1);//assignments[f]+1);
                            //obj["cluster"] = DoubleDataValue(oldIndices[assignments[f]]+1);
                        }
                        //obj["cluster"] = DoubleDataValue(1);
                        //obj["t"] = DoubleDataValue(f/prev.size());
                        pca->push_back(obj);
                }

                if (centroids.size() == clusterNum*2) {
                    //std::cout << centroids << std::endl;
                    
                    for (int f = 0; f < clusterNum; f++) {
                            vl::DataObject obj;
                            //std::cout << A.row(f);
                            obj["x"] = DoubleDataValue(centroids(0,f));
                            obj["y"] = DoubleDataValue(centroids(1,f));
                            //obj["id"] = DoubleDataValue(i);
                            obj["cluster"] = DoubleDataValue(0);
                            //obj["t"] = DoubleDataValue(f/prev.size());
                            pca->push_back(obj);
                    }
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
    //ModelSampleDecorator::update(callback);
    //producer->produce(*model, *sample);
}

void PCAModelSample::consume(IModel& model, IModelSample& sample) {
	//std::cout << sample.getData()["x"].get<double>() << " " << sample.getData()["y"].get<double>() << std::endl;
	//std::cout << model.getName() << " " << &sample << std::endl;

    const DataObject& obj = sample.getData();
    const DataObject& nav = sample.getNavigation();

    dataRows.push_back(obj);
    for (DataObject::const_iterator it = obj.begin(); it != obj.end(); it++) {
        if (keys.find(it->first) == keys.end()) {
            std::cout << it->first << std::endl;
            keys[it->first] = DoubleDataValue(0);
        }
    }

    this->nav["keys"] = keys;

#ifdef USE_MLPACK
    //std::cout << " time: " << nav["t"].get<double>() << " " << rows.size() << std::endl;

    const Object& dataSetObj = obj.get<Object>();

    if (columns.size() == 0) {
        for (Object::const_iterator it = dataSetObj.begin(); it != dataSetObj.end(); it++) {
            if(it->second.isType<double>()) {
                columns.push_back(it->first);
            }
        }
    }

    if (columns.size() > 0) {
        arma::rowvec r(columns.size());
        if (nav["t"].get<double>() > 0.0) {
            for (int i = 0; i < columns.size(); i++) {
                r(i) = obj[columns[i]].get<double>();
            }
            rows.push_back(r);
        }
    }
    

    /*arma::rowvec r;
    if (nav["t"].get<double>() > 0.0) {
        r << obj["actin"].get<double>()
        << obj["aflow"].get<double>() 
        << obj["en"].get<double>() 
        << obj["free_actin"].get<double>() 
        << obj["nm"].get<double>()
        << std::sqrt(std::pow(obj["fx"].get<double>(),2.0)+std::pow(obj["fy"].get<double>(),2.0)) 
        //<< dist
        << obj["rmc"].get<double>()
        << arma::endr;
        rows.push_back(r);
    }*/
#endif

	update();
}

/*IModelSample* PCAModel::create(const DataObject& params) {
    return model->create(params);
}*/

}