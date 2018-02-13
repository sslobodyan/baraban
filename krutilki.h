
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

  }
      
//    DBGserial.print("kr ");
//    DBGserial.print( krutilka_idx );
//    DBGserial.print("=");
//    DBGserial.print( new_value );
//    DBGserial.print("=");
//    DBGserial.print( tmp );
//    DBGserial.print("\t");
//    if (krutilka_idx == KRUTILKI_CNT-1 ) DBGserial.println();
      
  if ( ++krutilka_idx >= KRUTILKI_CNT ) krutilka_idx = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//                   Обработчики крутилок и педалей
// Должны на входе получить новое значение крутилки и выполнить с ним действие
//
//////////////////////////////////////////////////////////////////////////

void setPotLength( uint8_t value ) { // обработчик 0 крутилки - время звучания ноты
  byte old = cfg.noteoff_time;
  cfg.noteoff_time = value * 100;
  if ( old != cfg.noteoff_time ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH, cfg.noteoff_time, DRUMS );
    MIDI_Slave.sendControlChange( CC_NOTE_LENGTH, cfg.noteoff_time, DRUMS );   
    DBGserial.print("Length=");DBGserial.println( cfg.noteoff_time ); 
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
    // отсылаем все сообщение о педали
    MIDI_Master.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    MIDI_Slave.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    DBGserial.print("Pedal=");DBGserial.println( cfg.pedal );      
  }
}

void setPedalVoice( uint8_t value ) {
  
}

void setPedalOctave( uint8_t value ) {
  
}

void setPedalProgram( uint8_t value ) {
  
}

void setPedalPanic( uint8_t value ) {
  
}

void setPotVelocity1( uint8_t value ) {
  
}

void setPotVelocity127( uint8_t value ) {
  
}

void setPotVolume( uint8_t value ) {
  
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
    case POT_LENGTH: handl = setPotLength; break;
    case POT_VOLUME: handl = setPotVolume; break;
    deafult: handl = NULL ;
  }
}

void set_type(uint8_t idx, uint8_t tp) { // назначаем крутилке обработчик
  set_handl(tp);
  krutilka[ idx ].onChange = handl;
}

//////////////////////////////////////////////////////////////////////////
// Параметры по умолчанию для всех крутилок
//////////////////////////////////////////////////////////////////////////
#define krutilkaNoteLength 0
#define krutilkaPedal 1
void setup_krutilki() { // задать начальные параметры крутилкам
  for( byte i=0; i<KRUTILKI_CNT; i++) {
    krutilka[i].onChange = NULL;
    krutilka[i].gist = 2000;
  }
  
  // 0 - длительность звучания ноты
  krutilka[ krutilkaNoteLength ].velocity1 = 30;
  krutilka[ krutilkaNoteLength ].velocity127 = 3950;
  krutilka[ krutilkaNoteLength ].mx = 0;
  krutilka[ krutilkaNoteLength ].ch = 1;
  krutilka[ krutilkaNoteLength ].gist = 2;
  set_type( krutilkaNoteLength , POT_LENGTH );
  
  // 1 - основная педаль сустейна
  krutilka[ krutilkaPedal ].velocity1 = 15;
  krutilka[ krutilkaPedal ].velocity127 = 3950;
  krutilka[ krutilkaPedal ].mx = 0;
  krutilka[ krutilkaPedal ].ch = 0;
  krutilka[ krutilkaPedal ].gist = 255;
  set_type( krutilkaPedal , PEDAL_SUSTAIN );
}

