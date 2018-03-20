var connectionId;
const serial = chrome.serial;

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
	$("#console").val($("#console").val() + dat + "\n");
}

var SerialConnection = function() {
  this.connectionId = -1;
  this.buffer = new Uint8Array(0);
  this.boundOnReceive = this.onReceive.bind(this);
  this.boundOnReceiveError = this.onReceiveError.bind(this);
  this.onConnect = new chrome.Event();
  this.onReadLine = new chrome.Event();
  this.onError = new chrome.Event();
};


SerialConnection.prototype.onConnectComplete = function(connectionInfo) {
  if (!connectionInfo) {
    console.log("Connection failed.");
    return;
  }

  this.connectionId = connectionInfo.connectionId;
  this.buffer = new Uint8Array(0);

  chrome.serial.onReceive.addListener(this.boundOnReceive);
  chrome.serial.onReceiveError.addListener(this.boundOnReceiveError);

  $("button#open").html("Close Port");
  $("#cmdLine").show();

  $("#Global").load("global.html");
  $("#Piezos").load("piezos.html");
  $("#Pots").load("pots.html", function() { setupPots(); } ); 

  AddLog('Connection opened with id: ' + this.connectionId + ', Bitrate: ' + connectionInfo.bitrate);
  
  this.onConnect.dispatch();
};

SerialConnection.prototype.onReceive = function(receiveInfo) {

	//data = receiveInfo.data;
    //data = new Uint8Array(data);

	LedOn();

	data = new Uint8Array( receiveInfo.data );

	//console.log( data );

	doMidiHandler( data );

};

SerialConnection.prototype.onReceiveError = function(errorInfo) {
  if (errorInfo.connectionId === this.connectionId) {
    this.onError.dispatch(errorInfo.error);
  }
};

SerialConnection.prototype.connect = function(path, baud) {
  serial.connect(path, {bitrate: baud}, this.onConnectComplete.bind(this))
};

SerialConnection.prototype.send = function(msg) {
  if (this.connectionId < 0) {
    throw 'Invalid connection';
  }
  serial.send(this.connectionId, str2ab(msg), function() {});
};

SerialConnection.prototype.disconnect = function() {
  if (this.connectionId < 0) {
    throw 'Invalid connection';
  }
  serial.disconnect(this.connectionId, function( boolean result ) {
	  AddLog('Connection with id: ' + this.connectionId + ' closed. Result='+result);
  });

  $("#Global").load("noconn.html");
  $("#Piezos").load("noconn.html");
  $("#Pots").load("noconn.html");
  $("#led").css("background", "white");
  
};

var connection = new SerialConnection();

$(document).ready(function() {
    serial.getDevices(function(devices) {

        for (var i = 0; i < devices.length; i++) {
            $('select#portList').append('<option value="' + devices[i].path + '">' + devices[i].path + '</option>');
        }
    });
	
    // ui hook
    $('button#open').click(function() {
        var clicks = $(this).data('clicks');

        if (!clicks) {
            var port = $('select#portList').val();
			connection.connect(port, 115200);
        } else {
            connection.disconnect();
            $("button#open").html("Open Port");
			$("#cmdLine").hide();
        }

        $(this).data("clicks", !clicks);
    });
	
	$('button#send').click(function() {
		var line1 = ($("#Textfield").val() + "                ").slice(0,16);
		connection.send(line1);
		connection.send(($("#Textfield2").val() + "                ").slice(0,16));
    });
	
	$('button#clear').click(function() {
		$("#console").val("");
    });
});