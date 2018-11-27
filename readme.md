## Overlay
`overlay` - it is a window what try to stay over any other windows( even fullscreen games ) and show content of some `source`. 

`source` can be other window like chat or cpu monitor. 

Also app can create `web view`, open url in it and use that web view as source.

## nodejs module 
### Build 
  There is gyp project file in repository what can be used to make node module. 
```
node-gyp configure 
node-gyp build
```
  Module file will be in 'build\Release\'

#### Requirements
- node
- node-gyp
- python 2.7
- msbuild (vs studio make tools )

### Module use examples
  Examples to show api usage for simple usecases. 
```
node example\simple_add_remove_overlays.js
node example\simple_setup_overlay.js
node example\simple_thread_stress_text.js
```
  Simple UI example in 'example\electron_ui'

### Module API
Each overlay has ID by which it can be adressed in api.

Thread what control overlays have to be started and stoped explicitly by module user
- `start()` 
- `stop()`

For now overlays can be shown and hidden all together
- `show_overlays()`
- `hide_overlays()`

To get basic info about overlays 
- `get_overlays_count()`
- `get_overlays_ids()` it return list of overlay ids. 
- `get_overlay_info(overlay_id)`

To create, setup and remove overlay
- `add_overlay(url)` return overlay id 
- `add_overlay_ex(url, x, y, width, height)` return overlay id 
- `set_overlay_position(overlay_id, x, y, width, height)`
- `set_overlay_url(overlay_id, url)`
- `set_overlay_transparency(overlay_id, transparency)` from 0 to 255 like in SetLayeredWindowAttributes 
- `call_overlay_reload(overlay_id)` send web view a command to reload current page
- `remove_overlay(overlay_id)`

## stand alone app 
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
