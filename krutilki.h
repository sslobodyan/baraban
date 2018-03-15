
void update_krutilki() { // обработать одну krutilka_idx-крутилку
  int16_t old_value, new_value;
  
  old_value = krutilka[ krutilka_idx ].adc;
  new_value = buf_krutilka[ krutilka[krutilka_idx].mx ][ krutilka[krutilka_idx].ch ];

  if ( abs(new_value - old_value) >= krutilka[ krutilka_idx ].gist ) { // существенное изменение положения
    krutilka[ krutilka_idx ].adc = new_value;
    new_value = map(krutilka[ krutilka_idx ].adc, krutilka[krutilka_idx].velocity1, krutilka[krutilka_idx].velocity127,  1, 127);
    if (new_value < 0) new_value = 0;
    if (new_value > 126) new_value = 127;
    krutilka[ krutilka_idx ].value = new_value;
    // если есть обработчик, то выполнить его
    if ( krutilka[ krutilka_idx ].onChange != NULL ) {
      krutilka[ krutilka_idx ].onChange( new_value );
    }
    // если запросили вывод SysEx о состоянии крутилок
    if ( krutilka[ krutilka_idx ].show ) {
      send_sysex_krutilka_08( krutilka_idx );
      DBGserial.print("Kr_");
      DBGserial.print(krutilka_idx);
      DBGserial.print("=");
      DBGserial.println(new_value);
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
void setPotMetronom( uint8_t value ) { 
  uint16_t old = cfg.metronom;
  int32_t tempo = map(value, 0, 127, METRONOME_MIN, METRONOME_MAX);
  cfg.metronom = (int32_t) 60000 / tempo ;
  if ( value != cfg.metronom ) {
    MIDI_Master.sendControlChange( CC_METRONOM, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Metronome ");
      DBGserial.print( value ); 
      DBGserial.print(" ="); 
      DBGserial.println( tempo ); 
    }
  }
}

void setPotCrossPercent( uint8_t value ) { 
  if ( value != cfg.cross_percent ) {
    cfg.cross_percent = value;
    MIDI_Master.sendControlChange( CC_CROSS_PRCNT, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("CrossPercent=");DBGserial.println( value ); // ToDo Debug
    }
  }
}

void setPotMuteCnt( uint8_t value ) { 
  byte old = cfg.mute_cnt;
  cfg.mute_cnt = value * 4;
  if ( old != cfg.mute_cnt ) {
    MIDI_Master.sendControlChange( CC_MUTE_CNT, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("MuteCnt=");DBGserial.println( cfg.mute_cnt ); // ToDo Debug
    }
  }
}

void setPotScanCnt( uint8_t value ) { 
  byte old = cfg.scan_cnt;
  cfg.scan_cnt = value;
  if ( old != cfg.scan_cnt ) {
    MIDI_Master.sendControlChange( CC_SCAN_CNT, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("ScanCnt=");DBGserial.println( cfg.scan_cnt ); // ToDo Debug
    }
  }
}

void setPotCrossCnt( uint8_t value ) { 
  byte old = cfg.cross_cnt;
  cfg.cross_cnt = value;
  if ( old != cfg.cross_cnt ) {
    MIDI_Master.sendControlChange( CC_CROSS_CNT, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("CrossCnt=");DBGserial.println( cfg.cross_cnt ); // ToDo Debug
    }
  }
}

void setPotLength0( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты без педали
  byte old = cfg.noteoff_time0;
  cfg.noteoff_time0 = value * 100;
  if ( old != cfg.noteoff_time0 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH0, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Length0=");DBGserial.println( cfg.noteoff_time0 ); // ToDo Debug
    }
  }
}

void setPotLength1( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты с полупедалью
  byte old = cfg.noteoff_time1;
  cfg.noteoff_time1 = value * 100;
  if ( old != cfg.noteoff_time1 ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH1, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Length1=");DBGserial.println( cfg.noteoff_time1 ); // ToDo Debug
    }
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
    if (cfg.show_debug) {
      DBGserial.print("Sustain=");DBGserial.println( cfg.pedal ); // ToDo Deb
    }
  }
}

void setPedalVoice( uint8_t value ) {
  byte old = cfg.pedal_voice;
  if (value < 42) cfg.pedal_voice = cfg.delta_voice;
  else if (value > 85) cfg.pedal_voice = cfg.delta_voice * 2;
  else cfg.pedal_voice = 0;
  if ( old != cfg.pedal_voice ) {
    MIDI_Master.sendControlChange( CC_VOICE_PEDAL, cfg.pedal_voice, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("MultiVoice=");DBGserial.println( cfg.pedal_voice ); // ToDo Debug
    }
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
    if (cfg.show_debug) {
      DBGserial.print("Octave=");DBGserial.println( cfg.pedal_octave ); // ToDo Debug    
    }
  }    
}

void setPedalProgram( uint8_t value ) {
  byte old = cfg.curr_program;
  if ( value < 32 ) {
    if ( cfg.pedal_program != PEDAL_DOWN ) {
      cfg.pedal_program = PEDAL_DOWN;
      if (cfg.curr_program > 0) {
        cfg.curr_program -= 1;
        if (cfg.show_debug) {
          DBGserial.print("Program-- =");DBGserial.println( cfg.curr_program ); // ToDo Debug
        }
      }      
    }
  } else if ( value > 95 ) {
    if ( cfg.pedal_program != PEDAL_UP ) {
      cfg.pedal_program = PEDAL_UP;
      cfg.curr_program += 1;
      if (cfg.curr_program > cfg.max_program) cfg.curr_program = cfg.max_program;
      if (cfg.show_debug) {
        DBGserial.print("Program++ =");DBGserial.println( cfg.curr_program ); // ToDo Debug
      }
    }
  } else if ( value > 55 && value < 75 ) {
    cfg.pedal_program = PEDAL_CENTER; // среднее положение
  }
  if ( old != cfg.curr_program ) {
    MIDI_Master.sendProgramChange(cfg.curr_program, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Program=");DBGserial.println( cfg.curr_program ); // ToDo Debug
    }
  }    
}

void setPedalPanic( uint8_t value ) {
  if (value < 42) {
    MIDI_Master.sendControlChange( CC_PANIC, 127, DRUMS );
    if (cfg.show_debug) {
      DBGserial.println("Panic!"); // ToDo Debug
    }
  }   
}

void setPotVelocity1( uint8_t value ) {
  uint16_t old = cfg.velocity1;
  cfg.velocity1 = (uint16_t) value * 8;
  if ( old != cfg.velocity1 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY1, value, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Velocity1=");DBGserial.println( cfg.velocity1 ); // ToDo Debug
    }
  }  
}

void setPotVelocity127( uint8_t value ) {
  uint16_t old = cfg.velocity127;
  cfg.velocity127 = (uint16_t) value * 8;
  if ( old != cfg.velocity127 ) {
    MIDI_Master.sendControlChange( CC_VELOCITY127, cfg.velocity127, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Velocity127=");DBGserial.println( cfg.velocity127 ); // ToDo Debug
    }
  }  
}

void setPotVolume( uint8_t value ) {
  byte old = cfg.volume;
  cfg.volume = value;
  if ( old != cfg.volume ) {
    MIDI_Master.sendControlChange( CC_VOLUME, cfg.volume, DRUMS );
    if (cfg.show_debug) {
      DBGserial.print("Volume=");DBGserial.println( cfg.volume ); // ToDo Debug
    }
  } 
}

void setPotVolumeMetronome( uint8_t value ) {
  if (value < 5) {
    value = 0;
    RED_OFF;
    GREEN_OFF;
  }
  if ( value != cfg.metronom_volume ) {
    cfg.metronom_volume = value;
    if (cfg.show_debug) {
      DBGserial.print("Metronome Volume=");DBGserial.println( cfg.metronom_volume ); // ToDo Debug
    }
  } 
}

void setPedalMetronome1( uint8_t value ) {
  byte old = cfg.pedal_metronom1;
  if ( value < 32 ) {
    if ( cfg.pedal_metronom1 != PEDAL_DOWN ) {
      cfg.pedal_metronom1 = PEDAL_DOWN;
      if (cfg.metronom > METRONOME_MIN) {
        cfg.metronom -= 1;
        if (cfg.show_debug) {
          DBGserial.print("Metronome-- =");DBGserial.println( cfg.metronom ); // ToDo Debug
        }
      }      
    }
  } else if ( value > 95 ) {
    if ( cfg.pedal_metronom1 != PEDAL_UP ) {
      cfg.pedal_metronom1 = PEDAL_UP;
      if (cfg.metronom < METRONOME_MAX) cfg.metronom++;
      if (cfg.show_debug) {
        DBGserial.print("Metronome++ =");DBGserial.println( cfg.metronom ); // ToDo Debug
      }
    }
  } else if ( value > 55 && value < 75 ) {
    cfg.pedal_metronom1 = PEDAL_CENTER; // среднее положение
  }
}

void setPedalMetronome10( uint8_t value ) {
  byte old = cfg.pedal_metronom10;
  if ( value < 32 ) {
    if ( cfg.pedal_metronom10 != PEDAL_DOWN ) {
      cfg.pedal_metronom10 = PEDAL_DOWN;
      if (cfg.metronom >= METRONOME_MIN+10) {
        cfg.metronom -= 10;
        if (cfg.show_debug) {
          DBGserial.print("Metronome-- =");DBGserial.println( cfg.metronom ); // ToDo Debug
        }
      }      
    }
  } else if ( value > 95 ) {
    if ( cfg.pedal_metronom10 != PEDAL_UP ) {
      cfg.pedal_metronom10 = PEDAL_UP;
      if (cfg.metronom < METRONOME_MAX-10) cfg.metronom+=10;
      if (cfg.show_debug) {
        DBGserial.print("Metronome++ =");DBGserial.println( cfg.metronom ); // ToDo Debug
      }
    }
  } else if ( value > 55 && value < 75 ) {
    cfg.pedal_metronom10 = PEDAL_CENTER; // среднее положение
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
    case POT_CROSS_PRCNT: handl = setPotCrossPercent; break;
    case POT_METRONOM: handl = setPotMetronom; break;
    case PEDAL_METRONOM1: handl = setPedalMetronome1; break;
    case PEDAL_METRONOM10: handl = setPedalMetronome10; break;
    
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
void setup_krutilki() { // задать начальные параметры крутилкам
  const byte port[8][2] = {
    {0,0},
    {1,0},
    {3,0},
    {2,0},
    {3,1},
    {0,1},
    {2,1},
    {1,1}
  };
  for( byte i=0; i<KRUTILKI_CNT; i++) {
    krutilka[i].onChange = NULL;
    krutilka[i].gist = 32;
    //krutilka[i].mx = i / 2;
    //krutilka[i].ch = i & 0b001;
    if (i<8) {
      krutilka[i].mx = port[i][0];
      krutilka[i].ch = port[i][1];
    } else {
      krutilka[i].mx = port[i%8][0]+4;
      krutilka[i].ch = port[i%8][1];      
    }
    krutilka[i].velocity1 = 100;
    krutilka[i].velocity127 = 4000;
    krutilka[i].show = 0;
  }
  //krutilka_set_type( 2, POT_METRONOM  );
  //krutilka_set_type( 1, POT_LENGTH0  );
  //krutilka_set_type( 0, POT_VOLUME_METRONOM  );
/*
Расположение крутилок на плате:
3, 5, 1, 7,  4, 6, 2, 0
на доп.плате прибавить 8
*/  
}

void show_krutilki_adc() {
  DBGserial.println();
  DBGserial.println("Krutilki:");
  for (byte i=0; i<KRUTILKI_CNT; i++) {
    if (i<10) DBGserial.print(" ");
    DBGserial.print(i);
    DBGserial.print("=");
    if (krutilka[i].adc<10) DBGserial.print(" ");
    if (krutilka[i].adc<100) DBGserial.print(" ");
    if (krutilka[i].adc<1000) DBGserial.print(" ");
    DBGserial.print( krutilka[i].adc );
    DBGserial.print(" m");
    DBGserial.print( krutilka[i].mx );
    DBGserial.print("c");
    DBGserial.print( krutilka[i].ch );
    DBGserial.print("\t");
    if (i%4 == 3) DBGserial.println();
  }
}

void show_krutilki() {
  if ( tm_time < millis() ) {
    tm_time = millis() + 1000;
    DBGserial.println();
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      if (i<10) DBGserial.print(" ");
      DBGserial.print(i);
      DBGserial.print("=");
      if (krutilka[i].adc<10) DBGserial.print(" ");
      if (krutilka[i].adc<100) DBGserial.print(" ");
      if (krutilka[i].adc<1000) DBGserial.print(" ");
      DBGserial.print( krutilka[i].adc );
      DBGserial.print("  ");
    }
    DBGserial.println();
  } 
}

void show_krutilki_buf() {
  if ( tm_time < millis() ) {
    tm_time = millis() + 1000;
    DBGserial.println();
    uint16_t dat;
    for (byte i=0; i<KRUTILKI_CNT/2; i++) {
      if (i<10) DBGserial.print(" ");
      DBGserial.print(i);
      DBGserial.print("=");
      dat = buf_krutilka[i][0];
      if (dat<10) DBGserial.print(" ");
      if (dat<100) DBGserial.print(" ");
      if (dat<1000) DBGserial.print(" ");
      DBGserial.print( dat );
      DBGserial.print("/");
      dat = buf_krutilka[i][1];
      if (dat<10) DBGserial.print(" ");
      if (dat<100) DBGserial.print(" ");
      if (dat<1000) DBGserial.print(" ");
      DBGserial.print( dat );
      DBGserial.print("  ");
    }
    DBGserial.println();
  } 
}

