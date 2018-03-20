var connectionId;

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
	$("#console").val( $("#console").val() + dat + "\n" );
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
  $("#cmdLine").show();

  $("#Global").load("global.html");
  $("#Piezos").load("piezos.html");
  $("#Pots").load("pots.html", function() { setupPots(); } ); 

  AddLog('Connection opened with id: ' + connectionId + ', Bitrate: ' + connectionInfo.bitrate);
  
};

var onReceive = function(receiveInfo) {
	LedOn();
	data = new Uint8Array( receiveInfo.data );
	doMidiHandler( data );
};

var onReceiveError = function(errorInfo) {
  if (errorInfo.connectionId === connectionId) {
    AddLog(errorInfo.error);
	chrome.serial.flush(connectionId, function(result) {});
	chrome.serial.getInfo(connectionId, function (connectionInfo) {
		if ( connectionInfo.paused ) {
			AddLog("Unpaused");
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
	  AddLog('Connection with id: ' + connectionId + ' closed. Result='+result);
  });

  $("button#open").html("Open Port");
  $("#cmdLine").hide();

  $("#Global").load("noconn.html");
  $("#Piezos").load("noconn.html");
  $("#Pots").load("noconn.html");
  $("#led").css("background", "white");
  $("#console").val("");
  
};

$(document).ready(function() {

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
			//connection.connect(port, 115200);
			chrome.serial.connect(port, {bitrate: 115200}, ConnectComplete)
        } else {
			$('button#open').data("tag",1);
            doDisconnect();
        }

    });
	
	$('button#send').click(function() {
		var line1 = ($("#Textfield").val() + "                ").slice(0,16);
		chrome.serial.send(connectionId,line1, function( sendInfo ) {} );
    });
	
	$('button#clear').click(function() {
		$("#console").val("");
    });

});