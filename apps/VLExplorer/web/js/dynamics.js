// Important global data for the campus/city environment.
var api = new VLApi();
var modelList = [];
let currentModel = null;
let currentParams = null;
let pca = [];

// This is the function that is called once the document is started.
$( document ).ready(function() {
  api.getModels().then(function(models) {
    modelList = models;
    models.forEach(model => {
      $("#modelSelect").append($('<option value="' + model.index + '">' + model.name + '</option>'));
    });

    models[0].getParameters().then(function(params) {
      params.data = 1;
      params.clusters = 9;
      models[0].create(params).then(function(sample) { 
        sample.nav.keys = {x:0, y:0};
        //sample.update();
        //sample.nav.t = 10.0;
        updatePCA(sample, scatterPlot);
      });
    });

    /*models[0].getParameters().then(function(params) {
      params.params = 1;
      params.clusters = 0;
      models[0].create(params).then(function(sample) { 
        //sample.update();
        //sample.nav.t = 10.0;
        updatePCA(sample, scatterPlot2);
      });
    });*/

      
    models[0].getParameters().then(function(params) {
      params.params = 1;
      params.clusters = 0;
      models[0].create(params).then(function(sample) { 
        //sample.update();
        //sample.nav.t = 10.0;
        updatePCA(sample, scatterPlot3);
      });
    });
  });
  
  
});

function updatePCA(sample, plot) {
  if (!sample) {
    return;
  }

  sample.update().then(function() {  
    if (!sample) {
      return;
    }

    if (sample.nav.keys) {
      if (!sample.keys || Object.entries(sample.keys).length != Object.entries(sample.nav.keys).length) {
        sample.keys = sample.nav.keys;

        $(plot.containerId+" .pca-config").html("");

        d3.select(plot.containerId+" .pca-config")
          .selectAll("div")
          .data(Object.entries(sample.nav.keys))
          .enter()
          .append("div")
          .append("div")
          .text((d) => {return d[0];})
          .append("input")
          .attr("type", "checkbox")
          .text((d) => {
            return d[0];
          })
          .property("checked", (d) => {
              return d[1] > 0;
          })
          .on("change", (d,e) => {
            sample.keys[d[0]] = d3.event.target.checked ? 1 : 0;
            plot.reset();
          });
      }

      sample.nav.keys = sample.keys;
      sample.nav.zoom = plot.zoomTransform;
      //console.log(plot.SVG.node());
      //plot.SVG.node().append('<div class="pca-config">'+JSON.stringify(sample.nav)+'</div>');
      
      //$(plot.containerId+" .pca-config").html(JSON.stringify(sample.nav));

      //console.log(Object.entries(sample.nav.keys));

      
        
        /*.attr("cy", function (d) { return self.zoomY(b(d)); } )
        //.attr("cy", function (d) { return y(d["Petal_Length"]); } )
        .attr("r", 3)
        //.style("fill", "#61a3a9")
        .style("fill", function (d) { return self.colors[color(d)%self.colors.length];})
        .style("opacity", 1.0)*/
    }
    //console.log(sample.data);
    //000samples.push({id:sample.id, data:sample.data});
    //console.log(samples);
    $("#nav").append(JSON.stringify(sample.nav));

      			//Read the data
            //d3.csv("https://raw.githubusercontent.com/holtzy/D3-graph-gallery/master/DATA/iris.csv", function(data) {
              //updateData(data, "Sepal_Length", "Petal_Length");
              plot.updateData(sample.data.bounds, sample.data.pca, function(d) {return d.x;}, function(d){return d.y;}, function(d){return d.cluster;});
              
              //updateData(scatterPlot, samples, function(d) {return d.data.x;}, function(d){return d.data.y;}, function(d){return 0;});
            
            //});

    // Add VDI
    var vdi = d3.select("#spatial").selectAll(".vdi")
        .data(sample.data.vdi)

    var div = vdi.enter()
        .append("div")
        .attr("class", "vdi" )
        //.text((d) => {return d.id;})

        //.text((d) => {return d.id;})
    
    if (sample.data.vdi.length > 0) {

      var containerWidth = +d3.select('.vdi').style('width').slice(0, -2);
      var containerHeight = +d3.select('.vdi').style('height').slice(0, -2);
  
      var svg = div
        .append("svg")
        .attr("width", containerWidth)
        .attr("height", containerHeight)
        //.style("background-color", "green")
        .append("g");
        //.attr("transform",
                //"translate(" + this.margin.left + "," + this.margin.top + ")");

      /*svg.append("circle")
        .attr('cx', function(d) {console.log("test", d.data.x); return d.data.x/10.0 + 100.0;})
        .attr('cy', function(d) {return d.data.y/10.0 + 100.0;})
        .attr("r", "40")
        .attr("stroke", "black")
        .attr("stroke-width", "3")
        .attr("fill", "red")*/
      //<circle cx="50" cy="50" r="40" stroke="black" stroke-width="3" fill="red" />

      d3.select("#spatial").selectAll("g").each(function(a, i) {
        var data = sample.data.vdi[i].data;
        if (data.m) {
          var circles = data.m;
          //circles.push({x:data.x, y:data.y});
          /*circles.push({x:0, y:0});
          d3.select(this).selectAll("circle")
            .data(circles)
            .exit()
            .remove()
          d3.select(this).selectAll("circle")
            .data(circles)
            .enter()
            .append("circle")
            .attr('cx', function(d) {return (d.x-data.x)/100.0 + 100.0;})
            .attr('cy', function(d) {return (d.y-data.y)/100.0 + 100.0;})
            .attr("r", "5")
            .attr("stroke", "black")
            .attr("stroke-width", "5")
            .attr("fill", "red")
          d3.select(this).selectAll("circle")
            .attr('cx', function(d) {return (d.x-data.x)/100.0 + 100.0;})
            .attr('cy', function(d) {return (d.y-data.y)/100.0 + 100.0;})*/

          var arms = data.m;
          d3.select(this).selectAll("line")
            .data(arms)
            .exit()
            .remove()
          d3.select(this).selectAll("line")
            .data(arms)
            .enter()
            .append("line")
            .attr('x1', function(d) {return 100.0;})
            .attr('y1', function(d) {return 100.0;})
            .attr('x2', function(d) {return (d.x-data.x)/100.0 + 100.0;})
            .attr('y2', function(d) {return (d.y-data.y)/100.0 + 100.0;})
            .attr("stroke", "orange")
            .attr("stroke-width", "10")

          d3.select(this).selectAll("line")
            .attr('x2', function(d) {return (d.x-data.x)/100.0 + 100.0;})
            .attr('y2', function(d) {return (d.y-data.y)/100.0 + 100.0;})
        }

        /*<line x1="10" x2="50" y1="110" y2="150" stroke="orange" stroke-width="5"/>*/
        /*<rect x="60" y="10" rx="10" ry="10" width="30" height="30" stroke="black" fill="transparent" stroke-width="5"/>*/

      })
        /*.attr('cx', function(d) {console.log("test2", d.data.x); return d.data.x/10.0 + 100.0;})
        .attr('cy', function(d) {return d.data.y/10.0 + 100.0;})*/
        //.style("fill", function (d) { return self.colors[color(d)%self.colors.length];});*/
    }
            
    updatePCA(sample, plot);
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
    
    /*if (sample.nav.keys) {
      for (const [key, value] of Object.entries(sample.nav.keys)) {
        //console.log(`${key}: ${value}`);
        var checked = value > 0 ? 'checked' : '';
        $("#nav").append('<input type="checkbox" id="nav-'+key+'" name="'+key+'" value="1" '+checked+'><label for="nav-'+key+'">'+key+'</label><br>')
      }
    }
    $("#nav").append(JSON.stringify(sample.nav));*/

    $("#data").append(JSON.stringify(sample.data));
    sample.nav.t = Math.floor(sample.nav.t)+10;

    //samples.push({id:sample.id, data:sample.data});
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
    $("#runs").append($('<div class="run">' + JSON.stringify(currentParams) + '</div>'));
  }

}

function createModelSample(params) {
  /*sample = null;
  if (s) {
    s.delete();
  }*/

  currentModel.create(params).then(function(s) {
    let sample = s;
    //sample.nav.m = 0;
    sample.nav.t = 10;
    $("#nav").append(JSON.stringify(sample.nav));
    updateSample(sample);


  });
}

function getMetaData(params, param, key, defaultVal) {
  let metadata = params[".metadata"];
  if (metadata && metadata[param]) {
    metadata = metadata[param];
    return metadata[key];
  }
  else {
    return defaultVal;
  }
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
          if (getMetaData(params, key, "scale", "linear") == "log") {
            return Math.log(val);
          }
          else {
            return val;
          }
        };
        let val = scale(params[key]);
        let min = scale(getMetaData(params, key, "min", val));
        let max = scale(getMetaData(params, key, "max", val));
        val = 100.0*(val-min)/(max-min);
        $("#params").append('<input  type="range" min="1" max="100" value="' + val + '" class="slider" name="' + key + '"></input>')
        $("#params").append('<span id="val_' + key + '">' + params[key] +'</span>');  
      }
    }

    $('.slider').on('change', function(e) {
      let key = $(e.target).attr('name');
      let val = parseFloat($(e.target).val());
      let scale = function(val) {
        if (getMetaData(params, key, "scale", "linear") == "log") {
          return Math.log(val);
        }
        else {
          return val;
        }
      };

      let inv_scale = function(val) {
        if (getMetaData(params, key, "scale", "linear") == "log") {
          return Math.exp(val);
        }
        else {
          return val;
        }
      };
      
      let min = getMetaData(params, key, "min", val);
      let max = getMetaData(params, key, "max", val);
      val = inv_scale((1.0*val/100.0)*(scale(max) - scale(min)) + scale(min));
      params[key] = val;
      $("#val_" + key).html(params[key].toFixed(2));
      //createModel(params);
    });

    //createModel(params);
  });

}

