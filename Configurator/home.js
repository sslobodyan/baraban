
var homeSetup = function() {
	$('button#clearConsole').click(function() {
		$("#console").val("");
    });

	$('button#enableConsole').click(function() {
		activateConsole();
    });

	$('button#saveConfig').click(function() {
		saveConfig();
    });

	$('button#loadConfig').click(function() {
		loadConfig();
    });

	$('button#calibrate').click(function() {
		Calibrate();
    });

	$('.slider').each(	function() {
		$(this).slider();
		$(this).on("slideStop", function(slideEvt) {
			//console.log(slideEvt.value );
			var cc = $(slideEvt.target).data("cc");
			var val = slideEvt.value;
			$(slideEvt.target).next(".ccInput").val(val);
			sendCC(cc, val);
		});
	});

	$('.ccInput').change( function() {
		var cc=$(this).data("cc");
		//console.log('change '+cc);
        sendCC(cc, $(this).val() );
	});
	

	console.log('homeSetup');
}

var sendCC = function(cc, val) {
	//console.log("send "+val+" to "+cc);
	if (val > 127) val = 127;
	var comnd =  [0xB9, cc, val ] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var saveConfig = function() {
	var comnd =  [0xF0, 0x7D, getSelectedModule(), 0x13, 0x01, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var loadConfig = function() {
	var comnd =  [0xF0, 0x7D, getSelectedModule(), 0x15, 0x7F, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
	setTimeout(getModuleVersion, 500);
}

var getModuleVersion = function() {
	var comnd =  [0xF0, 0x7D, getSelectedModule(), 0x13, 0x02, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var Calibrate = function() {
	// деактивируем кнопку на 2 секунды пока идет калибровка
	$('button#calibrate').addClass("disabled").attr("disabled",true);
	$("#console").val('');
	setTimeout( enableCalibrate , 2000);
	var comnd =  [0xF0, 0x7D, getSelectedModule(), 0x14, 0x7F, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var enableCalibrate = function() {
	$('button#calibrate').removeClass("disabled").attr("disabled",false);
    var t=$("#console");
	t.val( t.val() + "Be free - calibrate was ended :)\n" );
}

var activateConsole = function() {
	var en = $("#enableConsole");
	if (en.data("enable") == "enable") {
		en.data("enable","disable");
		en.text("Enable");
		enableConsoleUpdate = false;
	} else {
		en.data("enable","enable");
		en.text("Disable");
		enableConsoleUpdate = true;
	}
}
