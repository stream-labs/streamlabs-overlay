const { app, BrowserWindow} = require("electron")
//const streamlabs_overlays = require('../index')
const fs = require("fs")
let win

function createWindow() {

    win = new BrowserWindow({ 
        show: true,
        width: 200, 
        height: 300,      
        frame: false,
        webPreferences: {
            offscreen :true,
            nodeIntegration: false,
        },
     })
    
    //win.webContents.loadURL(`https://google.com`)
    win.loadURL(`https://google.com`)
  
    let hwnd = win.getNativeWindowHandle();
    console.log(hwnd);
        
    win.webContents.on('paint', (event, dirty, image) => {
      //overlay.paintOverlay(overlayid, image);
//      console.log(dirty);
//      console.log(image.getBitmap().length);
//      console.log(image.getBitmap().byteLength);
      //fs.writeFileSync('c:\\work\\temp\\ps1.png', image.toPNG());
    })          
    
    win.webContents.setFrameRate(1);

    win.on("closed", () => {
      win = null    
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