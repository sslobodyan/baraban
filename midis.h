#include "sysex.h"

#define MIDI_SPEED 115200

// прописано в сэмплербоксе - не меняем
#define CC_FOOT_PEDAL 64 // 0-63-127
#define CC_VOICE 80
#define CC_PANIC 123
#define CC_VOLUME 7

// собственные команды
#define CC_NOTE_LENGTH 9
#define CC_VOICE_PEDAL 69 // 0-63-127
#define CC_SHIFT_OCTAVE 70 // ==64 - не сдвигать
#define CC_VELOCITY1 71 // 
#define CC_VELOCITY127 72 // 


struct MySettings : public midi::DefaultSettings
{
  static const unsigned SysExMaxSize = 100; // Accept SysEx messages up to 256 bytes long.
  static const long BaudRate = MIDI_SPEED;
  //static const bool Use1ByteParsing = false;
};

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, MIDIserial, MIDI_Master, MySettings);
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, DBGserial, MIDI_Slave, MySettings);

void doControlChangeMaster(byte channel, byte number, byte value) {   
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



void doNoteOnMaster(byte channel, byte note, byte velocity) {
}

void doNoteOnSlave(byte channel, byte note, byte velocity) {
}

void doNoteOffSlave(byte channel, byte note, byte velocity) {
}


void midiSetup(){
  MIDI_Master.begin(MIDI_CHANNEL_OMNI); // слушаем команды для всех каналов
  MIDI_Master.turnThruOn(); // включить повтор входных данных
  MIDI_Master.setHandleControlChange(doControlChangeMaster);
  //MIDI_Master.setHandleNoteOn(doNoteMaster);
  //MIDI_Master.setHandleNoteOff(doNoteMaster);
  MIDI_Master.setHandleSystemExclusive(sysexHanlerMaster);

  MIDI_Slave.begin(MIDI_CHANNEL_OMNI); // слушаем команды для всех каналов
  //MIDI_Slave.turnThruOff(); // отключить повтор входных данных
  MIDI_Slave.setHandleControlChange(doControlChangeSlave);
  //MIDI_Slave.setHandleNoteOn(doNoteOnSlave);
  //MIDI_Slave.setHandleNoteOff(doNoteOffSlave);
  MIDI_Slave.setHandleSystemExclusive(sysexHanlerSlave);
}

void note_on(byte idx) { // играть ноту по индексу из буфера нот
  //show_buf();
  
  byte ch = notes[idx].kanal;
  uint16_t level = notes[idx].level;
  
  int16_t velocity = map( level , kanal[ch].velocity1, kanal[ch].velocity127, 1, 127);
  if (velocity > 126) velocity=127;
  if (velocity < 1) velocity=0;

  MIDI_Master.sendNoteOn( kanal[ch].note , velocity, DRUMS);      
  kanal[ch].noteoff_time = millis() + cfg.noteoff_time;

  //LED_ON;
  if ( (TEST_KANAL_RED != ch) & (TEST_KANAL_GREEN != ch) ) { // вывод отчета по кроссталку
    DBGserial.print(" ");
    DBGserial.print( ch );
    DBGserial.print("=");
    DBGserial.print( notes[head_notes].level );
    DBGserial.print(" (+");
    DBGserial.print( notes[head_notes].level - kanal[ch].treshold );
    DBGserial.print(") ");
    DBGserial.print( kanal[ch].pressed );
    DBGserial.println();    
  } else {
    DBGserial.print( ch );
    DBGserial.print("\t");    
    DBGserial.print( ch );
    DBGserial.print("\t");    
    for (byte i=0; i<(velocity+8)/8; i++) DBGserial.print("=");
    DBGserial.print(" ");    
    for (byte i=0; i<NUM_CHANNELS; i++) {
      //DBGserial.print(kanal[i].pressed); DBGserial.print(",");
    }
    DBGserial.println( kanal[ch].pressed );
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
  
  MIDI_Master.sendNoteOff( kanal[ch].note , 0, DRUMS);

  if (TEST_KANAL_RED == ch) {
    RED_OFF;
  }
  if (TEST_KANAL_GREEN == ch) {
    GREEN_OFF;
  }
}


