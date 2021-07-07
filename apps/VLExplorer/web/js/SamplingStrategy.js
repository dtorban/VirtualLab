function SamplingStrategy(params, name, sampleMethod = null) {
  var self = this;
  this.name = name;
  this.sampleMethod = sampleMethod;

  var p = JSON.parse(JSON.stringify(params));
  self.params = p;
}

SamplingStrategy.prototype.sample = function() {
  var self = this;
  return new Promise(function(resolve, reject){
    var p = JSON.parse(JSON.stringify(self.params));

    if (self.sampleMethod) {
      self.sampleMethod(p, function(name) {
        console.log("Resolved!" + name);
        if (!name) {
          name = self.name;
        }
        resolve({name:name, params:p});
      });
    }
    else {
      console.log("Resolved!a");
      resolve({name:self.name, params:p});
    }
  });
  
}





