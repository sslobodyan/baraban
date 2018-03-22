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
    midiNote[CommandIdx] = dat;	
	if (++CommandIdx == 2) {
		Command = CommandEnum.Nothing;	
		handlerCC(midiNote[0],midiNote[1]);
	}
};

var doProgramChange = function(dat) {
	AddLog("Programm "+dat);
	Command = CommandEnum.Nothing;	
};

var SysExHandler = function() {
	switch ( midiSysEx[2] ) {
		case 0x11:
				handler_11();
				break;
		case 0x08:
				handler_08();
				break;
		case 0x07:
				handler_07();
				break;
		case 0x10:
				handler_10();
				break;
		default:
				handler_other();
	}
}


var handler_11 = function() { // параметры крутилки
	var id = midiSysEx[4];
	var adc = (midiSysEx[5] << 7) + midiSysEx[6];
    var value = getValuePot( midiSysEx[7] );
    var t1=((midiSysEx[8] << 7) + midiSysEx[9]) ;
	var vel1 = '<input style="width:60px" type="number" value="'+t1+'">';
    var t127=((midiSysEx[10] << 7) + midiSysEx[11]) ;
	var vel127 = '<input style="width:60px" type="number" value="'+t127+'">';
    var gist = '<input style="width:60px" type="number" value="'+midiSysEx[12]+'">';
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

var handler_08 = function() {
	var id = midiSysEx[4];
    var value = getValuePot( midiSysEx[5] );
	var adc = (midiSysEx[6] << 7) + midiSysEx[7];
    t = $("#trPot_"+id);
	if (t) {
		 t.find(".potValue").html(value);
		 t.find(".potAdc").html(adc);
	}
}

var handler_10 = function() { // параметры пьеза

	var id = midiSysEx[4];

    var t=((midiSysEx[5] << 7) + midiSysEx[6]) ;
	var treshold = '<input style="width:60px" type="number" value="'+t+'">';

    var note = '<input style="width:60px" type="number" value="'+midiSysEx[7]+'">';

    var t1=((midiSysEx[8] << 7) + midiSysEx[9]) ;
	var vel1 = '<input style="width:60px" type="number" value="'+t1+'">';

    var t127=((midiSysEx[10] << 7) + midiSysEx[11]) ;
	var vel127 = '<input style="width:60px" type="number" value="'+t127+'">';

    var group = '<input style="width:60px" type="number" value="'+midiSysEx[12]+'">';

    var show_check = '';
	if (midiSysEx[13]) show_check = 'checked';
	var show = '<input type="checkbox" ' + show_check +'>';

    t = $("#trPiez_"+id);
	if (t) {
		 t.find(".piezNote").html(note);
		 t.find(".piezVel1").html(vel1);
		 t.find(".piezVel127").html(vel127);
		 t.find(".piezTresh").html(treshold);
		 t.find(".piezGroup").html(group);
		 t.find(".piezShow").html(show);
	}
}

var handler_07 = function() {
	var id = midiSysEx[4]; // номер входа
	var adc = (midiSysEx[5] << 7) + midiSysEx[6];
    t = $("#trPiez_"+id);
	if (t) {
		 t.find(".piezAdc").html( getValuePiez(adc) );
	}
}

var handlerCC = function(ccNum, ccVal) { // пришел Control Change
	AddLog("CC "+ccNum+" = "+ccVal);	
	var s = $("#cc_"+ccNum+" input");
	if (s) {
		s.slider('setValue',ccVal);
	}
}

var handler_other = function() {
	AddLog("Unknown SysEx");
}

