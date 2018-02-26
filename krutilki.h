
void update_krutilki() { // обработать одну krutilka_idx-крутилку
  uint32_t tmp;
  uint8_t old_value, new_value;
  
  old_value = krutilka[ krutilka_idx ].value;
  tmp = buf_krutilka[ krutilka[krutilka_idx].mx ][ krutilka[krutilka_idx].ch ];
  new_value = map(tmp, krutilka[krutilka_idx].velocity1, krutilka[krutilka_idx].velocity127,  1, 127);
  if (new_value < 1) new_value = 1;
  if (new_value > 126) new_value = 127;

  if ( abs(new_value - old_value) >= krutilka[ krutilka_idx ].gist ) {
    // существенное изменение положения
    krutilka[ krutilka_idx ].value = new_value;
    // если есть обработчик, то выполнить его
    if ( krutilka[ krutilka_idx ].onChange != NULL ) {
      krutilka[ krutilka_idx ].onChange( new_value );
    }
    // если запросили вывод SysEx о состоянии крутилок
    if ( krutilka[ krutilka_idx ].show ) {
      send_sysex_krutilka( krutilka_idx );
    }
  }
  if ( ++krutilka_idx >= KRUTILKI_CNT ) krutilka_idx = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//                   Обработчики крутилок и педалей
// Должны на входе получить новое значение крутилки и выполнить с ним действие
//
//////////////////////////////////////////////////////////////////////////

void setPotLength0( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты без педали
  byte old = cfg.noteoff_time0;
  cfg.noteoff_time0 = value * 100;
  if ( old != cfg.noteoff_time0 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH0, cfg.noteoff_time0, DRUMS );
    //DBGserial.print("Length0=");DBGserial.println( cfg.noteoff_time0 ); // ToDo Debug
  }
}

void setPotLength1( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты с полупедалью
  byte old = cfg.noteoff_time1;
  cfg.noteoff_time1 = value * 100;
  if ( old != cfg.noteoff_time1 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH1, cfg.noteoff_time1, DRUMS );
    //DBGserial.print("Length1=");DBGserial.println( cfg.noteoff_time1 ); // ToDo Debug
  }
}

void setPedalSustain( uint8_t value ) { // обработчик 1 крутилки - состояние педали
  byte old = cfg.pedal;
  if ( value > 85 ) {
    cfg.pedal = 127;  
  } else if ( value < 42 ) {
    cfg.pedal = 0;  
  } else cfg.pedal = 63; 
  if ( old !=  cfg.pedal ) {
    changePedalSustain();
    // отсылаем все сообщение о педали
    MIDI_Master.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    //DBGserial.print("Pedal=");DBGserial.println( cfg.pedal ); // ToDo Deb
  }
}

void setPedalVoice( uint8_t value ) {
  byte old = cfg.pedal_voice;
  if (value < 42) cfg.pedal_voice = cfg.delta_voice;
  else if (value > 85) cfg.pedal_voice = cfg.delta_voice * 2;
  else cfg.pedal_voice = 0;
  if ( old != cfg.pedal_voice ) {
    MIDI_Master.sendControlChange( CC_VOICE_PEDAL, cfg.pedal_voice, DRUMS );
    //DBGserial.print("Voice=");DBGserial.println( cfg.pedal_voice ); // ToDo Debug
  }  
}

void setPedalOctave( uint8_t value ) {
  // ToDo только отсылается сэмплеру, у нас ноты не сдвигаются
  byte old = cfg.pedal_octave;
  if (value < 42) cfg.pedal_octave = 0;
  else if (value > 85) cfg.pedal_octave = 24;
  else cfg.pedal_octave = 12;
  if ( old != cfg.pedal_octave ) {
    MIDI_Master.sendControlChange( CC_SHIFT_OCTAVE, cfg.pedal_octave, DRUMS );
    //DBGserial.print("Octave=");DBGserial.println( cfg.pedal_octave ); // ToDo Debug
  }  
}

void setPedalProgram( uint8_t value ) {
  byte old = cfg.pedal_program;
  if (value < 42) if (cfg.pedal_program > 0) cfg.pedal_program -= 1;
  else if (value > 85) cfg.pedal_program += 1;
  if ( old != cfg.pedal_program ) {
    MIDI_Master.sendProgramChange(cfg.pedal_program, DRUMS );
    //DBGserial.print("Program=");DBGserial.println( cfg.pedal_program ); // ToDo Debug
  }    
}

void setPedalPanic( uint8_t value ) {
  if (value < 42) {
    MIDI_Master.sendControlChange( CC_PANIC, cfg.pedal_program, DRUMS );
    //DBGserial.println("Panic!"); // ToDo Debug
  }   
}

void setPotVelocity1( uint8_t value ) {
  uint16_t old = cfg.velocity1;
  cfg.velocity1 = (uint16_t) value * 8;
  if ( old != cfg.velocity1 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY1, cfg.velocity1, DRUMS );
    //DBGserial.print("Velocity1=");DBGserial.println( cfg.velocity1 ); // ToDo Debug
  }  
}

void setPotVelocity127( uint8_t value ) {
  uint16_t old = cfg.velocity127;
  cfg.velocity127 = (uint16_t) value * 8;
  if ( old != cfg.velocity127 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY127, cfg.velocity127, DRUMS );
    //DBGserial.print("Velocity127=");DBGserial.println( cfg.velocity127 ); // ToDo Debug
  }  
}

void setPotVolume( uint8_t value ) {
  byte old = cfg.volume;
  cfg.volume = value;
  if ( old != cfg.volume ) {
    MIDI_Master.sendControlChange( CC_VOLUME, cfg.volume, DRUMS );
    //DBGserial.print("Volume=");DBGserial.println( cfg.volume ); // ToDo Debug
  } 
}

void set_handl(uint8_t tp) { // назначаем глобальной переменной обработчик крутилки
  switch ( tp ) {
    case PEDAL_SUSTAIN: handl = setPedalSustain; break;
    case PEDAL_VOICE: handl = setPedalVoice; break;
    case PEDAL_OCTAVE: handl = setPedalOctave; break;
    case PEDAL_PROGRAM: handl = setPedalProgram; break;
    case PEDAL_PANIC: handl = setPedalPanic; break;
    case POT_VELOCITY1: handl = setPotVelocity1; break;
    case POT_VELOCITY127: handl = setPotVelocity127; break;
    case POT_LENGTH0: handl = setPotLength0; break;
    case POT_LENGTH1: handl = setPotLength1; break;
    case POT_VOLUME: handl = setPotVolume; break;
    deafult: handl = NULL ;
  }
}

void krutilka_set_type(uint8_t idx, uint8_t tp) { // назначаем крутилке обработчик
  set_handl(tp);
  if ( idx < KRUTILKI_CNT ) krutilka[ idx ].onChange = handl;
}

//////////////////////////////////////////////////////////////////////////
// Параметры по умолчанию для всех крутилок
//////////////////////////////////////////////////////////////////////////
#define krutilkaNoteLength 8
#define krutilkaPedal 0
void setup_krutilki() { // задать начальные параметры крутилкам
  for( byte i=0; i<KRUTILKI_CNT; i++) {
    krutilka[i].onChange = NULL;
    krutilka[i].gist = 3;
    krutilka[i].mx = i % 8; // [0...7]
    krutilka[i].ch = i / 8; // [0...1]
    krutilka[i].velocity1 = 20;
    krutilka[i].velocity127 = 4080;
  }
  krutilka_set_type( 8, POT_LENGTH0  );
}

