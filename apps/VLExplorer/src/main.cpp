/**
Copyright (c) 2019 Dan Orban
*/

#include <iostream>
#include <map>
#include <algorithm>
#include <random>
#include <chrono>
#include "WebServer.h"
#include "VirtualLab/net/Server.h"
#include "VirtualLab/net/Client.h"
#include "VirtualLab/impl/TestModel.h"
#include "VirtualLab/util/JSONSerializer.h"
#include "VirtualLab/pca/PCAModel.h" 
#include "VirtualLab/opt/OptimizedModel.h"
#include <mutex>

class VLWebServerSession;
class VLWebServerCommand;

using namespace vl;

struct VLWebServerSessionState {
	std::map<std::string, VLWebServerCommand*> commands;
	IVirtualLabAPI* api;
	std::vector<ModelProxy> models;
};

class VLWebServerCommand {
public:
	virtual ~VLWebServerCommand() {}
	virtual void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) = 0;
};

class VLWebServerSession : public JSONSession {
public:
	VLWebServerSession(VLWebServerSessionState state) : state(state), producerAPI(*state.api), currentSampleIndex(0) {
		IVirtualLabAPI* api = this->state.api;
		//producerAPI.registerModel(new TestModel());
		//producerAPI.registerModel(new ModelProxy(api->getModels()[0]));
		compositeApi.registerModel(new PCAModel("PCA", &producerAPI));
		compositeApi.registerModel(new SamplingModel("Sampling", producerAPI.getModels()[3]));
        compositeApi.registerModel(new OptimizedModel2(new ModelProxy(producerAPI.getModels()[4]), 0.01, 5));
		compositeApi.addApi(producerAPI);
		this->state.api = &compositeApi;
		this->state.models = this->state.api->getModels();
	}
	~VLWebServerSession() {
		for (std::map<int, IModelSample*>::iterator it = samples.begin(); it != samples.end(); it++) {
			delete it->second;
		}
	}
	void receiveJSON(picojson::value& val) {
		std::string cmd = val.get<picojson::object>()["command"].get<std::string>();
		std::map<std::string, VLWebServerCommand*>::iterator it = state.commands.find(cmd);
		if (it != state.commands.end()) {
			it->second->execute(this, val, &state);
		}
	}

	void onWrite() {
		std::unique_lock<std::mutex> lock(messageMutex);
		JSONSession::onWrite();
	}

	void sendMessage(const std::string& msg) {
		std::unique_lock<std::mutex> lock(messageMutex);
		JSONSession::sendMessage(msg);
	}

	IModelSample* getSample(int id) {
		std::map<int, IModelSample*>::iterator it = samples.find(id);
		if (it != samples.end()) {
			return it->second;
		}
		else {
			return NULL;
		}
	}

	int addSample(IModelSample* sample) {
		samples[currentSampleIndex] = sample;
		currentSampleIndex++;
		return currentSampleIndex-1;
	}

	void deleteSample(int id) {
		std::map<int, IModelSample*>::iterator it = samples.find(id);
		delete it->second;
		samples.erase(it);
	}

private:
	VLWebServerSessionState state;
	std::map<int, IModelSample*> samples;
	int currentSampleIndex;
	ProducerAPI producerAPI;
	CompositeAPI compositeApi;
	std::mutex messageMutex;
};

class DeleteSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double sampleId = command.get<picojson::object>()["sampleId"].get<double>();

		session->deleteSample(sampleId);

		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class UpdateSampleCommand : public VLWebServerCommand {
public:
	class UpdateCallback : public IUpdateCallback {
	public:
		UpdateCallback(picojson::object data, VLWebServerSession* session, IModelSample* sample) : data(data), session(session), sample(sample) {}
		virtual ~UpdateCallback() {
			//std::cout << "delete updatecallback" << std::endl;
		}

		void onComplete() {
			data["nav"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getNavigation()));
			data["data"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getData()));


			double sampleId = data["sampleId"].get<double>();
			
			picojson::value ret(data);
			session->sendJSON(ret);
			
		}

	private:
		picojson::object data;
		VLWebServerSession* session;
		IModelSample* sample;
	};

	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double sampleId = command.get<picojson::object>()["sampleId"].get<double>();

		IModelSample* sample = session->getSample(sampleId);
		if (sample) {

			JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["nav"], sample->getNavigation());
			//std::cout << JSONSerializer::instance().serializeJSON(sample->getNavigation()) << std::endl;
			// TODO: fix synchrounous update
			//sample->update();
			//UpdateCallback cb(data, session, sample);
			//cb.onComplete();
			sample->update(new UpdateCallback(data, session, sample));

			/*data["nav"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getNavigation()));
			data["data"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getData()));
			std::cout << JSONSerializer::instance().serializeJSON(sample->getData()) << std::endl;

			picojson::value ret(data);
			session->sendJSON(ret);*/

		}

	}


};

class CreateSampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double modelIndex = command.get<picojson::object>()["index"].get<double>();
		DataObject obj;
		JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["params"], obj);

		IModelSample* sample = state->models[modelIndex].create(obj);
		int id = session->addSample(sample);

		data["sampleId"] = picojson::value((double)id);
		data["nav"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getNavigation()));
		data["params"] = picojson::value(JSONSerializer::instance().serializeJSON(sample->getParameters()));
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class SampleCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		DataObject samplerParams;
		JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["samplerParams"], samplerParams);
		DataObject params;
		JSONSerializer::instance().deserializeJSON(command.get<picojson::object>()["params"], params);

		CompositeSampler localSampler;
		LatinHypercubeSampler latinSampler(5);
		IModelSampler* sampler = NULL;

		//std::cout << samplerParams["sampler"].get<std::string>() << std::endl;

		if (samplerParams["sampler"].get<std::string>() == "neighbor") {
			for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
				if (it->second.isType<double>() && it->first != "N") {
					localSampler.addSampler(new LocalRandomSampler(it->first, samplerParams["closeness"].get<double>()));
				}
			}
			sampler = &localSampler;
		}
		else if (samplerParams["sampler"].get<std::string>() == "latin") {
			
			for (DataObject::const_iterator it = params.begin(); it != params.end(); it++) {
				if (it->second.isType<double>() && it->first != "N") {
					latinSampler.addParameter(it->first);
				}
			}
			sampler = &latinSampler;
		}

		if (sampler) {
			if (!sampler->hasNext()) {
				sampler->reset();
			}
			sampler->sample(params);
			sampler->next();
		}


		data["params"] = picojson::value(JSONSerializer::instance().serializeJSON(params));
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class GetParametersCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		double modelIndex = command.get<picojson::object>()["index"].get<double>();

		data["params"] = picojson::value(JSONSerializer::instance().serializeJSON(state->models[modelIndex].getParameters()));
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class InitCommand : public VLWebServerCommand {
public:
	void execute(VLWebServerSession* session, picojson::value& command, VLWebServerSessionState* state) {
		picojson::object data = command.get<picojson::object>();
		picojson::array modelNames;
		for (int i = 0; i < state->models.size(); i++) {
			modelNames.push_back(picojson::value(state->models[i].getName()));
		}
		data["models"] = picojson::value(modelNames);
		
		picojson::value ret(data);
		session->sendJSON(ret);

	}
};

class MyDataConsumer : public IDataConsumer {
public:
    virtual void consume(IModel& model, IModelSample& sample) {
		std::cout << sample.getData()["x"].get<double>() << " " << sample.getData()["y"].get<double>() << std::endl;
		//std::cout << model.getName() << " " << &sample << std::endl;
	}
};



int main(int argc, char**argv) {
	std::cout << "Usage: ./bin/ExampleServer 8081 path/to/web" << std::endl;

	IVirtualLabAPI* api;

	Client client;
	api = new LoadBalancedAPI(&client);
	//ModelProxy cell = api->getModels()[0];
	//ModelProxy smothedCell = api->getModels()[1];
	//api->registerModel(new TypedModelDecorator<ModifiedSample>("Modified Cell", &cell));
	//api->registerModel(new TypedModelDecorator<ModifiedSample>("Modified Smooth Cell", &smothedCell));
    //api->registerModel(new OptimizedModel2(new ModelProxy(api->getModels()[4]), 0.01, 5));
	api->registerModel(new TestModel("Test"));

	/*ProducerAPI producerAPI(client);
	producerAPI.registerModel(new TestModel());
	producerAPI.registerModel(new ModelProxy(client.getModels()[0]));
	CompositeAPI compositeApi;
	compositeApi.registerModel(new PCAModel(&producerAPI));
	compositeApi.addApi(producerAPI);
	api = &compositeApi;*/


	if (argc > 2) {
		int port = std::atoi(argv[1]);
		std::string webDir = std::string(argv[2]);

        bool running = true;

		VLWebServerSessionState state;
		state.commands["init"] = new InitCommand();
		state.commands["getParameters"] = new GetParametersCommand();
		state.commands["createSample"] = new CreateSampleCommand();
		state.commands["updateSample"] = new UpdateSampleCommand();
		state.commands["deleteSample"] = new DeleteSampleCommand();
		state.commands["sample"] = new SampleCommand();
		state.api = api;
		//state.models = api->getModels();
		WebServerWithState<VLWebServerSession, VLWebServerSessionState> server(state,port, webDir);
		while (running) {
			server.service();
		}
	}

	return 0;
}


