## Overlay
`overlay` - it is a window what try to stay over any other windows( even fullscreen games ) and show content of some `source`. 

`source` - can be other window like chat or cpu monitor. 

Also app can create `web view`, open url in it and use that web view as source.

It can be build as nodejs module and as standalone app. 

## NodeJS Module 
### Build 
  There is gyp project file in repository what can be used to make node module. 

To setup env and make a build use command
```
yarn install
```
  Module file will be in 'build\Release\'

And to make build again after source code changed
```
node-gyp build
```
By default module works fine with electron 4. But to make one loadable in electron 2. Other steps have to be done 

```
nvm use 8.x.x
yarn install 
#remove build-tmp*
node-gyp configure 
node-gyp rebuild --target=2.0.16 --dist-url=https://atom.io/download/atom-shell
node scripts\pack.js 
```

### Pack 
Use command 'node script\pack.js' to make a package for slobs.

It uses version from package.json so update it to a new value.

#### Requirements
- node
- yarn
- node-gyp
- python 2.7
- msbuild (vs studio make tools )

### Module use examples
  Examples to show api usage for simple usecases. 
```
node examples\example_with_hwnd_node.js
node examples\simple_add_remove_overlays.js
node examples\simple_setup_overlay.js
node examples\simple_thread_stress_text.js
yarn electron examples\example_with_hwnd.js	  
```
  Simple UI example in 'examples\example_with_offscreen.js'

### Module API
Each overlay has ID by which it can be adressed in api.

Thread what control overlays have to be started and stoped explicitly by module user
- `start()` 
- `stop()`

For now overlays can be shown and hidden all together
- `show()`
- `hide()`

To get basic info about overlays 
- `getCount()`
- `getIds()` it return list of overlay ids. 
- `getInfo(overlay_id)`

To create, setup and remove overlay
- `add(url)` return overlay id 
- `addHWND(hwnd)` return overlay id 
- `addEx(url, x, y, width, height)` return overlay id 
- `setPosition(overlay_id, x, y, width, height)`
- `setUrl(overlay_id, url)`
- `setTransparency(overlay_id, transparency)` from 0 to 255 like in SetLayeredWindowAttributes 
- `reload(overlay_id)` send web view a command to reload current page
- `remove(overlay_id)`
- `paintOverlay(overlay_id, width, height, bitmap)` 

For interective mode set callbacks and switch on/off. See examples\example_with_hwnd_node.js. 
- `setMouseCallback(callback)` 
- `setKeyabordCallback(callback)`
- `switchInteractiveMode()`

## Stand Alone App 

Code for that mode can be outdated as main development have focus on nodejs module mode. 

### Build 
```
mkdir build_app
cd build_app
cmake -G "Visual Studio 15 2017 Win64" ..
cmake --build . --target ALL_BUILD --config Debug -- /p:CharacterSet=Unicode
```

### Run
```
cd build_app\Debug
streamlabs_overlay_app.exe
```

### Use 
  There is 'help.html' with information about app usage. 
