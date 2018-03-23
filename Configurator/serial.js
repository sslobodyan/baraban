var connectionId;
const waitTime = 20;
var enableConsoleUpdate=false;


var LedOff = function() {
	$("#led").css("background", "grey");	
}

var LedOn = function() {
	if ( $("#led").css("background") != "red") {
		$("#led").css("background", "red");
		setTimeout(LedOff, 100);
	}
}

var AddLog = function(dat) {
	if (enableConsoleUpdate == true) $("#console").val( $("#console").val() + dat + "\n" );
}


var ConnectComplete = function(connectionInfo) {
  if (!connectionInfo) {
    console.log("Connection failed.");
    return;
  }

  connectionId = connectionInfo.connectionId;

  chrome.serial.flush( connectionId, function(){} );
  chrome.serial.onReceive.addListener(onReceive);
  chrome.serial.onReceiveError.addListener(onReceiveError);

  $("button#open").html("Close Port");

	if ( $("#Piezos").html() == '' ) {
		console.log('Need load');
		$("#Piezos").load("piezos.html", function() { setupPiez(); } );
	} else { console.log('Yet loaded '); }
	if ( $("#Pots").html() == '' ) $("#Pots").load("pots.html", function() { setupPots(); } ); 
	if ( $("#home").html() == '' ) $("#home").load("home.html", function() { homeSetup(); }); 

  activateTab("home");

  $('.conn').show();

};

var onReceive = function(receiveInfo) {
	LedOn();
	data = new Uint8Array( receiveInfo.data );
	doMidiHandler( data );
};

var onReceiveError = function(errorInfo) {
  if (errorInfo.connectionId === connectionId) {
    //AddLog(errorInfo.error);
	chrome.serial.flush(connectionId, function(result) {});
	chrome.serial.getInfo(connectionId, function (connectionInfo) {
		if ( connectionInfo.paused ) {
			//AddLog("Unpaused");
			chrome.serial.setPaused(connectionId, false, function (){} );
		}		
	} );
  }
};

var doDisconnect = function() {
  if ( !connectionId ) {
    throw 'Invalid connection';
  }
  chrome.serial.disconnect(connectionId, function(result) {
	  //AddLog('Connection with id: ' + connectionId + ' closed. Result='+result);
  });

  $("button#open").html("Open Port");

  $("#led").css("background", "white");
  $("#console").val("");

	$('#Piezos').html('');  
	$('#Pots').html('');
	$('#home').html('');

  $('.conn').hide();
};

var logSendCommand =  function( dat ){
	var s=" -> Send ";
	for (var i=0; i<dat.length; i++) s += dat[i] + ",";
	AddLog( s );	
}


var CloseAllConnections = function() {
	// закрыть все предыдущие соединения - может быть зависшая сессия
	chrome.serial.getConnections(function(info){
		cc = info;
	    var cnt = cc.length;
		if (cnt>0) {
			for( var i=0; i<cnt; i++) {
				var id = cc[i].connectionId;		
				chrome.serial.disconnect(id, function(result) {
					console.log("Closed connection "+id);
			  	});
			}
		}

	});
}

var activateTab = function(tab){
  $('.nav-tabs a[href="#' + tab + '"]').tab('show');
};

$(document).ready(function() {

	CloseAllConnections();

    chrome.serial.getDevices(function(devices) {
		if ( devices.length < 1 ) {
			$("button#open").html("No active ports");
		} else {
	        for (var i = 0; i < devices.length; i++) {
    	        $('select#portList').append('<option value="' + devices[i].path + '">' + devices[i].path + '</option>');
	        }
		}
    });
	
    $('button#open').click(function() {
        if ($(this).data("tag")==1) {
			$('button#open').data("tag",0);
            var port = $('select#portList').val();
			chrome.serial.connect(port, { bitrate: 115200 }, ConnectComplete)
        } else {
			$('button#open').data("tag",1);
            doDisconnect();
        }
    });
		
});