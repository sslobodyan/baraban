#define POT_VELOCITY1 110
#define POT_VELOCITY127 111
#define POT_LENGTH 112
#define POT_VOLUME 113

#define PEDAL_SUSTAIN 114
#define PEDAL_VOICE 115
#define PEDAL_OCTAVE 116
#define PEDAL_PROGRAM 117
#define PEDAL_PANIC 118

void update_krutilki() { // обработать krutilka_idx-крутилку
  uint32_t tmp;
  uint8_t old_value, new_value;
  
  old_value = krutilka[ krutilka_idx ].value;
  tmp = buf_krutilka[ krutilka[krutilka_idx].mx ][ krutilka[krutilka_idx].ch ];
  new_value = map(tmp, krutilka[krutilka_idx].adc_1, krutilka[krutilka_idx].adc_127,  1, 127);
  if (new_value < 1) new_value = 1;
  if (new_value > 126) new_value = 127;

  if ( abs(new_value - old_value) >= krutilka[ krutilka_idx ].gist ) {
    // существенное изменение положения
    krutilka[ krutilka_idx ].value = new_value;
    // если есть обработчик, то выполнить его
    if ( krutilka[ krutilka_idx ].onChange ) {
      krutilka[ krutilka_idx ].onChange( krutilka_idx );
    }
  }
  
  if ( ++krutilka_idx >= NUM_MULTIPLEXORS*2 ) krutilka_idx = 0;
}

//////////////////////////////////////////////////////////////////////////
//
//                   Обработчики крутилок и педалей
//
//////////////////////////////////////////////////////////////////////////

void setPotLength( uint8_t idx ) { // обработчик 0 крутилки - время звучания ноты
  byte old = cfg.noteoff_time;
  cfg.noteoff_time = krutilka[ idx ].value * 100;
  if ( old != cfg.noteoff_time ) {
    MIDI_Master.sendControlChange( CC_NOTE_LENGTH, cfg.noteoff_time, DRUMS );
    MIDI_Slave.sendControlChange( CC_NOTE_LENGTH, cfg.noteoff_time, DRUMS );   
    DBGserial.print("Length=");DBGserial.println( cfg.noteoff_time ); 
  }
}

void setPedalSustain( uint8_t idx ) { // обработчик 1 крутилки - состояние педали
  byte old = cfg.pedal;
  if ( krutilka[ idx ].value > 85 ) {
    cfg.pedal = 127;  
  } else if ( krutilka[ idx ].value < 42 ) {
    cfg.pedal = 0;  
  } else cfg.pedal = 63; 
  if ( old !=  cfg.pedal ) {
    // отсылаем все сообщение о педали
    MIDI_Master.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    MIDI_Slave.sendControlChange( CC_FOOT_PEDAL, cfg.pedal, DRUMS );
    DBGserial.print("Pedal=");DBGserial.println( cfg.pedal );      
  }
}

void setPedalVoice( uint8_t idx ) {
  
}

void setPedalOctave( uint8_t idx ) {
  
}

void setPedalProgram( uint8_t idx ) {
  
}

void setPedalPanic( uint8_t idx ) {
  
}

void setPotVelocity1( uint8_t idx ) {
  
}

void setPotVelocity127( uint8_t idx ) {
  
}

void setPotVolume( uint8_t idx ) {
  
}


void set_type(uint8_t idx, uint8_t tp) {
  switch ( tp ) {
    case PEDAL_SUSTAIN: 
            krutilka[ idx ].onChange = setPedalSustain; break;
    case PEDAL_VOICE: 
            krutilka[ idx ].onChange = setPedalVoice; break;
    case PEDAL_OCTAVE: 
            krutilka[ idx ].onChange = setPedalOctave; break;
    case PEDAL_PROGRAM: 
            krutilka[ idx ].onChange = setPedalProgram; break;
    case PEDAL_PANIC: 
            krutilka[ idx ].onChange = setPedalPanic; break;
    case POT_VELOCITY1:
            krutilka[ idx ].onChange = setPotVelocity1; break;
    case POT_VELOCITY127:
            krutilka[ idx ].onChange = setPotVelocity127; break;    
    case POT_LENGTH:
            krutilka[ idx ].onChange = setPotLength; break;    
    case POT_VOLUME:           
            krutilka[ idx ].onChange = setPotVolume; break;    
    deafult: ;
  }
}

//////////////////////////////////////////////////////////////////////////
// Параметры по умолчанию для всех крутилок
//////////////////////////////////////////////////////////////////////////
#define krutilkaNoteLength 0
#define krutilkaPedal 1
void setup_krutilki() { // задать начальные параметры крутилкам
  // 0 - длительность звучания ноты
  krutilka[ krutilkaNoteLength ].adc_1 = 30;
  krutilka[ krutilkaNoteLength ].adc_127 = 3950;
  krutilka[ krutilkaNoteLength ].mx = 0;
  krutilka[ krutilkaNoteLength ].ch = 0;
  krutilka[ krutilkaNoteLength ].gist = 2;
  set_type( krutilkaNoteLength , POT_LENGTH );
  
  // 1 - основная педаль сустейна
  krutilka[ krutilkaPedal ].adc_1 = 15;
  krutilka[ krutilkaPedal ].adc_127 = 3950;
  krutilka[ krutilkaPedal ].mx = 0;
  krutilka[ krutilkaPedal ].ch = 1;
  krutilka[ krutilkaPedal ].gist = 5;
  set_type( krutilkaPedal , PEDAL_SUSTAIN );
}

