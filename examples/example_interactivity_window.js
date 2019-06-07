
const { app, BrowserWindow } = require("electron")
const streamlabs_overlays = require('../build/Release/streamlabs_overlay.node')
const fs = require("fs")

let win;
//
//  # command to run this test 
//  yarn run electron examples\example_interactivity_window.js
//
let test_started = false;

function createWindow() {
	test_started = true;
	console.log('--------------- step_createWindow');
	console.log('');

	streamlabs_overlays.start();

	win = new BrowserWindow({
		show: false,
		width: 700,
		height: 700,
		frame: false,
		webPreferences: {
			offscreen: true,
			nodeIntegration: false,
		},
	})

	win.loadURL(`https://www.google.com/search?q=1`);
	//win.loadURL(`https://unixpapa.com/js/testkey.html`);  //to test input 

	let hwnd = win.getNativeWindowHandle();
	console.log(hwnd);
	let overlayid = streamlabs_overlays.addHWND(hwnd);

	streamlabs_overlays.show();
	streamlabs_overlays.setTransparency(overlayid, 100);

	win.webContents.on('paint', (event, dirty, image) => {
		streamlabs_overlays.paintOverlay(overlayid, image.getSize().width, image.getSize().height, image.getBitmap());

	})

	win.webContents.setFrameRate(10);
	win.webContents.invalidate();
	win.webContents.once('dom-ready', () => {
		win.focus();
		//win.webContents.sendInputEvent({ type: 'char', keyCode: '\u0022' });
		//win.webContents.sendInputEvent({ type: "keyDown", keyCode: '\u0022' });
		//win.webContents.sendInputEvent({ type: "keyUp", keyCode: '\u0022' });
		//win.webContents.invalidate();
	});

	streamlabs_overlays.setMouseCallback((eventType, x, y, modifier) => {
		console.log('get first MouseCallback: ' + eventType + ', ' + x + ', ' + y + ', ' + modifier);
		return 1;
	});	

	streamlabs_overlays.setKeyboardCallback((eventType, keyCodeValue) => {
		//console.log('get KeyboardCallback: '+ eventType +', '+ keyCodeValue + ', '+ BrowserWindow.getAllWindows[0].getNativeWindowHandle());
		console.log('get KeyboardCallback: ' + eventType + ', ' + keyCodeValue);
		//console.log('get KeyboardCallback: app ' + app.getPath("test") );

		//BrowserWindow.getAllWindows()[0].webContents.sendInputEvent({type:eventType, x:10, y:10, keyCode:keyCodeValue});
		try {
			win.webContents.sendInputEvent({ type: eventType, keyCode: keyCodeValue });
			//win.webContents.invalidate();
			//win.loadURL(`https://www.google.com/search?q=`+keyCodeValue);
			//let hwnd = win.getNativeWindowHandle();
			//console.log(hwnd);
		} catch (error) {
			console.log(error);
		}

		if (keyCodeValue == 38) {
			streamlabs_overlays.switchInteractiveMode(false);
		} else {

			// win.focusOnWebView();
			// win.webContents.sendInputEvent({type:eventType, keyCode:keyCodeValue});
			// //win.loadURL(`https://www.google.com/search?q=2`)
			// win.webContents.invalidate();
		}
		// win.focus();
		//win.webContents.sendInputEvent({type:'keyDown', x:10, y:10, keyCode:'A'});
		//win.webContents.sendInputEvent({type:'contextMenu', x:10, y:10});
		//win.webContents.sendInputEvent({type:'char', x:10, y:10, keyCode:'A'});		
		return 2;
	});


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

function step_2() {
	console.log('--------------- step_2');
	console.log('');

	streamlabs_overlays.switchInteractiveMode(false);
	setTimeout(step_finish, 5000);
}

function step_finish() {
	streamlabs_overlays.stop();
	app.quit();
}

function step_1() {
	console.log('--------------- step_1');
	console.log('');
	console.log('Call to Interactive');

	streamlabs_overlays.switchInteractiveMode(true);

	setTimeout(step_2, 5000);
}
