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
var lines = [];
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
      //console.log(data);
      if (data["command"] == "getNavigation") {
        console.log(data["nav"]);
        sampleNavigation = data["nav"];
        connected = true;
      }
      if (data["command"] == "updateNavigation") {
        if (lines.length == 0) {
          for (var i = 0; i < data.data.data.length; i++) {
            lines.push( { line: null, points: [], modules: [], mods: null} );            
            //lines[i].points.push( new THREE.Vector3( 0.0, 1.0, 0.0 ) );
            lines[i].modules.push( new THREE.Vector3( 0.0, 1.0, 0.0 ) );
            lines[i].modules.push( new THREE.Vector3( 0.0, 0.0, 0.0 ) );
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
        }

        updateLines();
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
    if (canUpdate) {
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
        const geometry = new THREE.BufferGeometry().setFromPoints( lines[i].modules );
        lines[i].mods = new THREE.Line( geometry, material );
        scene.add( lines[i].mods );

        lines[i].mods.position.copy(new THREE.Vector3(dist*Math.floor(i%3),dist*Math.floor(i/3),0));
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
