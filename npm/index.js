const path = require('path');
//const bindingPath = binary.find(path.resolve(path.join(__dirname, './package.json')));
const bindingPath = path.resolve(path.join(__dirname, './streamlabs_overlay.node'))
const binding = require(bindingPath); 

//console.log( path.resolve(path.join(__dirname, './streamlabs_overlay.node')) );
//const binding = require('C:/work/repos/streamlabs-overlay/build-tmp-napi-v3/Release/streamlabs_overlay.node'); 

module.exports = binding;
