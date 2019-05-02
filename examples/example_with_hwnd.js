const { app, BrowserWindow} = require("electron")
//const streamlabs_overlays = require('../build-tmp-napi-v1/Release/streamlabs_overlay.node')
const streamlabs_overlays = require('../build/Release/streamlabs_overlay.node')

//
//  # command to run this test 
//  yarn run electron examples\example_with_hwnd.js
//

let win
//streamlabs_overlays.start();
//app.disableHardwareAcceleration();

function createWindow() {

    win = new BrowserWindow({ width: 800, height: 600 })
    
    win.loadURL(`https://google.com`)
  
    let hwnd = win.getNativeWindowHandle();
    console.log(hwnd);
    streamlabs_overlays.addHWND(hwnd);

    win.on("closed", () => {
      win = null
      //streamlabs_overlays.stop();
    })
  }
  
  app.on("ready", createWindow)
  
  app.on("window-all-closed", () => {
    if (process.platform !== "darwin") {
      app.quit()
    }
  })
  
  app.on("activate", () => {
    if (win === null) {
      createWindow()
    }
  })