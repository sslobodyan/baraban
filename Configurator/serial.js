var connectionId;
const waitTime = 20;


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

  $("#Global").load("global.html");
  $("#Piezos").load("piezos.html", function() { setupPiez(); } );
  $("#Pots").load("pots.html", function() { setupPots(); } ); 

  //$("#cmdLine").show();

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
  //$("#cmdLine").hide();

  $("#Global").load("noconn.html");
  $("#Piezos").load("noconn.html");
  $("#Pots").load("noconn.html");
  $("#led").css("background", "white");
  $("#console").val("");
  
};

var logSendCommand =  function( dat ){
	var s=" -> Send ";
	for (var i=0; i<dat.length; i++) s += dat[i] + ",";
	AddLog( s );	
}


var CloseAllConnections = function() {
	// закрыть все предыдущие соединения - может быть зависшая сессия
	//console.log( "CloseAllConnections" );
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


$(document).ready(function() {

	CloseAllConnections();

    $("#home").load("home.html", function() {  
		homeSetup();
		console.log("home loaded");
	}); 

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
			chrome.serial.connect(port, { bitrate: 115200 }, ConnectComplete)
        } else {
			$('button#open').data("tag",1);
            doDisconnect();
        }

    });
		
});