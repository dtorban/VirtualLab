function VLModelSample(api, id, params, nav) {
    this.api = api;
    this.id = id;
    this.params = params;
    this.nav = nav;
    this.data = {};
}

VLModelSample.prototype.update = function() {
    let self = this;
    return this.api.sendCommand({command: "updateSample", sampleId: this.id, nav: this.nav}, function(data) {
        self.nav = data.nav;
        self.data = data.data;
        return data;
    });
};

VLModelSample.prototype.delete = function() {
    let self = this;
    return this.api.sendCommand({command: "deleteSample", sampleId: this.id}, function(data) {
        return null;
    });
};

function VLModel(api, name, index) {
    this.api = api;
    this.name = name;
    this.index = index;
    this.params = {};
}

VLModel.prototype.getParameters = function() {
    let self = this;
    return this.api.sendCommand({command: "getParameters", index: this.index}, function(data) {
        self.params = data.params;
        return self.params;
    });
};

VLModel.prototype.create = function(params) {
    let self = this;
    return this.api.sendCommand({command: "createSample", index: this.index, params: params}, function(data) {
        let sample = new VLModelSample(self.api, data.sampleId, self.params, data.nav);
        return sample;
    });
};

function VLApi() {
    var self = this;
    this.socket = new WebSocket("ws://" + location.hostname+(location.port ? ':'+location.port: ''), "web_server");
    this.models = [];
    this.callbacks = {};
    this.requestId = 0;

    this.modelsPromise = new Promise(function(resolve, reject) {
        self.callbacks[0] = function(data) { 
            resolve(self.models); 
            delete self.callbacks[0];
        }
    });
    this.requestId++;

    this.socket.onmessage =function (msg) {
        var data = JSON.parse(msg.data);
        if (data.command == "init") {
            for (var i = 0; i < data.models.length; i++) {
                self.models.push(new VLModel(self, data.models[i], i));
            }
        }
        self.callbacks[data.id](data);
    }

    let connect = new Promise(function(resolve, reject) {
        self.socket.onopen = function() {
            resolve(self.socket);
        };
        self.socket.onerror = function(err) {
            reject(err);
        }
    });

    connect.then(socket => {
        socket.send(JSON.stringify({id: 0, command: "init"}));
    });

}

VLApi.prototype.getModels = function() {
    return this.modelsPromise;
};

VLApi.prototype.sendCommand = function(cmd, calcVal) {
    let self = this;
    cmd.id = this.requestId;
    this.socket.send(JSON.stringify(cmd));
    let promise = new Promise(function(resolve, reject) {
        self.callbacks[self.requestId] = function(data) { 
            resolve(calcVal(data)); 
            delete self.callbacks[data.id];
        }
    });
    this.requestId++;
    return promise;
};



