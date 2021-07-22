function SamplingStrategy(params, name, sampleMethod = null) {
  var self = this;
  this.name = name;
  this.sampleMethod = sampleMethod;
  this.color = "#e66465";
  this.numSamples = 1;
  this.createNum = 0;
  this.hasColor = true;
  this.model = null;
  this.visible = true;

  var p = JSON.parse(JSON.stringify(params));
  self.params = p;
}

SamplingStrategy.prototype.sample = function() {
  var self = this;

  if (this.createNum <= 0) {
    this.createNum = this.numSamples;
  }

  return new Promise(function(resolve, reject){
    var p = JSON.parse(JSON.stringify(self.params));

    if (self.sampleMethod) {
      self.sampleMethod(p, function(name) {
        console.log("Resolved!" + name);
        if (!name) {
          name = self.name;
        }
        resolve({name:name, params:p});
      }, self);
    }
    else {
      console.log("Resolved!a");
      resolve({name:self.name, params:p});
    }
  });
  
}





