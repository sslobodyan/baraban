

var indexOfPot = 0;
const waitTime = 10;
const PotsCNT = 8;

var logSendCommand =  function( dat ){
	var s=" -> Send ";
	for (var i=0; i<dat.length; i++) s += dat[i] + ",";
	AddLog( s );	
}

var GetOnePot = function() {
    var GetAllPot =  [0xF0, 0x7D, 0x7F, 0x0F, indexOfPot, 0xF7] ;
    //GetAllPot[4] = indexOfPot;
    bb = new Uint8Array(GetAllPot);
	logSendCommand( bb );
	chrome.serial.send(connectionId, bb, function( sendInfo ) {} );
	if ( ++indexOfPot < PotsCNT ) setTimeout(GetOnePot, waitTime);
}

var CreateTablePots = function() {
	indexOfPot = 0;
	var tbody = $('#table_pots');
	if ( $(tbody).find('.potId').length != 0 ) { // таблица уже есть
		setTimeout(GetOnePot, waitTime);
	} 
	else { // строим таблицу
		tbody.html("");
        for (var i = 0; i < 8; i++) {
			var s = '<tr id="trPot_'+i+'">';
			s += '<td class="potId">'+i+'</td>';
			s += '<td class="potType" style="width:15%"></td>';
			s += '<td class="potVel1"></td>';
			s += '<td class="potVel127"></td>';
			s += '<td class="potGist"></td>';
			s += '<td class="potAdc"></td>';
			s += '<td class="potShow"></td>';
			s += '<td class="potValue" style="width:15%"></td>';
			s += '<td><button data-tag="'+i+'" data-evnt="load">Load</button><button data-tag="'+i+'" data-evnt="save">Save</button></td>';
			s += '</tr>';
            tbody.append(s);
        };
		setTimeout(GetOnePot, waitTime);
	};
};

var setupPots = function() {
	var but = document.getElementById('LoadAll_Pots');
	but.addEventListener('click', CreateTablePots);
}

