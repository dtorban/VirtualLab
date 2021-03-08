// Important global data for the campus/city environment.
var socket = new WebSocket("ws://" + location.hostname+(location.port ? ':'+location.port: ''), "web_server");
var connected = false;

// More important related to models and animation.
var container = document.querySelector( '#scene-container' );
var container2 = document.querySelector( '#scene-container2' );
const mixers = [];
const clock = new THREE.Clock();
var query = null;
var sampleNavigation = {};
var scene;
var scene2;
var line = null;
var line2 = null;
var lines = [];
var controls;
var camera;
var camera2d;
var renderer;
var canUpdate = true;
var dataLines = [];
var running = true;

// Function definitions start here...
  // Adapted from http://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
  
  var randomColor = (function(){
    var golden_ratio_conjugate = 0.618033988749895;
    var h = Math.random();
  
    var hslToRgb = function (h, s, l){
        var r, g, b;
  
        if(s == 0){
            r = g = b = l; // achromatic
        }else{
            function hue2rgb(p, q, t){
                if(t < 0) t += 1;
                if(t > 1) t -= 1;
                if(t < 1/6) return p + (q - p) * 6 * t;
                if(t < 1/2) return q;
                if(t < 2/3) return p + (q - p) * (2/3 - t) * 6;
                return p;
            }
  
            var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
            var p = 2 * l - q;
            r = hue2rgb(p, q, h + 1/3);
            g = hue2rgb(p, q, h);
            b = hue2rgb(p, q, h - 1/3);
        }
  
        return '#'+Math.round(r * 255).toString(16)+Math.round(g * 255).toString(16)+Math.round(b * 255).toString(16);
    };
    
    return function(){
      h += golden_ratio_conjugate;
      h %= 1;
      return hslToRgb(h, 0.5, 0.60);
    };
  })();

var colors = [];
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
colors.push(randomColor())
console.log(colors);

// set the dimensions and margins of the graph
var margin = {top: 10, right: 30, bottom: 30, left: 60},
    width = 400 - margin.left - margin.right,
    height = 400 - margin.top - margin.bottom;

var radius = 3.0;

// append the svg object to the body of the page
var svg1 = d3.select("#my_dataviz")
  .append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
  .append("g")
    .attr("transform",
          "translate(" + margin.left + "," + margin.top + ")");

var svg2 = d3.select("#my_dataviz2")
.append("svg")
  .attr("width", width + margin.left + margin.right)
  .attr("height", height + margin.top + margin.bottom)
.append("g")
  .attr("transform",
        "translate(" + margin.left + "," + margin.top + ")");


function drawPlot(svg, data) {
  // Add X axis
  var x = d3.scaleLinear()
    .domain([-4, 6])
    .range([ 0, width ]);
  svg.append("g")
    .attr("transform", "translate(0," + height + ")")
    .call(d3.axisBottom(x));

  // Add Y axis
  var y = d3.scaleLinear()
    .domain([-4, 5])
    .range([ height, 0]);
  svg.append("g")
    .call(d3.axisLeft(y));

  // Add dots
  svg.append('g')
    .selectAll("dot")
    .data(data)
    .enter()
    .append("circle")
      .attr("cx", function (d) { return x(d.x); } )
      .attr("cy", function (d) { return y(d.y); } )
      .attr("r", radius)
      .style("fill", "#69b3a2");

    //x.domain([d3.min(data, function(d) { return d.x; }), d3.max(data, function(d) {return d.x; })]);
    //y.domain([d3.min(data, function(d) { return d.y; }), d3.max(data, function(d) {return d.y; })]);

      // Add brushing
  svg
  .call( d3.brush()                 // Add the brush feature using the d3.brush function
    .extent( [ [0,0], [width,height] ] ) // initialise the brush area: start at 0,0 and finishes at width,height: it means I select the whole graph area
    .on("start brush end", updateChart) // Each time the brush selection changes, trigger the 'updateChart' function
  )
}

function updateChart() {
  var x = d3.scaleLinear()
    .domain([-4, 6])
    .range([ 0, width ]);
  var y = d3.scaleLinear()
    .domain([-4, 5])
    .range([ height, 0]);
    
  if (d3.event.type == "start") {
    //console.log("the start");
    var extent = d3.event.selection;
    d3.select(this).selectAll("circle").classed("prospected", function(d){ return isBrushed(extent, x(d.x), y(d.y) ) } );
    d3.select(this).selectAll(".prospected").classed("selected", function(d){ return false; } );
  }
  else if (d3.event.type == "end") {
    //console.log("the end");
    console.log(d3.event, d3.select(this).selectAll(".prospected"));
    d3.select(this).selectAll(".prospected").classed("selected", function(d){ return true; } );
    d3.select(this).selectAll("circle").classed("prospected", function(d){ return false; } );
  }
  else {
    var extent = d3.event.selection;
    d3.select(this).selectAll("circle:not(.selected)").classed("prospected", function(d){ return isBrushed(extent, x(d.x), y(d.y) ) } );
    d3.select(this).selectAll(".selected").classed("selected", function(d){ return !isBrushed(extent, x(d.x), y(d.y) ) } );
  }
}

/*function updateChart() {
  if (d3.event.type == "end") {
    d3.select(this).call( d3.brush()                 // Add the brush feature using the d3.brush function
        .extent( [ [0,0], [width,height] ] ) // initialise the brush area: start at 0,0 and finishes at width,height: it means I select the whole graph area
        .on("start brush end", updateChart) // Each time the brush selection changes, trigger the 'updateChart' function
      )
  }
  else {
    var x = d3.scaleLinear()
      .domain([-4, 6])
      .range([ 0, width ]);
    var y = d3.scaleLinear()
      .domain([-4, 5])
      .range([ height, 0]);
  
    var extent = d3.event.selection;
    d3.select(this).selectAll("circle").classed("selected", function(d){ return isBrushed(extent, x(d.x), y(d.y) ) } );
  }
}*/


// A function that return TRUE or FALSE according if a dot is in the selection or not
function isBrushed(brush_coords, cx, cy) {
    var x0 = brush_coords[0][0],
        x1 = brush_coords[1][0],
        y0 = brush_coords[0][1],
        y1 = brush_coords[1][1];
   return x0 <= cx && cx <= x1 && y0 <= cy && cy <= y1;    // This return TRUE or FALSE depending on if the points is in the selected area
}

function updatePlot(svg, data) {
    var x = d3.scaleLinear()
      .domain([-4, 6])
      .range([ 0, width ]);

    // Add Y axis
    var y = d3.scaleLinear()
      .domain([-4, 5])
      .range([ height, 0]);
    //x.domain([d3.min(data, function(d) { return d.x; }), d3.max(data, function(d) {return d.x; })]);
    //y.domain([d3.min(data, function(d) { return d.y; }), d3.max(data, function(d) {return d.y; })]);

    /*var x = d3.scaleLinear()
        .domain([d3.min(data, function(d) { return Math.floor(d.x); }), d3.max(data, function(d) {return Math.ceil(d.x); })])
        .range([ 0, width ]);

    // Add Y axis
    var y = d3.scaleLinear()
        .domain([d3.min(data, function(d) { return Math.floor(d.y); }), d3.max(data, function(d) {return Math.ceil(d.y); })])
        .range([ height, 0]);*/

    svg.selectAll("circle")
        .data(data)
        .transition()
        .duration(0)
        //.append("circle")
        .attr("cx", function (d) { return x(d.x); } )
        .attr("cy", function (d) { return y(d.y); } )
        .attr("r", radius)
        .style("fill", function (d) { return colors[d.id % colors.length]; });
        //.style("fill", "#69b3a2");

    svg.selectAll("circle")
        .data(data)
        .enter()
        .append("circle")
        .attr("cx", function (d) { return x(d.x); } )
        .attr("cy", function (d) { return y(d.y); } )
        .attr("r", radius)
        .style("fill", function (d) { return colors[d.id % colors.length]; });
        //.style("fill", "#69b3a2");

    /*// Update X Axis
    svg.select(".x.axis")
        .transition()
        .duration(0)
        .call(x);

    // Update Y Axis
    svg.select(".y.axis")
        .transition()
        .duration(0)
        .call(y);*/
}


//Read the data
//d3.csv("https://raw.githubusercontent.com/holtzy/data_to_viz/master/Example_dataset/2_TwoNum.csv", function(data) { drawPlot(data); })

var data = [{x:3.14, y: 1}, {x:3.14, y: 2}];
drawPlot(svg1, data);
drawPlot(svg2, data);

// This is the function that is called once the document is started.
$( document ).ready(function() {
  // Init() starts up the scene and its update loop.
  init();

  // Start checking for when the user resizes their application window.
  window.addEventListener( 'resize', onWindowResize );

  // Listen for when the system wants to create new scene objects.
  try {
    socket.onmessage =function got_packet(msg) {
      var data = JSON.parse(msg.data);
      //console.log(data);
      if (data["command"] == "getNavigation") {
        //console.log(data["nav"]);
        sampleNavigation = data["nav"];
        connected = true;
      }
      if (data["command"] == "updateNavigation") {
        if (lines.length == 0) {
          //console.log(data["data"]);
          for (var i = 0; i < data.data.data.length; i++) {
            lines.push( { line: null, points: [], modules: [], mods: null} );            
            //lines[i].points.push( new THREE.Vector3( 0.0, 1.0, 0.0 ) );
            lines[i].modules.push( new THREE.Vector3( 0.0, 1.0, 0.0 ) );
            lines[i].modules.push( new THREE.Vector3( 0.0, 0.0, 0.0 ) );

            dataLines.push( { line: null, points: [], modules: [], mods: null} );  
            dataLines.push( { line: null, points: [], modules: [], mods: null} );  
            dataLines.push( { line: null, points: [], modules: [], mods: null} );  
            dataLines.push( { line: null, points: [], modules: [], mods: null} );  
            //dataLines.push( { line: null, points: [], modules: [], mods: null} );  
          }
        }
        //console.log(data["data"]);

        //lines.push( new THREE.Vector3( data.data.data[0]["x"]/1000.0, data.data.data[0]["y"]/1000.0, 0.0 ) );
        for (var i = 0; i < data.data.data.length; i++) {
          lines[i].points.push( new THREE.Vector3( data.data.data[i]["x"]/1000.0, data.data.data[i]["y"]/1000.0, 0.0 ) );
          lines[i].modules = [];

          for (var j = 0; j < data.data.data[i].m.length; j++) {
            //lines[i].modules.push( new THREE.Vector3( 0.0, 0.0, 0.0 ) );
            
            lines[i].modules.push( new THREE.Vector3( data.data.data[i]["x"]/1000.0, data.data.data[i]["y"]/1000.0, 0.0 ) );
            lines[i].modules.push(new THREE.Vector3( data.data.data[i].m[j]["x"]/1000.0, data.data.data[i].m[j]["y"]/1000.0, 0.0 ));

          }
          var d = data.data.data[i];

          var dist = 80;
          var scale = 3.0;

          dataLines[i*4+0].points.push(new THREE.Vector3( time/1.0, scale*d["actin"]/3000.0 + dist*-1.0-dist, 0.0 ));
          dataLines[i*4+1].points.push(new THREE.Vector3( time/1.0, scale*d["aflow"]/5.0 + dist*0.0-dist, 0.0 ));
          var f = Math.sqrt(d["fx"]*d["fx"] + d["fy"]*d["fy"]);
          dataLines[i*4+2].points.push(new THREE.Vector3( time/1.0, scale*f/5.0 + dist*1.0-dist, 0.0 ));
          var s = Math.sqrt(d["x"]*d["x"] + d["y"]*d["y"]);
          dataLines[i*4+3].points.push(new THREE.Vector3( time/1.0, scale*s/500.0 + dist*2.0-dist, 0.0 ));
          //dataLines[i*data.data.data.length+3].points.push(new THREE.Vector3( time/10.0, d["fy"]/5.0 + 50*3.0-50.0, 0.0 ));
          //console.log(dataLines);
        }

        updateLines();
        updatePlot(svg1, data.data.pca);
        updatePlot(svg2, data.data.pca2);
        canUpdate = true;
      }
    }
  }
  catch(exception) {
    alert('<p>Error' + exception);
  }
});

// This function is triggered once the web socket is opened.
socket.onopen = function() {
  //socket.send(JSON.stringify({command: "init"}));
  socket.send(JSON.stringify({command: "getNavigation"})); //updateNavigation
}

function init2d() {
  var w = $("#scene-container2").width();
  var h = $("#scene-container2").height();
  var viewSize = w;
  var aspectRatio = w / h;
  
  var viewport = {
      viewSize: viewSize,
      aspectRatio: aspectRatio,
      left: (-aspectRatio * viewSize) / 2,
      right: (aspectRatio * viewSize) / 2,
      top: viewSize / 2,
      bottom: -viewSize / 2,
      near: -100,
      far: 100
  }
  
  camera2d = new THREE.OrthographicCamera ( 
    viewport.left, 
    viewport.right, 
    viewport.top, 
    viewport.bottom, 
    viewport.near, 
    viewport.far 
  );
  //camera2d.position.set( 0, 0, 0.01 );

  scene2 = new THREE.Scene();
  //scene2.background = new THREE.Color( 'red' );

  scene2.add( camera2d );

  /*var p = [
    new THREE.Vector3(0,0,0),
    new THREE.Vector3(0,700,10),
    new THREE.Vector3(10,10,0),
    new THREE.Vector3(0,10,70),
    new THREE.Vector3(0,10,-20)
  ];

  var material = new THREE.LineBasicMaterial( { color: 0xffffff } );
  const geometry = new THREE.BufferGeometry().setFromPoints( p );
  var line = new THREE.Line( geometry, material );
  scene2.add( line );*/
  
  renderer2 = new THREE.WebGLRenderer( { antialias: true } );
  renderer2.setSize( $("#scene-container2").width(), $("#scene-container2").height() );
  renderer2.setClearColor( 0x000000, 0 );
  container2.appendChild( renderer2.domElement );
}

// This function defines the properties of the scene as well as starts the
// update loop.
function init() {
  init2d();

  const fov = 35; // fov = Field Of View
  const aspect = container.clientWidth / container.clientHeight;
  const near = 0.1;
  const far = 1000;

  camera = new THREE.PerspectiveCamera( fov, aspect, near, far );
  camera.position.set( 0, 0, 10 );
  controls = new THREE.OrbitControls( camera, container );

  scene = new THREE.Scene();
  scene.background = new THREE.Color( 'white' );
  //scene.background = new THREE.Color( 'lightgrey' );

  // create a Standard material using the texture we just loaded as a color map
  material = new THREE.MeshStandardMaterial( {color: 0x85868f});

  geometry = new THREE.BoxBufferGeometry( 2, 2, 2 );
  mesh = new THREE.Mesh( geometry, material );

  geometry = new THREE.PlaneBufferGeometry( 5, 5, 5 );
  mesh = new THREE.Mesh( geometry, material );

  const ambientLight = new THREE.AmbientLight( 0xffffff, 1 );
  scene.add( ambientLight );
  const light = new THREE.DirectionalLight( 0xffffff, 1 );
  light.position.set( 10, 10, 10 );
  scene.add( ambientLight, light );
  const light2 = new THREE.DirectionalLight( 0xffffff, 1 );
  light2.position.set( 0, 10, -10 );
  scene.add( ambientLight, light2 );

  renderer = new THREE.WebGLRenderer( { antialias: true } );
  renderer.setSize( window.innerWidth, window.innerHeight );
  document.body.appendChild( renderer.domElement );

  // start the animation/render loop
  renderer.setAnimationLoop( () => {
    update();
    render();
  });
}

var time = 0.0;

function updateNavigation() {
  if (connected) {
    if (canUpdate && running) {
      canUpdate = false;
      time += 1.0;//10.0*60.0;
      //const delta = clock.getDelta();
      //time += delta;
      sampleNavigation.t = time;
      socket.send(JSON.stringify({command: "updateNavigation", nav: sampleNavigation}));
    }
  }
}

function updateLines() {
    const dist = 30;

    for (var i = 0; i < lines.length; i++) {
      if (lines[i].line) {
        scene.remove(lines[i].line);
      }

      {
        //create a blue LineBasicMaterial
        //var material = new THREE.LineBasicMaterial( { color: 0xff751a } );
        var material = new THREE.LineBasicMaterial( { color: 0xff751a } );
        const geometry = new THREE.BufferGeometry().setFromPoints( lines[i].points );
        lines[i].line = new THREE.Line( geometry, material );
        scene.add( lines[i].line );

        lines[i].line.position.copy(new THREE.Vector3(dist*Math.floor(i%3),dist*Math.floor(i/3),0));
        
      }
    /*if (lines.length > 0) {
      //line.position.copy(new THREE.Vector3(-lines[lines.length-1].x,0,0));
    }*/

    /*if (line2) {
      scene.remove(line2);
    }
      //create a blue LineBasicMaterial
      const geometry2 = new THREE.BufferGeometry().setFromlines( lines2 );
      line2 = new THREE.Line( geometry2, material );
      scene.add( line2 );
  
      if (lines.length > 0) {
        line2.position.copy(new THREE.Vector3(-lines2[lines2.length-1].x,2.0,0));
      }*/
      

      if (lines[i].mods) {
        scene.remove(lines[i].mods);
      }

      {
        //create a blue LineBasicMaterial
        var material = new THREE.LineBasicMaterial( { color: 0x0000ff } );
        material.color.setHex(parseInt(colors[i%10].replace("#","0x"), 16));
        const geometry = new THREE.BufferGeometry().setFromPoints( lines[i].modules );
        lines[i].mods = new THREE.Line( geometry, material );
        scene.add( lines[i].mods );

	      lines[i].mods.position.copy(new THREE.Vector3(dist*Math.floor(i%3),dist*Math.floor(i/3),0));
      }
    }

    for (var i = 0; i < dataLines.length; i++) {
      if (dataLines[i].line) {
        scene2.remove(dataLines[i].line);
      }

      {
        //create a blue LineBasicMaterial
        var material = new THREE.LineBasicMaterial( { color: 0xffffff } );
        const geometry = new THREE.BufferGeometry().setFromPoints( dataLines[i].points );
        dataLines[i].line = new THREE.Line( geometry, material );
        scene2.add( dataLines[i].line );

        dataLines[i].line.position.copy(new THREE.Vector3(-dataLines[i].points[dataLines[i].points.length-1].x + 45,0,0));
      }
    }
}

// This function updates the scene's animation cycle.
function update() {

  // Send the update command to the socket.
  if (connected) {
    //socket.send(JSON.stringify({command: "update", delta: delta}));
    //socket.send(JSON.stringify({command: "update"}));
    updateNavigation();
  }
}

// This function simply renders the scene based on the camera position.
function render() {
  renderer.render( scene, camera );
  renderer2.render( scene2, camera2d );
}

// This function updates the projection matrix and renderer whenever the
// user's application window is resized.
function onWindowResize() {
  // set the aspect ratio to match the new browser window aspect ratio
  camera.aspect = container.clientWidth / container.clientHeight;

  // update the camera's frustum
  camera.updateProjectionMatrix();

  // update the size of the renderer AND the canvas
  renderer.setSize( container.clientWidth, container.clientHeight );
}
