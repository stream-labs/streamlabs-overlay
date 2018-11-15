## Build nodejs module 
```
node-gyp configure 
node-gyp build
```
### Run module 
`node example\simple_load_module.js`

## Build stand alone app 
```
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
