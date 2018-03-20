/* ��������� �������� ���� ��������� */



/*
���� ������� ��� ����������� - ���������� ����� �������
����� �������� ���� � ���������� �����
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
		if ( buf[0] > 127 ) { // ����������� ���
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
							if (Command == CommandEnum.SysEx) {
								if ( buf[0] == 0xF7 ) {
									// ������ ����� ������
									doSysEx(buf[0]);
								} else {
									// ��������� ����� - �����������
									Command = CommandEnum.Nothing;
								}
							} else {
								if ( buf[0] == 0xF0 ) {
									// �������� ����� �������
									CommandIdx = 0;
									Command = CommandEnum.SysEx;
								} else {
									// ��������� ����� - �����������
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
		else { // ������������ ������
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
		// ������� ������������ ����
		buf = buf.slice(1);
    }
  }
};

var doNoteOn = function(dat) {
	if (++CommandIdx == 2) {
		AddLog("NoteOn");	
		$("#console").val("");
		Command = CommandEnum.Nothing;	
	}
};

var doNoteOff = function(dat) {
	if (++CommandIdx == 2) {
		AddLog("NoteOff");	
		Command = CommandEnum.Nothing;	
	}
};

var doSysEx = function(dat) {
};

var doControlChange = function(dat) {
};

var doProgramChange = function(dat) {
};
