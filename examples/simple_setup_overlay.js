const streamlabs_overlays = require('../index');

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
	streamlabs_overlays.hide();

	console.log('Call get count');
	console.log(streamlabs_overlays.getCount());

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.getIds();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.getInfo(overlays_ids[0]));

	console.log('Call remove overlay');
	streamlabs_overlays.remove(overlays_ids[0]);

	setTimeout(script_finished, 2000);
}

function step_4()
{
	console.log('--------------- step_4');
	console.log('');

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.getIds();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	streamlabs_overlays.getInfo(overlays_ids[0]);

	setTimeout(step_finish, 5000);
}

function step_3()
{
	console.log('--------------- step_3');
	console.log('');

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.getIds();
	console.log(overlays_ids);

	console.log('Call set overlay transparency 20/255');
	streamlabs_overlays.setTransparency(overlays_ids[0], 20);

	console.log('Call reload overlay page');
	streamlabs_overlays.reload(overlays_ids[0]);

	setTimeout(step_4, 5000);
}

function step_2()
{
	console.log('--------------- step_2');
	console.log('');

	console.log('Call get overlays ids');
	var overlays_ids = streamlabs_overlays.getIds();
	console.log(overlays_ids);

	console.log('Call get overlay info ' + overlays_ids[0]);
	console.log(streamlabs_overlays.getInfo(overlays_ids[0]));

	console.log('Call set overlay position');
	streamlabs_overlays.setPosition(overlays_ids[0], 500, 500, 600, 200);

	console.log('Call set overlay url');
	streamlabs_overlays.setUrl(overlays_ids[0], "http://bing.com/");

	console.log('Call set overlay transparency off');
	streamlabs_overlays.setTransparency(overlays_ids[0], 255);

	setTimeout(step_3, 5000);
}

function step_1()
{
	console.log('--------------- step_1');
	console.log('');
	console.log('Call get count');
	console.log(streamlabs_overlays.getCount());

	console.log('Call add overlay');
	console.log(streamlabs_overlays.add("https://www.google.com/doodles/"));

	console.log('Call show');
	streamlabs_overlays.show();

	setTimeout(step_2, 5000);
}

setTimeout(step_1, 2000);

setTimeout(script_finished, 25000);
