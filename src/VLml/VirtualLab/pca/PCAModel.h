#ifndef VIRTUALLAB_PCA_PCA_MODEL_H_
#define VIRTUALLAB_PCA_PCA_MODEL_H_

#include "VirtualLab/IModel.h"
#include "VirtualLab/impl/VirtualLabAPI.h"

namespace vl {

class PCAModel : public IModel, public IDataConsumer {
public:
	PCAModel(const std::string& name, IDataProducer* producer) : name(name) {
		producer->addConsumer(this);
	}
	const std::string& getName() const { return name; }
    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params);

    virtual void consume(IModel& model, IModelSample& sample) {
		//std::cout << sample.getData()["x"].get<double>() << " " << sample.getData()["y"].get<double>() << std::endl;
		//std::cout << model.getName() << " " << &sample << std::endl;
		for (int i = 0; i < consumers.size(); i++) {
			consumers[i]->consume(model, sample);
		}
	}

private:
	std::string name;
	DataObject params;
	std::vector<IDataConsumer*> consumers;
};

/*class PCAModel : public IModel {
public:
    PCAModel(const std::string& name, IModel* model) : name(name), model(model) {
        params = model->getParameters();
    }
    virtual ~PCAModel() {
        delete model;
    }

    const std::string& getName() const { return name; }

    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params);

private:
    IModel* model;
    std::string name;
    DataObject params;
};
*/

}

#endif