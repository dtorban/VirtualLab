#ifndef VIRTUALLAB_PCA_PCA_MODEL_H_
#define VIRTUALLAB_PCA_PCA_MODEL_H_

#include "VirtualLab/IModel.h"
#include "VirtualLab/impl/VirtualLabAPI.h"

namespace vl {

class PCAModel : public IModel, public IDataConsumer {
public:
	PCAModel(const std::string& name, IDataProducer* producer) : name(name) {
		producer->addConsumer(this);
        params["params"] = DoubleDataValue(0);
        params["navigation"] = DoubleDataValue(0);
        params["data"] = DoubleDataValue(0);
        params["clusters"] = DoubleDataValue(10);
	}
	const std::string& getName() const { return name; }
    const DataObject& getParameters() { return params; }

    IModelSample* create(const DataObject& params);

    virtual void consume(IModel& model, IModelSample& sample) {
        const DataObject& obj = sample.getData();
        const DataObject& nav = sample.getNavigation();
        const DataObject& params = sample.getParameters();

        info.navRows.push_back(nav);
        info.dataRows.push_back(obj);
        info.samplePtr.push_back(&sample);
        if (info.paramRows.find(&sample) == info.paramRows.end()) {
            info.paramRows[&sample] = sample.getParameters();
        }
        
		//std::cout << sample.getData()["x"].get<double>() << " " << sample.getData()["y"].get<double>() << std::endl;
		//std::cout << model.getName() << " " << &sample << std::endl;
		for (int i = 0; i < consumers.size(); i++) {
			consumers[i]->consume(model, sample);
		}
	}

    struct SampleInfo {
        std::vector<DataObject> dataRows;
        std::vector<DataObject> navRows;
        std::vector<IModelSample*> samplePtr;
        std::map<IModelSample*, DataObject> paramRows;
    };

private:
	std::string name;
	DataObject params;
	std::vector<IDataConsumer*> consumers;
    SampleInfo info;
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