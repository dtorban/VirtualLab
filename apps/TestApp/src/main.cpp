/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
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

	/*Client api;
	IModel* model = api.getModels()[1];
	std::cout << model->getName() << std::endl;
	
	DataObject params = model->getParameters();
	params["N"].set<double>(10);
	IModelSample* sample = model->create(params);
	
	std::cout << "Parameters: " << JSONSerializer::instance().serialize(sample->getParameters()) << std::endl;
	std::cout << "Navigation: " << JSONSerializer::instance().serialize(sample->getNavigation()) << std::endl;
	std::cout << "Data: " << JSONSerializer::instance().serialize(sample->getData()) << std::endl;

	sample->getNavigation()["m"].set<double>(1);

	for (int i = 0; i < 10; i++) {
		sample->getNavigation()["t"].set<double>(0.1*i);
		sample->update();
		std::cout << "t=" << sample->getNavigation()["t"].get<double>() << ": ";
		std::cout << JSONSerializer::instance().serialize(sample->getData()) << std::endl;
		std::cout << std::endl;
	}*/

	return 0;
}


