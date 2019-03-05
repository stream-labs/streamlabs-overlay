const streamlabsOverlay = require('./index');
const assert = require('assert');

assert(typeof streamlabsOverlay !== 'undefined');
assert(typeof streamlabsOverlay.start === 'function');
