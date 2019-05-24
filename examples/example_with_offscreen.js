const { app, BrowserWindow} = require("electron")
const streamlabs_overlays = require('../build/Release/streamlabs_overlay.node')

streamlabs_overlays.start();

app.disableHardwareAcceleration();

const fs = require("fs")
let win
let counter = 1;
function createWindow() {

    win = new BrowserWindow({ 
        show: false,
        width: 200, 
        height: 300,      
        frame: false,
        webPreferences: {
            offscreen :true,
            nodeIntegration: false,
        },
     })
    win.focus();
    //win.webContents.loadURL(`https://google.com`)
    win.loadURL(`https://google.com`)
    win.webContents.once('dom-ready', () => {
      win.webContents.sendInputEvent({type:'char', x:10, y:10, keyCode:'A'});
    });
      
    let hwnd = win.getNativeWindowHandle();
    console.log(hwnd);
    let overlayid = streamlabs_overlays.addHWND(hwnd);
    streamlabs_overlays.show();
	
	win.webContents.on('paint', (event, dirty, image) => {
      streamlabs_overlays.paintOverlay(overlayid,200, 300, image.getBitmap());
//      console.log(dirty);
//      console.log(image.getBitmap().length);
//      console.log(image.getBitmap().byteLength);
      //fs.writeFileSync('c:\\work\\temp\\ps'+counter+'.png', image.toPNG());
      //counter++;
      //win.webContents.sendInputEvent({type:'keyDown', x:10, y:10, keyCode:'A'});
      //win.webContents.sendInputEvent({type:'contextMenu', x:10, y:10});
      //win.webContents.sendInputEvent({type:'char', x:10, y:10, keyCode:'A'});
    })          
    
    win.webContents.setFrameRate(10);
    win.webContents.invalidate();

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