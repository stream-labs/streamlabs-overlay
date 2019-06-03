const { app, BrowserWindow } = require("electron")
const streamlabs_overlays = require('../build/Release/streamlabs_overlay.node')
const fs = require("fs")

//
//  # command to run this test 
//  yarn run electron examples\example_with_offscreen.js
//
let test_started = false;
let win;

function createWindow() {
  test_started = true;
  console.log('--------------- step_createWindow');
  console.log('');

  streamlabs_overlays.start();

  win = new BrowserWindow({
    show: false,
    width: 600,
    height: 800,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  //win.loadURL(`https://time.is/`);
  win.loadURL(`https://codepen.io/jasonleewilson/pen/gPrxwX`);


  let hwnd = win.getNativeWindowHandle();
  console.log(hwnd);
  let overlayid = streamlabs_overlays.addHWND(hwnd);

  streamlabs_overlays.show();

  win.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid, 600, 800, image.getBitmap());

  })

  win.webContents.setFrameRate(10);
  win.webContents.invalidate();

  streamlabs_overlays.show();

  win.on("closed", () => { win = null });

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

  for (let overlayid of overlays_ids) {
    console.log('Call set overlay transparency 20/255');
    streamlabs_overlays.setTransparency(overlayid, 20);
  }

  setTimeout(step_2, 5000);
}

function step_2() {
  console.log('--------------- step_2');
  console.log('');

  console.log('Call get overlays ids');
  var overlays_ids = streamlabs_overlays.getIds();
  console.log(overlays_ids);

  for (let overlayid of overlays_ids) {
    console.log('Call set overlay transparency 20/255');
    streamlabs_overlays.setTransparency(overlayid, 200);
  }

  setTimeout(step_finish, 5000);
}

function step_finish() {
  streamlabs_overlays.stop();
  app.quit();
}