/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <cmath>
#include "VirtualLab/net/Client.h"
#include "VirtualLab/DataValue.h"
#include "VirtualLab/util/JSONSerializer.h"

#include <mlpack/prereqs.hpp>
#include <mlpack/methods/pca/pca.hpp>
#include <mlpack/methods/pca/decomposition_policies/exact_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/quic_svd_method.hpp>
#include <mlpack/methods/pca/decomposition_policies/randomized_svd_method.hpp>
using namespace mlpack;
using namespace mlpack::pca;
using namespace mlpack::util;
using namespace vl;

// Run RunPCA on the specified dataset with the given decomposition method.
template<typename DecompositionPolicy>
void RunPCA(arma::mat& dataset,
            const size_t newDimension,
            const bool scale,
            const double varToRetain)
{
  PCA<DecompositionPolicy> p(scale);

  std::cout << "Performing PCA on dataset..." << std::endl;
  double varRetained;

    varRetained = p.Apply(dataset, newDimension);

  std::cout << (varRetained * 100) << "% of variance retained (" <<
      dataset.n_rows << " dimensions)." << std::endl;
}

int main(int argc, char**argv) {
	//arma::mat dataset(10, 10);
	arma::Mat<double> A;
	A.load("iris.csv");
	A = A.t();

	std::cout << A << std::endl;
	RunPCA<ExactSVDPolicy>(A, 2, 1.0, 1.0);
	std::cout << A.t() << std::endl;
	std::cout << A(0,0) << std::endl;
	std::vector<arma::rowvec> rows;

	Client api;
	IModel* model = api.getModels()[0];
	std::cout << model->getName() << std::endl;
	
	DataObject params = model->getParameters();
	params["N"].set<double>(1);
	IModelSample* sample = model->create(params);
	
	std::cout << "Parameters: " << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << "Navigation: " << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << "Data: " << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	sample->getNavigation()["m"].set<double>(1);

	for (int i = 0; i < 10000; i++) {
		sample->getNavigation()["t"].set<double>(1.0*i + 1.0);
		sample->update();
		std::cout << "t=" << sample->getNavigation()["t"].get<double>() << ": ";
		std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
		std::cout << std::endl;
		
		arma::rowvec r;
		r << sample->getData()["actin"].get<double>()
			<< sample->getData()["aflow"].get<double>() 
			<< sample->getData()["en"].get<double>() 
			<< sample->getData()["free_actin"].get<double>() 
			<< sample->getData()["nm"].get<double>()
			<< std::sqrt(std::pow(sample->getData()["fx"].get<double>(),2.0)+std::pow(sample->getData()["fy"].get<double>(),2.0)) 
			<< arma::endr;
		rows.push_back(r);

/*{"actin":15230.475345225901,"aflow":74.275631905510465,"en":114, 
"ev":{"cell":5,"module_bind":135,"module_unbind":24},"free_actin":84769.524654774097,"fx":-8.0839199746521098e-14,"fy":-1.4002173181243537e-13,
"m":[{"en":40,"fx":-38.465570992414456,"fy":66.624323301009397,"id":228,"x":-2576.5200079803994,"y":4462.6635605398187},{"en":45,"fx":-38.965388028526625,"fy":-67.490031802044101,"id":229,"x":-2576.5200079804026,"y":-4462.6635605398178},{"en":29,"fx":75.919791408226274,"fy":-2.3378581759322738e-14,"id":230,"x":5153.0400159608007,"y":-1.2717674669499602e-12}],"nm":3,"x":1.8188819942967246e-13,"y":3.1504889657797943e-13},{"actin":15232.147450933153,"aflow":75.528598943551231,"en":139,"ev":{"cell":12,"module_bind":147,"module_unbind":11},"free_actin":84767.852549066854,"fx":-0.0031984719872663412,"fy":-0.2234055109620669,"m":[{"en":44,"fx":-38.027799682203401,"fy":65.730882119921219,"id":231,"x":-2576.6427750223388,"y":4462.616081774042},{"en":41,"fx":-37.776356301752443,"fy":-65.436847086113275,"id":232,"x":-2574.1934942613452,"y":-4458.6510756479474},{"en":54,"fx":75.807354456125253,"fy":-0.070629523228745106,"id":233,"x":5153.0602739471806,"y":-0.13862828788316794}],
"nm":3,"x":2.4532081300016739,"y":4.6601794796114717}]}*/
		if (rows.size() > 2) {
			
			arma::mat B(rows.size(), rows[0].n_cols);
			//B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
			for (int i = 0; i < rows.size(); i++) {
				B.row(i) = rows[i];
			}
			B = B.t();
			RunPCA<ExactSVDPolicy>(B, 2, true, 1.0);
			B = B.t();
			//std::cout << B.t() << std::endl;
			for (int f = 100; f < rows.size(); f+=100) {
				std::cout << B.row(f);
			}
		}

	}
  
		/*arma::mat B(rows.size(), rows[0].n_cols);
		//B << sample->getNavigation()["t"].get<double>() << arma::endr << sample->getNavigation()["t"].get<double>() << arma::endr;
		for (int i = 0; i < rows.size(); i++) {
			B.row(i) = rows[i];
		}*/

	//std::cout << B << std::endl;

	/*B = B.t();
	std::cout << A << std::endl;
	RunPCA<ExactSVDPolicy>(B, 2, true, 1.0);
	std::cout << B.t() << std::endl;*/

	return 0;
}


