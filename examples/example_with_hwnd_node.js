const streamlabs_overlays = require('../index')

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

function step_finish()
{
	console.log('--------------- step_finish');
	console.log('');


	setTimeout(script_finished, 2000);
}

function step_3()
{
	console.log('--------------- step_3');
	console.log('');

	setTimeout(step_finish, 5000);
}

function step_2()
{
	console.log('--------------- step_2');
	console.log('');
	streamlabs_overlays.toInteractive();
	setTimeout(step_3, 5000);
}

function got_text( test)
{
	console.log('--------------- got_text -----------___________' + test);
}


function step_1()
{
    console.log('--------------- step_1');
	console.log('');
	console.log('Call to Interactive');
	streamlabs_overlays.initInteractive( (test_text1, test_text2, test_text3) => {
		console.log('CallInteractive: '+ test_text1 +', '+ test_text2+', '+ test_text3);
		if(test_text3 == 38)
		{
			streamlabs_overlays.toInteractive();
		}
		return 1112233;
	} );


	setTimeout(step_2, 5000);
}

setTimeout(step_1, 2000);

setTimeout(script_finished, 25000);
