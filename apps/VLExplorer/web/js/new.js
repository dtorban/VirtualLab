// Important global data for the campus/city environment.
var api = new VLApi();

// This is the function that is called once the document is started.
$( document ).ready(function() {
  api.getModels().then(console.log);
  api.getModels().then(function(models) {
    models[0].getParameters().then(console.log);
    models[0].getParameters().then(console.log);
  });
  
});