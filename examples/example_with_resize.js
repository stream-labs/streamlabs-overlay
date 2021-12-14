const { app, BrowserWindow } = require("electron")
const electron = require('electron')
const streamlabs_overlays = require('../build/Debug/game_overlay.node')
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
let win3;
let overlayid3;
let hwnd;

const frame_rate = 2;
const win_width = 700;
const win_height = 700;

function createWindow() {
  test_started = true;
  console.log('--------------- step_createWindow');
  console.log('');

  streamlabs_overlays.start();

  win1 = new BrowserWindow({
    show: false,
    width: win_width/2,
    height: win_height,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  win1.loadURL(`https://codepen.io/jasonleewilson/pen/gPrxwX`);
  
  win1.setBounds({width: win_width/2, height: win_height, x: 100, y: 100 });
  hwnd = win1.getNativeWindowHandle();
  console.log(hwnd);

  overlayid1 = streamlabs_overlays.addHWND(hwnd);
  let win1_rect = win1.getBounds();
  streamlabs_overlays.setPosition(overlayid1, win1_rect.x, win1_rect.y, Number(win1_rect.width), Number(win1_rect.height));

  win1.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid1, image.getSize().width, image.getSize().height, image.getBitmap());
  })

  win1.webContents.setFrameRate(frame_rate);
  win1.webContents.invalidate();

  win2 = new BrowserWindow({
    show: false,
    width: win_width,
    height: win_height/2,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  win2.loadURL(`https://time.is/`);
  
  win2.setBounds({width: win_width, height: win_height/2, x: 100+win_width/2, y: 100 });
  hwnd = win2.getNativeWindowHandle();
  console.log(hwnd);
  overlayid2 = streamlabs_overlays.addHWND(hwnd);
  
  let win2_rect = win2.getBounds();
  streamlabs_overlays.setPosition(overlayid2, win2_rect.x, win2_rect.y, Number(win2_rect.width), Number(win2_rect.height));
  
  win2.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid2, image.getSize().width, image.getSize().height, image.getBitmap());

  })

  win2.webContents.setFrameRate(frame_rate);
  win2.webContents.invalidate();

  win3 = new BrowserWindow({
    show: false,
    width: win_width,
    height: win_height/2,
    frame: false,
    webPreferences: {
      offscreen: true,
      nodeIntegration: false,
    },
  })

  win3.loadURL(`https://time.is/`);
  win3.setBounds({width: win_width, height: win_height/2, x: 100+win_width/2, y: 100 +win_height/2});
  hwnd = win3.getNativeWindowHandle();
  console.log(hwnd);
  overlayid3 = streamlabs_overlays.addHWND(hwnd);
  
  let win3_rect = win3.getBounds();
  streamlabs_overlays.setPosition(overlayid3, win3_rect.x, win3_rect.y, Number(win3_rect.width), Number(win3_rect.height));
  
  win3.webContents.on('paint', (event, dirty, image) => {
    streamlabs_overlays.paintOverlay(overlayid3, image.getSize().width, image.getSize().height, image.getBitmap());
  })

  win3.webContents.setFrameRate(frame_rate);
  win3.webContents.invalidate();

  streamlabs_overlays.show();
 
  win1.on("closed", () => { win1 = null });
  win2.on("closed", () => { win2 = null });
  win3.on("closed", () => { win3 = null });

  setTimeout(step_1, 10000);
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

  console.log('' + streamlabs_overlays.getInfo(overlays_ids[0]) );

  console.log('Call set overlay transparency 20/255');
  streamlabs_overlays.setTransparency(overlays_ids[0], 150);
  streamlabs_overlays.setTransparency(overlays_ids[1], 150);
  streamlabs_overlays.setTransparency(overlays_ids[1], 150);

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
  
  win1.setBounds({width: 400, height: 200, x: 100, y: 100 });
  let win1_rect = win1.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[0], win1_rect.x, win1_rect.y, Number(win1_rect.width), Number(win1_rect.height));

  win2.setBounds({width: 150, height: 400, x: 100, y: 300 });
  let win2_rect = win2.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[1], win2_rect.x, win2_rect.y, Number(win2_rect.width), Number(win2_rect.height));

  win3.setBounds({width: 200, height: 500, x: 250, y: 300 });  
  let win3_rect = win3.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[2], win3_rect.x, win3_rect.y, Number(win3_rect.width), Number(win3_rect.height));

  setTimeout(step_3, 11000);
}

function step_3() {
  console.log('--------------- step_2');
  console.log('');

  console.log('Call get overlays ids');
  var overlays_ids = streamlabs_overlays.getIds();
  console.log(overlays_ids);

  for (let overlayid of overlays_ids) {
    console.log('Call set overlay transparency 20/255');
    streamlabs_overlays.setTransparency(overlayid, 180);
  }

  win1.setBounds({width: 600, height: 300, x: 100, y: 100 });
  let win1_rect = win1.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[0], win1_rect.x, win1_rect.y, Number(win1_rect.width), Number(win1_rect.height));

  win2.setBounds({width: 300, height: 600, x: 100, y: 400 });
  let win2_rect = win2.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[1], win2_rect.x, win2_rect.y, Number(win2_rect.width), Number(win2_rect.height));

  win3.setBounds({width: 300, height: 600, x: 400, y: 400 });  
  let win3_rect = win3.getBounds();
  streamlabs_overlays.setPosition(overlays_ids[2], win3_rect.x, win3_rect.y, Number(win3_rect.width), Number(win3_rect.height));

  setTimeout(step_finish, 11000);
}

function step_finish() {
  streamlabs_overlays.stop();
  app.quit();
}