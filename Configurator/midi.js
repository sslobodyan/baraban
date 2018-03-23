var CommandEnum = {
  Nothing: 0,
  NoteOn: 9,
  NoteOff: 8,
  SysEx:15,
  ControlChange:11,
  ProgramChange:12
};

var Command = CommandEnum.Nothing;
var CommandIdx = 0;
var midiNote = new Array(10);


var doMidiHandler = function(buf) {
  if (buf) {
    //console.log(buf);
    s = buf;

	//var l=""; //for (var i=0; i<buf.length; i++) {l+=buf[i]+",";}  AddLog(l);

	while ( buf.length > 0 ) {
		//l+=buf[0]+",";
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
	//AddLog(l);
  }
};

var deactivateNote = function( dat  ) {
	dat.css("color","black");
	//console.log("deactivateNote "+dat);
}

var doNoteOn = function(dat) {
	//AddLog("N "+CommandIdx+" > "+dat);
    midiNote[CommandIdx] = dat;
	if (++CommandIdx == 2) {
		AddLog("NoteOn "+midiNote[0]+" vel "+midiNote[1]);	
		Command = CommandEnum.Nothing;	
	}
	var t=$("#Piezos").find("[note="+ midiNote[0] +"]");
	if (t.length > 0) {
		//console.log(t);
		t.html(midiNote[1]);
		t.css("color","red");
		setTimeout( deactivateNote, 1000, t );
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
	switch ( midiSysEx[3] ) {
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
		case 0x14:
				handler_14();
				break;
		case 0x16:
				handler_16();
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

    var gt1=((midiSysEx[14] << 7) + midiSysEx[15]) ;
    var gt127=((midiSysEx[16] << 7) + midiSysEx[17]);
	var vmax= (t127-gt127)  - (gt1+t1);
	if (vmax < 16) vmax = (gt1+t1) + 16;
	else vmax = t127-gt127;
	var gvel1_127 = (gt1+t1)+"</br>"+(vmax);

    t = $("#trPiez_"+id);
	if (t) {
		 t.find(".piezNote").html(note);

		 var nt = t.find(".piezOn");
		 nt.attr("note", midiSysEx[7] );
		 nt.html("-");	

		 t.find(".piezVel1").html(vel1);
		 t.find(".piezVel127").html(vel127);
		 t.find(".piezTresh").html(treshold);
		 t.find(".piezGroup").html(group);
		 t.find(".piezShow").html(show);
		 t.find(".piezDiapazon").html(gvel1_127);
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

var update_slider = function(ccNum, ccVal) { 
	if (ccNum == 80) { // смена голоса
		if (ccVal<10) ccVal = '0'+ccVal;
		$("#numVoice").html(ccVal);
	}
	if (ccNum == 110) { // темп
		var t= 40.0 + (210.0-40.0) / 128.0 * ccVal;
	    t = t.toFixed(0);
		//if (t<100) t = '0'+ccVal;
		$("#metronomTempo").html(t);
	}
	var s = $("#cc_"+ccNum).find("input.slider");
	if (s) {
		//console.log(s);
		s.slider('setValue',ccVal);
		$(s).next('.ccInput').val(ccVal);
	}
}

var handlerCC = function(ccNum, ccVal) { // пришел Control Change
	AddLog("CC "+ccNum+" = "+ccVal);	
	update_slider(ccNum, ccVal);
}

var handler_other = function() {
	AddLog("Unknown SysEx "+midiSysEx[3]);
}

var handler_14 = function() {
    var t=$("#console");
	t.val( "Calibrate - freeze!!!\n" );
}

var handler_16 = function() {
	AddLog( "Config" );
	var noteoff0 = midiSysEx[4];
	update_slider(3, noteoff0);

	var noteoff1 = midiSysEx[5];
	update_slider(9, noteoff1);

	var sustain = midiSysEx[6];
	update_slider(64, sustain);

	var ped_voice = midiSysEx[7];
	update_slider(102, ped_voice);

	var voice = midiSysEx[8];
	update_slider(102, ped_voice);

	var ped_octave = midiSysEx[9];
	update_slider(103, ped_octave);

	var ped_program = midiSysEx[10];
	//update_slider(102, ped_voice);

	var curr_program = midiSysEx[11];
	//update_slider(102, ped_voice);

	var volume = midiSysEx[12];
	update_slider(7, volume);

	var cross_cnt = midiSysEx[13];
	update_slider(108, cross_cnt);

	var vel1 = midiSysEx[14];
	update_slider(104, vel1);

	var vel127 = midiSysEx[15];
	update_slider(105, vel127);

	var scan_cnt = midiSysEx[16];
	update_slider(107, scan_cnt);

	var mute_cnt = midiSysEx[17];
	update_slider(106, mute_cnt);

	var metronom = midiSysEx[18];
	update_slider(110, metronom);

	var metronom_value = midiSysEx[19];
	update_slider(112, metronom_value);

	var metronom_krat = midiSysEx[20];
	//update_slider(102, ped_voice);

	var cross_perc = midiSysEx[21];
	update_slider(109, cross_perc);

	var max_level = midiSysEx[22]<<7+midiSysEx[23];
	var ped_metro1 = midiSysEx[24];
	var ped_metro10 = midiSysEx[25];

}
