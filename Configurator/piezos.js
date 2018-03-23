
var indexOfPiez = 0;

const PiezCNT = 32;

var GetOnePiez = function() {
	var GetPiez =  [0xF0, 0x7D, 0x7F, 0x0E, indexOfPiez, 0xF7] ;
    bb = new Uint8Array(GetPiez);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var GetAllPiez = function() {
	if ( indexOfPiez < PiezCNT ) {
		GetOnePiez();
		if (++indexOfPiez < PiezCNT) setTimeout(GetAllPiez, waitTime);
	}
}

var CreateTablePiez = function() {
	var cl;
	indexOfPiez = 0;
	var tbody = $('#table_piez');
	if ( $(tbody).find('.piezId').length != 0 ) { // таблица уже есть
		setTimeout(GetAllPiez, waitTime);
	} 
	else { // строим таблицу
		tbody.html("");
        for (var i = 0; i < PiezCNT; i++) {

			if (i<8) cl = ' class="piez18"';
			else if (i<16) cl = ' class="piez916"';
			else if (i<24) cl = ' class="piez1724"';
			else cl = ' class="piez2532"';

			var s = '<tr '+cl+' id="trPiez_'+i+'" >';
			s += '<td class="piezId">'+i+'</td>';
			s += '<td class="piezNote" ></td>';
			s += '<td class="piezOn style="width:5%"" ></td>';
			s += '<td class="piezAdc" style="width:15%"></td>';
			s += '<td class="piezDiapazon" style="width:6%"></td>';
			s += '<td class="piezVel1" ></td>';
			s += '<td class="piezVel127" ></td>';
			s += '<td class="piezTresh" ></td>';
			s += '<td class="piezGroup"></td>';
			s += '<td class="piezShow"></td>';
			s += '<td class="piezAction" style="width:15%"><button data-tag="'+i+'" class="bload">Get</button><button data-tag="'+i+'" class="bsave">Send</button></td>';
			s += '</tr>';
            tbody.append(s);
        };

		if ( $("#liPiez18.active").length == 1 ) ActivatePiez18();
		else if ( $("#liPiez916.active").length == 1 ) ActivatePiez916();
		else if ( $("#liPiez916.active").length == 1 ) ActivatePiez1724();
		else ActivatePiez2532();

		tbody.find(".bsave").each(function(){
				var idx = $(this).data("tag");
				this.addEventListener('click', function(){ SaveOnePiez(idx); } );
			});
		tbody.find(".bload").each(function(){
				var idx = $(this).data("tag");
				this.addEventListener('click', function(){ LoadOnePiez(idx); } );
			});
		setTimeout(GetAllPiez, waitTime);
	};
};


var SaveOnePiez = function( idx ) {
	var t = $("#trPiez_"+idx);
	var tresh = t.find(".piezTresh").find("input").val();
	var vel1 = t.find(".piezVel1").find("input").val();
	var vel127 = t.find(".piezVel127").find("input").val();
	var note = t.find(".piezNote").find("input").val();
	var group = t.find(".piezGroup").find("input").val();
	var show = t.find(".piezShow").find("input:checked").length;

    var cmd=[0xF0,0x7D,0x7F,0x06,idx,tresh>>7,tresh&0b1111111,vel1>>7,vel1&0b1111111,vel127>>7,vel127&0b1111111,note,group,show,0xF7] ;
    bb = new Uint8Array(cmd);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
}

var LoadOnePiez = function( idx ) {
	indexOfPiez = idx;
	GetOnePiez();
}

var setupPiez = function() {
	var but = document.getElementById('LoadAll_Piez');
	but.addEventListener('click', CreateTablePiez);

	var but = document.getElementById('liPiez18');
	but.addEventListener('click', ActivatePiez18);

	var but = document.getElementById('liPiez916');
	but.addEventListener('click', ActivatePiez916);

	var but = document.getElementById('liPiez1724');
	but.addEventListener('click', ActivatePiez1724);

	var but = document.getElementById('liPiez2532');
	but.addEventListener('click', ActivatePiez2532);

	setTimeout(CreateTablePiez, 1000);

	console.log('setupPiez');
}

var ActivatePiez18 = function() {
	$(".piez18").show();
	$(".piez916").hide();
	$(".piez1724").hide();
	$(".piez2532").hide();
}

var ActivatePiez916 = function() {
	$(".piez18").hide();
	$(".piez916").show();
	$(".piez1724").hide();
	$(".piez2532").hide();
}
var ActivatePiez1724 = function() {
	$(".piez18").hide();
	$(".piez916").hide();
	$(".piez1724").show();
	$(".piez2532").hide();
}
var ActivatePiez2532 = function() {
	$(".piez18").hide();
	$(".piez916").hide();
	$(".piez1724").hide();
	$(".piez2532").show();
}

var getValuePiez = function( dat ) {
	const maxAdc = 2000;
	var perc = dat / maxAdc * 100;
	perc = perc.toPrecision(1);
	var dan = '';
	if (dat > 1800) dan = ' bg-warning ';
    var s = '<div class="progress" style="width:50px"><div class="progress-bar '+dan+'" role="progressbar" style="width:'+perc+'%">'+ dat +'</div></div>';
	return (s);
}
