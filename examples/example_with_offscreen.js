const { app, BrowserWindow } = require("electron")
const streamlabs_overlays = require('../build/Debug/streamlabs_overlay.node')
const fs = require("fs")

//
//  # command to run this test 
//  yarn run electron examples\example_with_offscreen.js
//
let test_started = false;
let win1;
let overlayid1;
let win2;
let overlayid2;
let hwnd;

const frame_rate = 2;
const win_width = 600;
const win_height = 800;

function createWindow() {
  test_started = true;
  console.log('--------------- step_createWindow');
  console.log('');

  streamlabs_overlays.start();

  win1 = new BrowserWindow({
    show: false,
    width: win_width,
    height: win_height,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  //win.loadURL(`https://time.is/`);
  win1.loadURL(`https://codepen.io/jasonleewilson/pen/gPrxwX`);

  hwnd = win1.getNativeWindowHandle();
  console.log(hwnd);
  overlayid1 = streamlabs_overlays.addHWND(hwnd);
  streamlabs_overlays.setPosition(overlayid1, 900, 200, win_width, win_height);

  win1.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid1, image.getSize().width, image.getSize().height, image.getBitmap());
  })

  win1.webContents.setFrameRate(frame_rate);
  win1.webContents.invalidate();


  win2 = new BrowserWindow({
    show: false,
    width: win_width,
    height: win_height,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  win2.loadURL(`https://time.is/`);
  
  hwnd = win2.getNativeWindowHandle();
  console.log(hwnd);
  overlayid2 = streamlabs_overlays.addHWND(hwnd);
  streamlabs_overlays.setPosition(overlayid2, 100,100, win_width, win_height);

  win2.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid2, image.getSize().width, image.getSize().height, image.getBitmap());

  })

  win2.webContents.setFrameRate(frame_rate);
  win2.webContents.invalidate();


  streamlabs_overlays.show();
 
  win1.on("closed", () => { win1 = null });
  win2.on("closed", () => { win2 = null });

  setTimeout(step_1, 5000);
}

app.on("ready", createWindow);

app.on("window-all-closed", () => {
  app.quit();
})

app.on("activate", () => {
  if (!test_started) {
    createWindow();
  }
})

function step_1() {
  console.log('--------------- step_1');
  console.log('');

  console.log('Call get overlays ids');
  var overlays_ids = streamlabs_overlays.getIds();
  console.log(overlays_ids);

  console.log('Call set overlay transparency 20/255');
  streamlabs_overlays.setTransparency(overlays_ids[0], 30);
  streamlabs_overlays.setTransparency(overlays_ids[1], 230);
  
  //console.log(' ' + win1.getBounds() );
  win1.setSize( 250, 250 , false );
  streamlabs_overlays.setPosition(overlays_ids[0], 900, 50, 250, 250);

  setTimeout(step_2, 11000);
}

function step_2() {
  console.log('--------------- step_2');
  console.log('');

  console.log('Call get overlays ids');
  var overlays_ids = streamlabs_overlays.getIds();
  console.log(overlays_ids);

  for (let overlayid of overlays_ids) {
    console.log('Call set overlay transparency 20/255');
    streamlabs_overlays.setTransparency(overlayid, 180);
  }

  setTimeout(step_finish, 11000);
}

function step_finish() {
  streamlabs_overlays.stop();
  app.quit();
}