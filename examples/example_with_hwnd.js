const { app, BrowserWindow } = require("electron")
const streamlabs_overlays = require('../build/Release/streamlabs_overlay.node')

//
//  # command to run this test 
//  yarn run electron examples\example_with_hwnd.js
//
let test_started = false;

app.disableHardwareAcceleration();

function createWindow() {
  test_started = true;
  console.log('--------------- step_createWindow');
  console.log('');
const w_width = 500;
const w_heihgt = 500;

  let win1;
  let win2;
  let win3;
  let win4;
  let win5;
  let win6;
  let hwnd;
  streamlabs_overlays.start();

  win1 = new BrowserWindow({ x: 100+w_width*0, y: 100+w_heihgt*0, width: w_width, height: w_heihgt })
  win1.loadURL(`https://www.google.com/search?q=1`);
  hwnd = win1.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  win2 = new BrowserWindow({ x: 100+w_width*1, y: 100+w_heihgt*0, width: w_width, height: w_heihgt })
  win2.loadURL(`https://www.google.com/search?q=2`);
  hwnd = win2.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  win3 = new BrowserWindow({ x: 100+w_width*2, y: 100+w_heihgt*0, width: w_width, height: w_heihgt })
  win3.loadURL(`https://www.google.com/search?q=3`);
  hwnd = win3.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  win4 = new BrowserWindow({ x: 100+w_width*0, y: 100+w_heihgt*1, width: w_width, height: w_heihgt })
  win4.loadURL(`https://www.google.com/search?q=a`);
  hwnd = win4.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  win5 = new BrowserWindow({ x: 100+w_width*1, y: 100+w_heihgt*1, width: w_width, height: w_heihgt })
  win5.loadURL(`https://www.google.com/search?q=b`);
  hwnd = win5.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  win6 = new BrowserWindow({ x: 100+w_width*2, y: 100+w_heihgt*1, width: w_width, height: w_heihgt })
  win6.loadURL(`https://www.google.com/search?q=c`);
  hwnd = win6.getNativeWindowHandle();
  streamlabs_overlays.addHWND(hwnd);

  streamlabs_overlays.show();

  win1.on("closed", () => { win1 = null });
  win2.on("closed", () => { win2 = null });
  win3.on("closed", () => { win3 = null });

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

  let count = 2;
  for (let overlayid of overlays_ids) {
    console.log('Call get overlay info ' + overlayid);
    console.log(streamlabs_overlays.getInfo(overlayid));

    console.log('Call set overlay transparency 20/255');
    streamlabs_overlays.setTransparency(overlayid, 20 + count * 235 / 2);
    count = count - 1;
  }

  setTimeout(step_2, 5000);
}

function step_2() {
  console.log('--------------- step_2');
  console.log('');

  console.log('Call get overlays ids');
  var overlays_ids = streamlabs_overlays.getIds();
  console.log(overlays_ids);

  console.log('Call remove overlay ' + overlays_ids[0]);
  streamlabs_overlays.remove(overlays_ids[0]);

  setTimeout(step_finish, 5000);
}

function step_finish() {
  streamlabs_overlays.stop();
  app.quit();
}