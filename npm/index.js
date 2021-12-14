const path = require('path');
const bindingPath = path.resolve(path.join(__dirname, './game_overlay.node'))
const binding = require(bindingPath); 

module.exports = binding;
