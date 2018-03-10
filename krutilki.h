
void update_krutilki() { // обработать одну krutilka_idx-крутилку
  uint8_t old_value, new_value;
  
  old_value = krutilka[ krutilka_idx ].value;
  krutilka[ krutilka_idx ].adc = buf_krutilka[ krutilka[krutilka_idx].mx ][ krutilka[krutilka_idx].ch ];
  new_value = map(krutilka[ krutilka_idx ].adc, krutilka[krutilka_idx].velocity1, krutilka[krutilka_idx].velocity127,  1, 127);
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
      send_sysex_krutilka_08( krutilka_idx );
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
void setPotMuteCnt( uint8_t value ) { 
  byte old = cfg.mute_cnt;
  cfg.mute_cnt = value * 4;
  if ( old != cfg.mute_cnt ) {
    MIDI_Master.sendControlChange( CC_MUTE_CNT, value, DRUMS );
    DBGserial.print("MuteCnt=");DBGserial.println( cfg.mute_cnt ); // ToDo Debug
  }
}

void setPotScanCnt( uint8_t value ) { 
  byte old = cfg.scan_cnt;
  cfg.scan_cnt = value;
  if ( old != cfg.scan_cnt ) {
    MIDI_Master.sendControlChange( CC_SCAN_CNT, value, DRUMS );
    DBGserial.print("ScanCnt=");DBGserial.println( cfg.scan_cnt ); // ToDo Debug
  }
}

void setPotCrossCnt( uint8_t value ) { 
  byte old = cfg.cross_cnt;
  cfg.cross_cnt = value;
  if ( old != cfg.cross_cnt ) {
    MIDI_Master.sendControlChange( CC_CROSS_CNT, value, DRUMS );
    DBGserial.print("CrossCnt=");DBGserial.println( cfg.cross_cnt ); // ToDo Debug
  }
}

void setPotLength0( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты без педали
  byte old = cfg.noteoff_time0;
  cfg.noteoff_time0 = value * 100;
  if ( old != cfg.noteoff_time0 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH0, value, DRUMS );
    DBGserial.print("Length0=");DBGserial.println( cfg.noteoff_time0 ); // ToDo Debug
  }
}

void setPotLength1( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты с полупедалью
  byte old = cfg.noteoff_time1;
  cfg.noteoff_time1 = value * 100;
  if ( old != cfg.noteoff_time1 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH1, value, DRUMS );
    DBGserial.print("Length1=");DBGserial.println( cfg.noteoff_time1 ); // ToDo Debug
  }
}

void setPedalSustain( uint8_t value ) { // обработчик 1 крутилки - состояние педали
  byte old = cfg.pedal;
  if ( value > 85 ) {
    cfg.pedal = PEDAL_UP;  
  } else if ( value < 42 ) {
    cfg.pedal = PEDAL_DOWN;  
  } else if ( value > 55 && value < 75 ) cfg.pedal = PEDAL_CENTER; 
  if ( old !=  cfg.pedal ) {
    changePedalSustain();
    // отсылаем все сообщение о педали
    MIDI_Master.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    DBGserial.print("Sustain=");DBGserial.println( cfg.pedal ); // ToDo Deb
  }
}

void setPedalVoice( uint8_t value ) {
  byte old = cfg.pedal_voice;
  if (value < 42) cfg.pedal_voice = cfg.delta_voice;
  else if (value > 85) cfg.pedal_voice = cfg.delta_voice * 2;
  else cfg.pedal_voice = 0;
  if ( old != cfg.pedal_voice ) {
    MIDI_Master.sendControlChange( CC_VOICE_PEDAL, cfg.pedal_voice, DRUMS );
    DBGserial.print("Voice=");DBGserial.println( cfg.pedal_voice ); // ToDo Debug
  }  
}

void setPedalOctave( uint8_t value ) {
  // ToDo только отсылается сэмплеру, у нас ноты не сдвигаются
  byte old = cfg.pedal_octave;
  if ( value < 32 ) {
    if ( cfg.pedal_octave != PEDAL_DOWN ) {
      cfg.pedal_octave = PEDAL_DOWN;
    }
  } else if ( value > 95 ) {
    if ( cfg.pedal_octave != PEDAL_UP ) {
      cfg.pedal_octave = PEDAL_UP;
    }
  } else if ( value > 55 && value < 75 ) {
    cfg.pedal_octave = PEDAL_CENTER; // среднее положение
  }
  if ( old != cfg.pedal_octave ) {
    // стандартно 0х40 0х00 что дает 8192, каждая октава изменяет на 1536, т.е 6656 и 9728
    // if (cfg.pedal_octave == PEDAL_DOWN) MIDI_Master.sendPitchBend( 6656, DRUMS );
    // if (cfg.pedal_octave == PEDAL_CENTER) MIDI_Master.sendPitchBend( 8192, DRUMS );
    //if (cfg.pedal_octave == PEDAL_UP) MIDI_Master.sendPitchBend( 9728, DRUMS );
    
    // отошлем наше СС
    MIDI_Master.sendControlChange( CC_SHIFT_OCTAVE, cfg.pedal_octave, DRUMS );
    DBGserial.print("Octave=");DBGserial.println( cfg.pedal_octave ); // ToDo Debug    
  }    
}

void setPedalProgram( uint8_t value ) {
  byte old = cfg.curr_program;
  if ( value < 32 ) {
    if ( cfg.pedal_program != PEDAL_DOWN ) {
      cfg.pedal_program = PEDAL_DOWN;
      if (cfg.curr_program > 0) {
        cfg.curr_program -= 1;
        DBGserial.print("Program-- =");DBGserial.println( cfg.curr_program ); // ToDo Debug
      }      
    }
  } else if ( value > 95 ) {
    if ( cfg.pedal_program != PEDAL_UP ) {
      cfg.pedal_program = PEDAL_UP;
      cfg.curr_program += 1;
      if (cfg.curr_program > cfg.max_program) cfg.curr_program = cfg.max_program;
      DBGserial.print("Program++ =");DBGserial.println( cfg.curr_program ); // ToDo Debug
    }
  } else if ( value > 55 && value < 75 ) {
    cfg.pedal_program = PEDAL_CENTER; // среднее положение
  }
  if ( old != cfg.curr_program ) {
    MIDI_Master.sendProgramChange(cfg.curr_program, DRUMS );
    DBGserial.print("Program=");DBGserial.println( cfg.curr_program ); // ToDo Debug
  }    
}

void setPedalPanic( uint8_t value ) {
  if (value < 42) {
    MIDI_Master.sendControlChange( CC_PANIC, 127, DRUMS );
    DBGserial.println("Panic!"); // ToDo Debug
  }   
}

void setPotVelocity1( uint8_t value ) {
  uint16_t old = cfg.velocity1;
  cfg.velocity1 = (uint16_t) value * 8;
  if ( old != cfg.velocity1 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY1, value, DRUMS );
    DBGserial.print("Velocity1=");DBGserial.println( cfg.velocity1 ); // ToDo Debug
  }  
}

void setPotVelocity127( uint8_t value ) {
  uint16_t old = cfg.velocity127;
  cfg.velocity127 = (uint16_t) value * 8;
  if ( old != cfg.velocity127 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY127, cfg.velocity127, DRUMS );
    DBGserial.print("Velocity127=");DBGserial.println( cfg.velocity127 ); // ToDo Debug
  }  
}

void setPotVolume( uint8_t value ) {
  byte old = cfg.volume;
  cfg.volume = value;
  if ( old != cfg.volume ) {
    MIDI_Master.sendControlChange( CC_VOLUME, cfg.volume, DRUMS );
    DBGserial.print("Volume=");DBGserial.println( cfg.volume ); // ToDo Debug
  } 
}

void setPotVolumeMetronome( uint8_t value ) {
  if ( value != cfg.metronom_volume ) {
    cfg.metronom_volume = value;
    DBGserial.print("Metronome Volume=");DBGserial.println( cfg.metronom_volume ); // ToDo Debug
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
    case POT_VOLUME_METRONOM: handl = setPotVolumeMetronome; break;
    deafult: handl = NULL ;
  }
}

void krutilka_set_type(uint8_t idx, uint8_t tp) { // назначаем крутилке обработчик
  if ( idx < KRUTILKI_CNT ) {
    if (tp > 0) {
      set_handl(tp);
      krutilka[ idx ].onChange = handl;
    } else {
      krutilka[ idx ].onChange = NULL;
    }
  }
}

//////////////////////////////////////////////////////////////////////////
// Параметры по умолчанию для всех крутилок
//////////////////////////////////////////////////////////////////////////
#define krutilkaNoteLength 8
#define krutilkaPedal 0
void setup_krutilki() { // задать начальные параметры крутилкам
  for( byte i=0; i<KRUTILKI_CNT; i++) {
    krutilka[i].onChange = NULL;
    krutilka[i].gist = 5;
    krutilka[i].mx = i % 8; // [0...7]
    krutilka[i].ch = i / 8; // [0...1]
    krutilka[i].velocity1 = 20;
    krutilka[i].velocity127 = 4080;
  }
  //krutilka_set_type( 8, POT_LENGTH0  );
}

