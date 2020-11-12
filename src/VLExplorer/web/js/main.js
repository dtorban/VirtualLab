// Important global data for the campus/city environment.
var socket = new WebSocket("ws://" + location.hostname+(location.port ? ':'+location.port: ''), "web_server");
var connected = false;

// More important related to models and animation.
var container = document.querySelector( '#scene-container' );
const mixers = [];
const clock = new THREE.Clock();
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
      console.log(data);
    }
  }
  catch(exception) {
    alert('<p>Error' + exception);
  }
});

// This function is triggered once the web socket is opened.
socket.onopen = function() {
  socket.send(JSON.stringify({command: "test"}));
  connected = true;
}

// This function defines the properties of the scene as well as starts the
// update loop.
function init() {
  const fov = 35; // fov = Field Of View
  const aspect = container.clientWidth / container.clientHeight;
  const near = 0.1;
  const far = 1000;

  camera = new THREE.PerspectiveCamera( fov, aspect, near, far );
  camera.position.set( -10, 10, 10 );
  controls = new THREE.OrbitControls( camera, container );

  scene = new THREE.Scene();
  scene.background = new THREE.Color( 'lightgrey' );

  // create a Standard material using the texture we just loaded as a color map
  material = new THREE.MeshStandardMaterial( {color: 0x85868f});

  geometry = new THREE.BoxBufferGeometry( 2, 2, 2 );
  mesh = new THREE.Mesh( geometry, material );

  geometry = new THREE.PlaneBufferGeometry( 100, 100, 100 );
  mesh = new THREE.Mesh( geometry, material );
  mesh.rotation.x = -3.14159/2.0;

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

// This function updates the scene's animation cycle.
function update() {

  // Send the update command to the socket.
  if (connected) {
    //socket.send(JSON.stringify({command: "update", delta: delta}));
    //socket.send(JSON.stringify({command: "update"}));
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
