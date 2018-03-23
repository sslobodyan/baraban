

var indexOfPot = 0;

const PotsCNT = 16;

var GetOnePot = function() {
	var GetOnePot =  [0xF0, 0x7D, 0x7F, 0x0F, indexOfPot, 0xF7] ;
    bb = new Uint8Array(GetOnePot);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var GetAllPot = function() {
	if ( indexOfPot < PotsCNT ) {
		GetOnePot();
		if (++indexOfPot < PotsCNT) setTimeout(GetAllPot, waitTime);
	}
}

var CreateTablePots = function() {
	var cl;
	indexOfPot = 0;
	var tbody = $('#table_pots');
	if ( $(tbody).find('.potId').length != 0 ) { // таблица уже есть
		setTimeout(GetAllPot, waitTime);
	} 
	else { // строим таблицу
		tbody.html("");
        for (var i = 0; i < PotsCNT; i++) {

			if (i<8) cl = ' class="pot18"';
			else cl = ' class="pot916"';

			var s = '<tr '+cl+' id="trPot_'+i+'" >';
			s += '<td class="potId">'+i+'</td>';
			s += '<td class="potType" ></td>';
			s += '<td class="potVel1" ></td>';
			s += '<td class="potVel127" ></td>';
			s += '<td class="potGist" ></td>';
			s += '<td class="potAdc"></td>';
			s += '<td class="potShow"></td>';
			s += '<td class="potValue" style="width:15%"></td>';
			s += '<td class="potAction" style="width:15%"><button data-tag="'+i+'" class="bload">Get</button><button data-tag="'+i+'" class="bsave">Send</button></td>';
			s += '</tr>';
            tbody.append(s);
        };

		if ( $("#liPot18.active").length == 1 ) ActivatePot18();
		else ActivatePot916();

		tbody.find(".bsave").each(function(){
				var idx = $(this).data("tag");
				this.addEventListener('click', function(){ SaveOnePot_01(idx); } );
			});
		tbody.find(".bload").each(function(){
				var idx = $(this).data("tag");
				this.addEventListener('click', function(){ LoadOnePot_01(idx); } );
			});
		setTimeout(GetAllPot, waitTime);
	};
};


var SaveOnePot_01 = function( idx ) {
	var t = $("#trPot_"+idx);
	//console.log(t);
	var typePot = t.find(".potType").find(":selected").val();
	var vel1 = t.find(".potVel1").find("input").val();
	var vel127 = t.find(".potVel127").find("input").val();
	var gist = t.find(".potGist").find("input").val();
	var show = t.find(".potShow").find("input:checked").length;
    var cmd =  [0xF0, 0x7D, 0x7F, 0x01, idx, typePot, vel1 >> 7, vel1 & 0b1111111, vel127 >> 7, vel127 & 0b1111111, gist, show , 0xF7] ;
    bb = new Uint8Array(cmd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var LoadOnePot_01 = function( idx ) {
	indexOfPot = idx;
	GetOnePot();
}

var setupPots = function() {
	var but = document.getElementById('LoadAll_Pots');
	but.addEventListener('click', CreateTablePots);

	var but = document.getElementById('liPot18');
	but.addEventListener('click', ActivatePot18);

	var but = document.getElementById('liPot916');
	but.addEventListener('click', ActivatePot916);

	CreateTablePots();

	console.log('setupPots');
}

var ActivatePot18 = function() {
	$(".pot18").show();
	$(".pot916").hide();
}

var ActivatePot916 = function() {
	$(".pot18").hide();
	$(".pot916").show();
}

var getTypePot = function(tp) {
	switch (tp) {
		case 110: return("POT_VELOCITY1"); break;
		case 111: return("POT_VELOCITY127"); break;
		case 112: return("POT_LENGTH0"); break;
		case 119: return("POT_LENGTH1"); break;
		case 113: return("POT_VOLUME"); break;
		case 120: return("POT_VOLUME_METRONOM"); break;
		case 121: return("POT_MUTE_CNT"); break;
		case 122: return("POT_SCAN_CNT"); break;
		case 123: return("POT_CROSS_CNT"); break;
		case 124: return("POT_CROSS_PRCNT"); break;
		case 125: return("POT_METRONOM"); break;
		case 109: return("PEDAL_AUTOTRESHOLD"); break;
		case 114: return("PEDAL_SUSTAIN"); break;
		case 115: return("PEDAL_VOICE"); break;
		case 116: return("PEDAL_OCTAVE"); break;
		case 117: return("PEDAL_PROGRAM"); break;
		case 118: return("PEDAL_PANIC"); break;
		case 126: return("PEDAL_METRONOM1"); break;
		case 127: return("PEDAL_METRONOM10"); break;
		case 0: return("No Defined"); break;
		default: return(""); break;
	}
}

var getOptionTypePot = function(tp, sel) {
	var s = getTypePot(tp);
	if ( s != "" )	return( '<option value='+tp+' '+sel+'>' + s + '</option>' );
	else return("");
}

var getSelectTypePot=function(tp) {
	var s='<select  >';
	for (var i=0; i<128; i++) {
		var sel = '';
		if ( i == tp ) sel = 'selected="selected"';
		var opt = getOptionTypePot(i, sel);
		if ( opt != '' ) s += opt;
	}
    s += '</select>';
	return( s );
}

var getValuePot = function( dat ) {
	var perc = dat / 128 * 100;
	perc = perc.toPrecision(1);
   var s = '<div class="progress" style="width:50px"><div class="progress-bar" role="progressbar" style="width:'+perc+'%">'+ dat +'</div></div>';
	return (s);
}
