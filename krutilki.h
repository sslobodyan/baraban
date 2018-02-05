#define krutilkaNoteLength 0
#define krutilkaPedal 1

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
      krutilka[ krutilka_idx ].onChange();
    }
  }
  
  if ( ++krutilka_idx >= NUM_MULTIPLEXORS*2 ) krutilka_idx = 0;
}

void setNoteLength() { // обработчик 0 крутилки - время звучания ноты
  DBGserial.print("Length=");DBGserial.println(krutilka[ krutilkaNoteLength ].value);
  cfg.noteoff_time = krutilka[ krutilkaNoteLength ].value * 100;
}

void setPedal() { // обработчик 1 крутилки - состояние педали
  if ( krutilka[ krutilkaPedal ].value > 85 ) {
    cfg.pedal = 127;  
  } else if ( krutilka[ krutilkaPedal ].value < 42 ) {
    cfg.pedal = 0;  
  } else cfg.pedal = 63;  
  DBGserial.print("Pedal=");DBGserial.println( cfg.pedal );  
}

void setup_krutilki() { // задать параметры крутилкам
  // 0 - длительность звучания ноты
  krutilka[ krutilkaNoteLength ].adc_1 = 30;
  krutilka[ krutilkaNoteLength ].adc_127 = 3950;
  krutilka[ krutilkaNoteLength ].mx = 0;
  krutilka[ krutilkaNoteLength ].ch = 0;
  krutilka[ krutilkaNoteLength ].gist = 2;
  krutilka[ krutilkaNoteLength ].onChange = setNoteLength;
  
  // 1 - педаль
  krutilka[ krutilkaPedal ].adc_1 = 15;
  krutilka[ krutilkaPedal ].adc_127 = 3950;
  krutilka[ krutilkaPedal ].mx = 0;
  krutilka[ krutilkaPedal ].ch = 1;
  krutilka[ krutilkaPedal ].gist = 5;
  krutilka[ krutilkaPedal ].onChange = setPedal;
}

