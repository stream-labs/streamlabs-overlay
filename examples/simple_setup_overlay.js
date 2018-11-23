const streamlabs_overlays = require('../build/Release/streamlabs_overlay');

console.log('Call start');
streamlabs_overlays.start();

function script_finished() {	
	console.log('--------------- script_finished');
	console.log('');
	
	console.log('Call stop');
	streamlabs_overlays.stop();

    console.log('END');
}

function step_finish() 
{
	console.log('--------------- step_finish');
	console.log('');

	console.log('Call hide');
	streamlabs_overlays.hide_overlays();

	console.log('Call get count');
	console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));

	console.log('Call remove overlay');
	streamlabs_overlays.remove_overlay(overlays_ids[0]);
	
	setTimeout(script_finished, 2000);
}

function step_3() 
{
	console.log('--------------- step_3');
	console.log('');

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));
	
	setTimeout(step_finish, 5000);
}

function step_2() 
{
	console.log('--------------- step_2');
	console.log('');

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.get_overlays_ids();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.get_overlay_info(overlays_ids[0]));

	console.log('Call set overlay position');
	console.log(streamlabs_overlays.set_overlay_position(overlays_ids[0], 500, 500, 600, 200));

	console.log('Call set overlay url');
	console.log(streamlabs_overlays.set_overlay_url(overlays_ids[0], "http://bing.com/"));
		
	setTimeout(step_3, 5000);
}

function step_1() 
{
	console.log('--------------- step_1');
	console.log('');
	console.log('Call get count');
	console.log(streamlabs_overlays.get_overlays_count());

	console.log('Call add overlay');
	console.log(streamlabs_overlays.add_overlay("https://www.google.com/doodles/"));

	console.log('Call show');
	streamlabs_overlays.show_overlays();
	
	setTimeout(step_2, 5000);
}

setTimeout(step_1, 2000);

setTimeout(script_finished, 25000);