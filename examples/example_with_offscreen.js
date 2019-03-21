const { app, BrowserWindow} = require("electron")
//const streamlabs_overlays = require('../index')
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
        
    win.webContents.on('paint', (event, dirty, image) => {
      //overlay.paintOverlay(overlayid, image);
//      console.log(dirty);
//      console.log(image.getBitmap().length);
//      console.log(image.getBitmap().byteLength);
      fs.writeFileSync('c:\\work\\temp\\ps'+counter+'.png', image.toPNG());
      counter++;
      //win.webContents.sendInputEvent({type:'keyDown', x:10, y:10, keyCode:'A'});
      //win.webContents.sendInputEvent({type:'contextMenu', x:10, y:10});
      //win.webContents.sendInputEvent({type:'char', x:10, y:10, keyCode:'A'});
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