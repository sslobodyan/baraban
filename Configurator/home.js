
var homeSetup = function() {
	$('button#clearConsole').click(function() {
		$("#console").val("");
    });

	$('button#saveConfig').click(function() {
		saveConfig();
    });

	$('button#calibrate').click(function() {
		Calibrate();
    });

	$('.slider').each(	function() {
		$(this).slider();
		$(this).on("slideStop", function(slideEvt) {
			console.log(slideEvt.value);
		});
	});

}

var saveConfig = function() {
	var comnd =  [0xF0, 0x7D, 0x7F, 0x13, 0x01, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var Calibrate = function() {
	// ������������ ������ �� 2 ������� ���� ���� ����������
	$('button#calibrate').addClass("disabled").attr("disabled",true);
	setTimeout( enableCalibrate , 2000);
	var comnd =  [0xF0, 0x7D, 0x7F, 0x14, 0x7F, 0xF7] ;
    bb = new Uint8Array(comnd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var enableCalibrate = function() {
	$('button#calibrate').removeClass("disabled").attr("disabled",false);
}

