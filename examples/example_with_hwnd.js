const { app, BrowserWindow} = require("electron")
const streamlabs_overlays = require('../index')

let win
streamlabs_overlays.start();

function createWindow() {
    win = new BrowserWindow({ width: 800, height: 600 })
  
    win.loadURL(`https://google.com`)
  
    let hwnd = win.getNativeWindowHandle();
    console.log(hwnd);
    streamlabs_overlays.addHWND(hwnd);

    win.on("closed", () => {
      win = null
      streamlabs_overlays.stop();
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