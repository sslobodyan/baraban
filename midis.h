struct MySettings : public midi::DefaultSettings
{
  static const unsigned SysExMaxSize = 256; // Accept SysEx messages up to 256 bytes long.
  static const long BaudRate = 115200;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, MIDIserial, MIDI, MySettings);

void doControlChange(byte channel, byte number, byte value) {
    switch (number) {
    default:                    
                    DBGserial.print(" CC channel ");
                    DBGserial.print(channel, HEX);
                    DBGserial.print(" number ");
                    DBGserial.print(number, HEX);
                    DBGserial.print(" value ");
                    DBGserial.println(value, HEX);
    }
}

void midiSetup(){
  MIDI.begin(MIDI_CHANNEL_OMNI); // слушаем команды для всех каналов
  MIDI.setHandleControlChange(doControlChange);
  MIDI.turnThruOff(); // отключить повтор входных данных
}

void note_on(byte idx) { // играть ноту по индексу из буфера нот
  //show_buf();
  
  byte ch = notes[idx].kanal;
  uint16_t level = notes[idx].level;
  
  kanal[ch].noteoff_time = millis() + cfg.noteoff_time;

  int16_t velocity = map( level , kanal[ch].velocity1, kanal[ch].velocity127, 1, 127);
  if (velocity > 126) velocity=127;
  if (velocity < 1) velocity=0;

  MIDI.sendNoteOn( kanal[ch].note , velocity, DRUMS);

  //LED_ON;
  if ( (TEST_KANAL_RED != ch) & (TEST_KANAL_GREEN != ch) ) { // вывод отчета по кроссталку
    DBGserial.print(" ");
    DBGserial.print( ch );
    DBGserial.print("=");
    DBGserial.print( notes[head_notes].level );
    DBGserial.print(" (+");
    DBGserial.print( notes[head_notes].level - kanal[ch].treshold );
    DBGserial.print(")");
    DBGserial.println();    
  } else {
    DBGserial.print( ch );
    DBGserial.print("\t");    
    DBGserial.print( ch );
    DBGserial.print("\t");    
    for (byte i=0; i<(velocity+8)/8; i++) DBGserial.print("=");
    DBGserial.println();    
  }

  if (TEST_KANAL_RED == ch) {
    RED_ON;
  }

  if (TEST_KANAL_GREEN == ch) {
    GREEN_ON;
  }
  //LED_OFF;
}

void note_off(byte ch) {
  MIDI.sendNoteOff( kanal[ch].note , 127, DRUMS);
  if (TEST_KANAL_RED == ch) {
    RED_OFF;
  }
  if (TEST_KANAL_GREEN == ch) {
    GREEN_OFF;
  }
}


