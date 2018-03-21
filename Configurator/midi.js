/* обработка вход€щих ћ»ƒ» сообщений */



/*
если текущий бат управл€ющий - определ€ем класс команды
иначе передаем байт в предыдущий класс
*/

var CommandEnum = {
  Nothing: 0,
  NoteOn: 9,
  NoteOff: 8,
  SysEx:15,
  ControlChange:11,
  ProgramChange:12
};

Command = CommandEnum.Nothing;
CommandIdx = 0;

var doMidiHandler = function(buf) {
  if (buf) {
    //console.log(buf);
    s = buf;
	//for (var i=0; i<buf.length; i++) {
	while ( buf.length > 0 ) {
		if ( buf[0] > 127 ) { // управл€ющий код
				//LedOn();
				switch ( buf[0] >> 4 ) {
					case CommandEnum.NoteOn:
							CommandIdx = 0;
							Command = CommandEnum.NoteOn;
							break;
					case CommandEnum.NoteOff:
							CommandIdx = 0;
							Command = CommandEnum.NoteOff;
							break;
					case CommandEnum.SysEx:
							if (Command == CommandEnum.SysEx) { // уже принмаем
								if ( buf[0] == 0xF7 ) {
									// прин€т конец пакета
									doSysEx( buf[0] );
								} else {
									// ошибочный пакет - отбрасываем
									CommandIdx = 0;
									Command = CommandEnum.Nothing;
								}
							} else { // нова€ сиська
								if ( buf[0] == 0xF0 ) {
									// начинаем прием сисекса
									midiSysEx = new Array();
									doSysEx( buf[0] );
									Command = CommandEnum.SysEx;
								} else {
									// ошибочный пакет - отбрасываем
									CommandIdx = 0;
									Command = CommandEnum.Nothing;
								}
							}
							break;
					case CommandEnum.ControlChange:
							CommandIdx = 0;
							Command = CommandEnum.ControlChange;
							break;
					case CommandEnum.ProgramChange:
							CommandIdx = 0;
							Command = CommandEnum.ProgramChange;
							break;
					default:
							CommandIdx = 0;
							Command = CommandEnum.Nothing;
				}	
	   	} 
		else { // обрабатываем данные
			switch (Command) {
				case CommandEnum.NoteOn:
						doNoteOn(buf[0]);
						break;
				case CommandEnum.NoteOff:
						doNoteOff(buf[0]);
						break;
				case CommandEnum.SysEx:
						doSysEx(buf[0]);
						break;
				case CommandEnum.ControlChange:
						doControlChange(buf[0]);
						break;
				case CommandEnum.ProgramChange:
						doProgramChange(buf[0]);
						break;
				default: ;
			}
       	}
		// убираем обработанный байт
		buf = buf.slice(1);
    }
  }
};

midiNote = new Array(3);
midiCC = new Array();
midiPC = new Array();

var doNoteOn = function(dat) {
    midiNote[CommandIdx] = dat;
	if (++CommandIdx == 2) {
		AddLog("NoteOn "+midiNote[0]+" vel "+midiNote[1]);	
		Command = CommandEnum.Nothing;	
	}
};

var doNoteOff = function(dat) {
    midiNote[CommandIdx] = dat;
	if (++CommandIdx == 2) {
		AddLog("NoteOff "+midiNote[0]+" vel "+midiNote[1]);	
		Command = CommandEnum.Nothing;	
	}
};

var doSysEx = function(dat) {
	midiSysEx.push(dat);
	if (dat == 0xF7) { // конец сиськи
		s = "SysEx ";
		for (var i=0; i<midiSysEx.length; i++) {
			s += midiSysEx[i] + ",";
		}
		AddLog(s);
		SysExHandler();
	}
};

var doControlChange = function(dat) {
};

var doProgramChange = function(dat) {
};

var SysExHandler = function() {
	switch ( midiSysEx[2] ) {
		case 0x11:
				handler_11();
				break;
		
	}
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
	if ( s != "" )	return( '<option value='+tp+'>' + s + '</option>' );
	else return("");
}

var getSelectTypePot=function(tp) {
	var s='<select>';
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

var handler_11 = function() {
	var id = midiSysEx[4];
	var adc = (midiSysEx[5] << 7) + midiSysEx[6];
    var value = getValuePot( midiSysEx[7] );
	var vel1 = (midiSysEx[8] << 7) + midiSysEx[9];
	var vel127 = (midiSysEx[10] << 7) + midiSysEx[11];
    var gist = midiSysEx[12];
    var typ = getSelectTypePot( midiSysEx[13] );
    var show_check = '';
	if (midiSysEx[14]) show_check = 'checked';
	var show = '<input type="checkbox" ' + show_check +'>';

    t = $("#trPot_"+id);
	if (t) {
		 t.find(".potAdc").html(adc);
		 t.find(".potVel1").html(vel1);
		 t.find(".potVel127").html(vel127);
		 t.find(".potGist").html(gist);
		 t.find(".potValue").html(value);
         t.find(".potType").html(typ);
		 t.find(".potShow").html(show);
	}
}