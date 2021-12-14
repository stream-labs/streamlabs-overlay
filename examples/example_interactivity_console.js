const streamlabs_overlays = require('../build/Release/game_overlay.node')

console.log('Call start');
streamlabs_overlays.start();
console.log('status =' + streamlabs_overlays.getStatus() + ';');

function script_finished() {
	console.log('--------------- script_finished');
	console.log('');

	console.log('Call stop');
	streamlabs_overlays.stop();

	console.log('END');
}

function step_finish() {
	console.log('--------------- step_finish');
	console.log('');


	setTimeout(script_finished, 2000);
}

function step_3() {
	console.log('--------------- step_3');
	console.log('');
	streamlabs_overlays.switchInteractiveMode(false);

	setTimeout(step_finish, 5000);
}

function step_2() {
	console.log('--------------- step_2');
	console.log('');
	streamlabs_overlays.switchInteractiveMode(true);

	setTimeout(step_3, 5000);
}

function step_1() {
	console.log('--------------- step_1');
	console.log('');
	console.log('Call to Interactive');

	streamlabs_overlays.setMouseCallback((eventType, x, y, modifier) => {
		console.log('get MouseCallback: ' + eventType + ', ' + x + ', ' + y + ', ' + modifier);
		return 3;
	});

	streamlabs_overlays.setKeyboardCallback((eventType, keyCode) => {
		console.log('get KeyboardCallback: ' + eventType + ', ' + keyCode);
		if (keyCode == 38) {
			streamlabs_overlays.switchInteractiveMode(false);
		}

		return 1;
	});

	setTimeout(step_2, 5000);
}

setTimeout(step_1, 2000);

setTimeout(script_finished, 25000);
