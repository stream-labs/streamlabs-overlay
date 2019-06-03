## Overlay
`overlay` - it is a window what try to stay over any other windows( even fullscreen games ) and show content of some `source`. 

`source` - can be other window like chat or cpu monitor. 

It should be build as nodejs module. 

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

To make debug build for nodejs tests 

```
node-gyp --debug configure rebuild 
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
- `addHWND(hwnd)` return overlay id 
- `setPosition(overlay_id, x, y, width, height)`
- `setTransparency(overlay_id, transparency)` from 0 to 255 like in SetLayeredWindowAttributes 
- `reload(overlay_id)` send web view a command to reload current page
- `remove(overlay_id)`
- `paintOverlay(overlay_id, width, height, bitmap)` 

For interective mode set callbacks and switch on/off. See examples\example_with_hwnd_node.js. 
- `setMouseCallback(callback)` 
- `setKeyabordCallback(callback)`
- `switchInteractiveMode()`
