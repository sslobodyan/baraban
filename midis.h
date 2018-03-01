#include "sysex.h"

#define MIDI_SPEED 115200

// прописано в сэмплербоксе - не меняем
#define CC_FOOT_PEDAL 64 // 0-63-127
#define CC_VOICE 80
#define CC_PANIC 120
#define CC_VOLUME 7

// собственные команды
#define CC_NOTE_LENGTH0 3
#define CC_NOTE_LENGTH1 9
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
                      changePedalSustain();
                      break;
      case CC_NOTE_LENGTH0: cfg.noteoff_time0 = value * 100;
                      break;
      case CC_NOTE_LENGTH1: cfg.noteoff_time1 = value * 100;
                      break;
      case CC_VOICE_PEDAL: cfg.pedal_voice = value;
                      break;                      
      case CC_VELOCITY1: cfg.velocity1 = value * 8;
                      break;
      case CC_VELOCITY127: cfg.velocity127 = value * 8;
                      break;
      default: break;
    }
}

void doControlChangeSlave(byte channel, byte number, byte value) {
    // обработаем если есть что-то интересное для нас от рабов
    switch (number) {
      case CC_FOOT_PEDAL:  cfg.pedal = value; 
                      changePedalSustain();
                      break;
      case CC_NOTE_LENGTH0: cfg.noteoff_time0 = value * 100;
                      break;
      case CC_NOTE_LENGTH1: cfg.noteoff_time1 = value * 100;
                      break;
      case CC_VOICE_PEDAL: cfg.pedal_voice = value;
                      break;                      
      case CC_VELOCITY1: cfg.velocity1 = value * 8;
                      break;
      case CC_VELOCITY127: cfg.velocity127 = value * 8;
                      break;
      default: break;
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
  MIDI_Slave.turnThruOff(); // отключить повтор входных данных
  MIDI_Slave.setHandleControlChange(doControlChangeSlave);
  //MIDI_Slave.setHandleNoteOn(doNoteOnSlave);
  //MIDI_Slave.setHandleNoteOff(doNoteOffSlave);
  MIDI_Slave.setHandleSystemExclusive(sysexHanlerSlave);
}

bool note_on(byte idx) { // играть ноту по индексу из буфера нот

  byte ch = notes[idx].kanal;
  uint16_t level = notes[idx].level;
  uint8_t voice;
  uint32_t time_to_off; // когда посылать note_off

  if (level == 0) return false;

  // определить каким голосом играть ноту в зависимости от датчика касания и педали сустейна, а также сдвига голосов
  if ( cfg.pedal > 42 ) { // педаль или полупедаль
    voice = 1; 
    if ( cfg.pedal < 85 ) { // полупедаль
      time_to_off = cfg.noteoff_time1;   
    } else {
      time_to_off = 1800000; // 30 минут для педали
    }
  } else { // без педали
    voice = 2; 
    time_to_off = cfg.noteoff_time0;
  }

  if ( kanal[ch].pressed ) { // было касаниe - голоса 3 и 4
    voice += 2;
  }

  voice += cfg.pedal_voice; // добавляем сдвиг голосов по педалям

  if ( voice != cfg.voice ) {
    cfg.voice = voice; // запомним текущий голос для уменьшения трафика
    MIDI_Master.sendControlChange( CC_VOICE, voice, DRUMS ); // смена голоса  
  }

  // границы громкости учитывая крутилки
  int16_t vel1 = kanal[ch].velocity1+cfg.velocity1;
  int16_t vel127 = kanal[ch].velocity127-cfg.velocity127;

  // по границам определяем уровень
  int16_t velocity = map( level , vel1, vel127, 1, 127);
  if (velocity > 126) velocity=127;
  if (velocity < 1) velocity=0;

  if (TEST_KANAL_RED == ch) {
    RED_ON;
  }

  if (TEST_KANAL_GREEN == ch) {
    GREEN_ON;
  }

  if (velocity == 0) { // прижали палочку - глушим ранее играющую ноту
    note_off( ch );
    return false;
  } else {
    MIDI_Master.sendNoteOn( kanal[ch].note , velocity, DRUMS);      
    kanal[ch].noteoff_time = millis() + time_to_off;
  }

#define SHOW_NOTE_ON_

  #ifdef SHOW_NOTE_ON
    DBGserial.print("  ");    
    DBGserial.print( ch );
    DBGserial.print("\t");    
    DBGserial.print( level );    
    DBGserial.print("\t");    
    DBGserial.print( velocity );    
    DBGserial.print(" ");    
    for (byte i=0; i<(velocity+4)/4; i++) DBGserial.print("=");
    DBGserial.print(" ");   
    DBGserial.print( notes[idx].dma_cnt );    
    DBGserial.print(" ");    
    for (byte i=0; i<NUM_CHANNELS; i++) {
      //DBGserial.print(kanal[i].pressed); DBGserial.print(",");
    }
    //DBGserial.print( kanal[ch].pressed );
    DBGserial.println();
    return true;
  #else
    return false;  
  #endif
  
  
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

void send_SysEx(byte size, byte *arr) { // выслать системное сообщение 
  MIDI_Master.sendSysEx(sizeof(arr), arr, false);
}

  

