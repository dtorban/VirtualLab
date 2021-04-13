// Important global data for the campus/city environment.
var api = new VLApi();

// This is the function that is called once the document is started.
$( document ).ready(function() {
  api.getModels().then(function(models) {
    models[0].getParameters().then(function(params) {
      models[0].create(params).then(function(sample) { 
        sample.update();
        sample.nav.t = 1.0;
        sample.update();
      });
    });
    ;
  });
  
});