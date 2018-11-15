const addon = require('../build/Release/smg_overlay');

console.log(addon.hello());

function function1() {
    // stuff you want to happen right away
    console.log('Welcome to My Console,');
}

function function2() {
    // all the stuff you want to happen after that pause
    console.log('Blah blah blah blah extra-blah');
}

// call the first chunk of code right away
function1();

// call the rest of the code and have it execute after 3 seconds
setTimeout(function2, 15000);