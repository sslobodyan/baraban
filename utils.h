
void print_adc_buffer(byte from, byte to) {
  byte i,idx=0;
  bool ena=false;
  for (i=from*2; i<to*2; i++) {
    if ( buf_adc[last_buf_idx][i] > 550 ) {
      ena=true;
      idx = i;
    }
  }  
  if ( ena == false ) return;
  DBGserial.print("idx=");
  DBGserial.print(idx/2);
  DBGserial.print("  ");
  for (i=from*2; i<to*2; i++) {
    if (i%2) DBGserial.print((byte)i/2);
    else DBGserial.print("#");
    DBGserial.print("=");
    DBGserial.print(buf_adc [last_buf_idx] [i] );
    DBGserial.print("\t");
  }
  DBGserial.println();
}

void setup_kanal() {
  for (byte i=0; i<NUM_CHANNELS; i++) {
    kanal[i].note = i+35;
    kanal[i].treshold = 0xFFFF;
    kanal[i].velocity1 = 300;
    kanal[i].velocity127 = 1300;
  }
}

void add_note(byte ch, uint16_t level) { // из прерывания
  if (++head_notes >= NOTES_CNT) head_notes = 0;
  notes[head_notes].kanal = ch;
  notes[head_notes].level = level;
}

void show_buf(){ // чисто отладка
  byte n = last_buf_idx;
  for (byte i=0; i<BUFFER_CNT; i++ ) {
    if (++n >= BUFFER_CNT) n=0;
    DBGserial.print( i );
    DBGserial.print( "\t\t" );
    for (byte k=0; k<32; k++) {
      //DBGserial.print("(");DBGserial.print( k );DBGserial.print(") ");
      if (buf_adc[n][k] < 1000) DBGserial.print(" ");
      if (buf_adc[n][k] < 100) DBGserial.print(" ");
      if (buf_adc[n][k] < 10) DBGserial.print(" ");
      DBGserial.print( buf_adc[n][k] );
      DBGserial.print( " / " );
    }
    DBGserial.println( );
    delay(20);
  }
}

void update_krutilki() { // обработать krutilka_idx-крутилку
  uint32_t tmp;
  uint8_t old_value, new_value;
  
  old_value = krutilka[ krutilka_idx ].value;
  tmp = buf_krutilka[ krutilka[krutilka_idx].mx ][ krutilka[krutilka_idx].ch ];
  new_value = map(tmp, krutilka[krutilka_idx].adc_1, krutilka[krutilka_idx].adc_127,  1, 127);
  if (new_value < 1) new_value = 1;
  if (new_value > 126) new_value = 127;

  if ( abs(new_value - old_value) > krutilka[ krutilka_idx ].gist ) {
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
  DBGserial.print("Length=");DBGserial.println(krutilka[0].value);
  cfg.noteoff_time = krutilka[0].value * 100;
}

void setup_krutilki() { // задать параметры крутилкам
  // 0 - длительность звучания ноты
  krutilka[ 0 ].adc_1 = 15;
  krutilka[ 0 ].adc_127 = 3950;
  krutilka[ 0 ].mx = 0;
  krutilka[ 0 ].ch = 0;
  krutilka[ 0 ].gist = 2;
  krutilka[ 0 ].onChange = setNoteLength;
  
  // 1 - педаль
  krutilka[ 1 ].adc_1 = 15;
  krutilka[ 1 ].adc_127 = 3950;
  krutilka[ 1 ].mx = 0;
  krutilka[ 1 ].ch = 1;
  krutilka[ 1 ].gist = 5;
}

