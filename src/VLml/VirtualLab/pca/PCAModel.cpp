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
namespace vl {

IModelSample* PCAModel::create(const DataObject& params) {
    return model->create(params);
}

}
#endif