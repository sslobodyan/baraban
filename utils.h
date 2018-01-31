
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

void note_on(byte idx) { // играть ноту по индексу из буфера нот
  //show_buf();
  kanal[ notes[idx].kanal ].noteoff_time = millis() + cfg.noteoff_time;
  LED_ON;
  //if ( (TEST_KANAL_RED != notes[idx].kanal) & (TEST_KANAL_GREEN != notes[idx].kanal)) {
    DBGserial.print(" ");
    DBGserial.print( notes[idx].kanal );
    DBGserial.print("=");
    DBGserial.print( notes[head_notes].level );
    DBGserial.print(" (+");
    DBGserial.print( notes[head_notes].level - kanal[ notes[idx].kanal ].treshold );
    DBGserial.print(")");
    DBGserial.println();    
  //}

  if (TEST_KANAL_RED == notes[idx].kanal) {
    RED_ON;
  }

  if (TEST_KANAL_GREEN == notes[idx].kanal) {
    GREEN_ON;
  }
  
  LED_OFF;
}

void note_off(byte ch) {
  if (TEST_KANAL_RED == ch) {
    RED_OFF;
  }
  if (TEST_KANAL_GREEN == ch) {
    GREEN_OFF;
  }
}

