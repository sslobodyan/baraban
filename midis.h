#define MIDI_SPEED 115200

// прописано в сэмплербоксе - не меняем
#define CC_FOOT_PEDAL 64
#define CC_VOICE 80
#define CC_PANIC 123
#define CC_VOLUME 7

// собственные команды
#define CC_NOTE_LENGTH 9


struct MySettings : public midi::DefaultSettings
{
  static const unsigned SysExMaxSize = 256; // Accept SysEx messages up to 256 bytes long.
  static const long BaudRate = MIDI_SPEED;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, MIDIserial, MIDI_Master, MySettings);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, DBGserial, MIDI_Slave, MySettings);

void doControlChangeMaster(byte channel, byte number, byte value) {
    // пришло от вышестоящих - обязательно к исполнению и повтору для младших
    MIDI_Slave.sendControlChange( number, value, channel );    
    switch (number) {
      case CC_FOOT_PEDAL:  cfg.pedal = value; 
                      break;
      case CC_NOTE_LENGTH: cfg.noteoff_time = value;
                      break;
      default:                    
            DBGserial.print(" Master CC channel ");
            DBGserial.print(channel, HEX);
            DBGserial.print(" number ");
            DBGserial.print(number, HEX);
            DBGserial.print(" value ");
            DBGserial.println(value, HEX);
    }
}

void doControlChangeSlave(byte channel, byte number, byte value) {
    // передаем повыше
    MIDI_Master.sendControlChange( number, value, channel );
    // обработаем если есть что-то интересное для нас от рабов
    switch (number) {
      case CC_FOOT_PEDAL:  cfg.pedal = value; 
                      break;
      case CC_NOTE_LENGTH: cfg.noteoff_time = value;
                      break;
      default:                    
            DBGserial.print(" Slave CC channel ");
            DBGserial.print(channel, HEX);
            DBGserial.print(" number ");
            DBGserial.print(number, HEX);
            DBGserial.print(" value ");
            DBGserial.println(value, HEX);
    }
}

void doNoteMaster(byte channel, byte note, byte velocity) {
  // пока на мастере все ноты игнорим
}

void doNoteOnSlave(byte channel, byte note, byte velocity) {
  // передать повыше
  MIDI_Master.sendNoteOn( note, velocity, channel );
}

void doNoteOffSlave(byte channel, byte note, byte velocity) {
  // передать повыше
  MIDI_Master.sendNoteOff( note, velocity, channel );
}


void midiSetup(){
  MIDI_Master.begin(MIDI_CHANNEL_OMNI); // слушаем команды для всех каналов
  MIDI_Master.setHandleControlChange(doControlChangeMaster);
  MIDI_Master.setHandleNoteOn(doNoteMaster);
  MIDI_Master.setHandleNoteOff(doNoteMaster);
  MIDI_Master.turnThruOff(); // отключить повтор входных данных

  MIDI_Slave.begin(MIDI_CHANNEL_OMNI); // слушаем команды для всех каналов
  MIDI_Slave.turnThruOff(); // отключить повтор входных данных
  MIDI_Slave.setHandleControlChange(doControlChangeSlave);
  MIDI_Slave.setHandleNoteOn(doNoteOnSlave);
  MIDI_Slave.setHandleNoteOff(doNoteOffSlave);
}

void note_on(byte idx) { // играть ноту по индексу из буфера нот
  //show_buf();
  
  byte ch = notes[idx].kanal;
  uint16_t level = notes[idx].level;
  
  kanal[ch].noteoff_time = millis() + cfg.noteoff_time;

  int16_t velocity = map( level , kanal[ch].velocity1, kanal[ch].velocity127, 1, 127);
  if (velocity > 126) velocity=127;
  if (velocity < 1) velocity=0;

  // ToDo Здесь по типу входа определяем играть ноту или выполнить обработчик крутилки

  MIDI_Master.sendNoteOn( kanal[ch].note , velocity, DRUMS);

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
  MIDI_Master.sendNoteOff( kanal[ch].note , 127, DRUMS);
  if (TEST_KANAL_RED == ch) {
    RED_OFF;
  }
  if (TEST_KANAL_GREEN == ch) {
    GREEN_OFF;
  }
}


