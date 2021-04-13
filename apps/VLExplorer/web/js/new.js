// Important global data for the campus/city environment.
var api = new VLApi();
var modelList = [];
let currentModel = null;
let sample = null;

// This is the function that is called once the document is started.
$( document ).ready(function() {
  api.getModels().then(function(models) {
    modelList = models;
    models.forEach(model => {
      $("#modelSelect").append($('<option value="' + model.index + '">' + model.name + '</option>'));
    });
    /*models[0].getParameters().then(function(params) {
      models[0].create(params).then(function(sample) { 
        sample.update();
        sample.nav.t = 10.0;
        sample.update();
      });
    });*/
  });
  
});

function updateSample() {
  if (!sample) {
    return;
  }

  sample.update().then(function() {  
    if (!sample) {
      return;
    }
    $("#nav").html("");
    $("#data").html("");
    $("#nav").append(JSON.stringify(sample.nav));
    $("#data").append(JSON.stringify(sample.data));
    sample.nav.t = Math.floor(sample.nav.t)+1;
    updateSample();
  })
}

function changeModel() {
  let s = sample;
  sample = null;
  if (s) {
    s.delete();
  }
  time = 0;
  let selection = $("#modelSelect").val();
  if (selection >= 0) {
    $("#sampleCreate").show();
    currentModel = modelList[selection];
  }
  else {
    currentModel == null;
    return;
  }

  currentModel.getParameters().then(function(params) {
    $("#params").html("");
    $("#nav").html("");
    $("#data").html("");
    $("#params").append(JSON.stringify(params));
    currentModel.create(params).then(function(s) {
      sample = s;
      sample.nav.m = 0;
      $("#nav").append(JSON.stringify(sample.nav));
      updateSample();
    });
  });

}
