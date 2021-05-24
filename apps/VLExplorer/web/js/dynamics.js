// Important global data for the campus/city environment.
var api = new VLApi();
var modelList = [];
let currentModel = null;
let currentParams = null;
let samples = [];
let pca = [];

// This is the function that is called once the document is started.
$( document ).ready(function() {
  api.getModels().then(function(models) {
    modelList = models;
    models.forEach(model => {
      $("#modelSelect").append($('<option value="' + model.index + '">' + model.name + '</option>'));
    });

    models[0].getParameters().then(function(params) {
      models[0].create(params).then(function(sample) { 
        //sample.update();
        //sample.nav.t = 10.0;
        updatePCA(sample);
      });
    });
  });
  

  
});

function updatePCA(sample) {
  if (!sample) {
    return;
  }

  sample.update().then(function() {  
    if (!sample) {
      return;
    }
    //sample.nav.t = Math.floor(sample.nav.t)+1;
    //console.log(sample.data);
    //000samples.push({id:sample.id, data:sample.data});
    //console.log(samples);

    samples.push({id:sample.id, data:sample.data});
      			//Read the data
            //d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
              //updateData(data, "Sepal_Length", "Petal_Length");
              updateData(scatterPlot, sample.data.pca, function(d) {return d.x;}, function(d){return d.y;}, function(d){return 0;});
              //updateData(scatterPlot, samples, function(d) {return d.data.x;}, function(d){return d.data.y;}, function(d){return 0;});
            
            //});
            
    updatePCA(sample);
  })
}

function updateSample(sample, isPCA) {
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

    samples.push({id:sample.id, data:sample.data});
    //console.log(samples);

      			//Read the data
            //d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
              //updateData(data, "Sepal_Length", "Petal_Length");
              //updateData(scatterPlot, samples, function(d) {return d.data[Object.keys(d.data)[0]];}, function(d){return d.data[Object.keys(d.data)[1]];}, function(d){return d.id;});
            
            //});
            
    updateSample(sample);
  })
}

function createSample() {
  if (currentParams) {
    createModelSample(currentParams);
  }

}

function createModelSample(params) {
  /*sample = null;
  if (s) {
    s.delete();
  }*/

  currentModel.create(params).then(function(s) {
    let sample = s;
    sample.nav.m = 0;
    sample.nav.t = 10;
    $("#nav").append(JSON.stringify(sample.nav));
    updateSample(sample);


  });
}

function changeModel() {
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
    currentParams = params;
    $("#params").html("");
    $("#nav").html("");
    $("#data").html("");
    //$("#params").append(JSON.stringify(params));
    for (var key in params) {
      if (!(typeof params[key] === 'object')) {
        $("#params").append('<div style="float:left;width:100px;">' + key + "</div>")
        let scale = function(val) {
          if (params.scale[key] == "log") {
            return Math.log(val);
          }
          else {
            return val;
          }
        };
        let val = scale(params[key]);
        let min = scale(params.min[key]);
        let max = scale(params.max[key]);
        val = 100.0*(val-min)/(max-min);
        $("#params").append('<input  type="range" min="1" max="100" value="' + val + '" class="slider" name="' + key + '"></input>')
        $("#params").append('<span id="val_' + key + '">' + params[key] +'</span>');  
      }
    }

    $('.slider').on('change', function(e) {
      let key = $(e.target).attr('name');
      let val = parseFloat($(e.target).val());
      let scale = function(val) {
        if (params.scale[key] == "log") {
          return Math.log(val);
        }
        else {
          return val;
        }
      };

      let inv_scale = function(val) {
        if (params.scale[key] == "log") {
          return Math.exp(val);
        }
        else {
          return val;
        }
      };
      val = inv_scale((1.0*val/100.0)*(scale(params.max[key]) - scale(params.min[key])) + scale(params.min[key]));
      params[key] = val;
      $("#val_" + key).html(params[key].toFixed(2));
      //createModel(params);
    });

    //createModel(params);
  });

}

