// Important global data for the campus/city environment.
var socket = new WebSocket("ws://" + location.hostname+(location.port ? ':'+location.port: ''), "web_server");
var connected = false;

// More important related to models and animation.
var container = document.querySelector( '#scene-container' );
const mixers = [];
const clock = new THREE.Clock();
var query = null;
var sampleNavigation = {};
var scene;
var line = null;
var line2 = null;
var points = [];
var points2 = [];
var controls;
var camera;
var canUpdate = true;

// Function definitions start here...

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
      if (data.command == "updateSample") {
        connected = true;

        sampleNavigation = data.sample.navigation;

        //$("#notification-bar").empty();
        if (!query) {
          if (data.query) {
            query = data.query;
          }

          $("#notification-bar").append("<p>Query</p>")
          for (var key in query) {
            //$("#notification-bar").append(key + ": <input type='text' value='" + query[key] + "'><br>");
            $("#notification-bar").append(key + ': <input type="range" min="1" max="100" value="' + (query[key]/(3.14159*2.0))*100 + '" class="slider" name="' + key + '"></input>' + query[key] +'<br>');
          }
          $('.slider').on('input', function(e) {
            //console.log($(e.target).val(), $(e.target).attr('name'));
            query[$(e.target).attr('name')] = (2.0 * 3.14159 * $(e.target).val() / 100.0)
            updateQuery();
          });
          $("#notification-bar").append("<p>Navigation</p>")
          for (var key in sampleNavigation) {
            $("#notification-bar").append(key + ": <input type='text' value='" + sampleNavigation[key] + "'><br>");
          }
          $("#notification-bar").append("<p>Data</p>");
          $("#notification-bar").append("<div id='data-container'></div>");
        }

        $("#data-container").empty();
        for (var key in data.sample.data) {
          $("#data-container").append(key + ": " + data.sample.data[key] + "<br>");
        }
        for (var key in data.sample.navigation) {
          $("#data-container").append(key + ": " + data.sample.navigation[key] + "<br>");
        }

        points.push( new THREE.Vector3( data.sample.navigation["time"], data.sample.data["y"], 0.0 ) );
        //points.push( new THREE.Vector3( data.sample.data["x"]/1000.0, data.sample.data["y"]/1000.0, 0.0 ) );
        //points2.push( new THREE.Vector3( sampleNavigation["time"], data.sample.data["nm"], 0.0 ) );
        updateLines();

        canUpdate = true;
        
      }
      //console.log(data);
    }
  }
  catch(exception) {
    alert('<p>Error' + exception);
  }
});

// This function is triggered once the web socket is opened.
socket.onopen = function() {
  socket.send(JSON.stringify({command: "createSample"}));
}

function updateQueryValue(key, val) {

}

// This function defines the properties of the scene as well as starts the
// update loop.
function init() {
  const fov = 35; // fov = Field Of View
  const aspect = container.clientWidth / container.clientHeight;
  const near = 0.1;
  const far = 1000;

  camera = new THREE.PerspectiveCamera( fov, aspect, near, far );
  camera.position.set( 0, 0, 10 );
  controls = new THREE.OrbitControls( camera, container );

  scene = new THREE.Scene();
  scene.background = new THREE.Color( 'lightgrey' );

  // create a Standard material using the texture we just loaded as a color map
  material = new THREE.MeshStandardMaterial( {color: 0x85868f});

  geometry = new THREE.BoxBufferGeometry( 2, 2, 2 );
  mesh = new THREE.Mesh( geometry, material );

  geometry = new THREE.PlaneBufferGeometry( 5, 5, 5 );
  mesh = new THREE.Mesh( geometry, material );
  //mesh.rotation.x = -3.14159/2.0;

  const ambientLight = new THREE.AmbientLight( 0xffffff, 1 );
  scene.add( ambientLight );
  const light = new THREE.DirectionalLight( 0xffffff, 1 );
  light.position.set( 10, 10, 10 );
  scene.add( ambientLight, light );
  const light2 = new THREE.DirectionalLight( 0xffffff, 1 );
  light2.position.set( 0, 10, -10 );
  scene.add( ambientLight, light2 );

  //scene.add( mesh);

  renderer = new THREE.WebGLRenderer( { antialias: true } );
  renderer.setSize( window.innerWidth, window.innerHeight );
  document.body.appendChild( renderer.domElement );

  // start the animation/render loop
  renderer.setAnimationLoop( () => {
    update();
    render();
  });
}

// This function kills the webpage's socket connection.
function kill() {
  if (connected) {
    socket.send(JSON.stringify({command: "kill"}));
  }
}

// This function kills the webpage's socket connection.
function updateQuery() {
  if (connected) {
    if (canUpdate) {
      canUpdate = false;
      socket.send(JSON.stringify({command: "updateQuery", query: query, navigation: sampleNavigation}));
    }
  }
}

var time = 0.0;

// This function kills the webpage's socket connection.
function updateNavigation() {
  if (connected) {
    if (canUpdate) {
      canUpdate = false;
      const delta = clock.getDelta();
      time += delta;
      //time += 0.1;
      sampleNavigation.time = time;
      socket.send(JSON.stringify({command: "updateNavigation", navigation: sampleNavigation}));
    }
  }
}

function updateLines() {
  if (line) {
    scene.remove(line);
  }
    //create a blue LineBasicMaterial
    var material = new THREE.LineBasicMaterial( { color: 0x0000ff } );
    const geometry = new THREE.BufferGeometry().setFromPoints( points );
    line = new THREE.Line( geometry, material );
    scene.add( line );

    if (points.length > 0) {
      line.position.copy(new THREE.Vector3(-points[points.length-1].x,0,0));
    }

    /*if (line2) {
      scene.remove(line2);
    }
      //create a blue LineBasicMaterial
      const geometry2 = new THREE.BufferGeometry().setFromPoints( points2 );
      line2 = new THREE.Line( geometry2, material );
      scene.add( line2 );
  
      if (points.length > 0) {
        line2.position.copy(new THREE.Vector3(-points2[points2.length-1].x,2.0,0));
      }*/
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
