
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

void fill_notes() { // присвоить используемым входам номера нот
  for (byte i=0; i<NUM_CHANNELS; i++) {
    if (i+cfg.start_note <= cfg.end_note) {
      kanal[i].note = i+cfg.start_note;  
    } else kanal[i].note = 0; // канал не используется
  }  
}

void setup_kanal() {
  fill_notes();
  for (byte i=0; i<NUM_CHANNELS; i++) {
    kanal[i].treshold = 0xFFFF;
    kanal[i].velocity1 = 300;
    kanal[i].velocity127 = 1300;
    kanal[i].scan_cnt = -1;
  }
}

void setup_touch() {
  for (byte mdl=0; mdl<4; mdl++) {
    for (byte n=0; n<8; n++) {
      touch[mdl].kanal[n] = n;
    }
  }
}

void add_note(byte ch, uint16_t level) { // из прерывания строим кольцевой буфер сработавших каналов
  byte idx = head_notes;
  if ( ++idx >= NOTES_CNT) idx = 0;
  notes[idx].kanal = ch;
  notes[idx].level = level;
  notes[idx].micros = micros(); // время удара для обработки групп сенсоров
  notes[idx].cross_cnt = cross_cnt + cfg.cross_cnt ;
  head_notes = idx;
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

void changePedalSustain() {
  if (cfg.pedal == PEDAL_UP) {
    // заглушить все ноты при отпускании педали
    for (byte i=0; i<NUM_CHANNELS; i++) { // проверка на время note_off
      if ( kanal[i].noteoff_time ) {
          note_off(i);
          kanal[i].noteoff_time = 0;
      }
    }
  }
}

bool check_groups() { // контроль кросстолка
byte idx_note;

#define SHOW_GROUPS_HIDE_

  if (tail_notes >= NOTES_CNT) idx_note = 0; 
  else idx_note = tail_notes + 1;

  if ( cross_cnt < notes[ idx_note ].cross_cnt ) return false; // еще не прошло время контроля кросстолка - играть запрещаем
  
//  if ( micros() - notes[ idx_note ].micros < cfg.time_crosstalk )  {
//    return false; // еще не прошло время контроля кросстолка - играть запрещаем
//  }
  
    // ищем самый громкий сигнал
    int max_idx = 0;
    uint16_t max_level = 0;
    idx_note = tail_notes;
    while ( idx_note != head_notes) {
      if ( ++idx_note >= NOTES_CNT) idx_note = 0;
      #ifdef SHOW_GROUPS_HIDE
        DBGserial.print( notes[idx_note].kanal ); DBGserial.print(">"); DBGserial.print(notes[idx_note].level); 
        DBGserial.print(" t "); 
        DBGserial.print( notes[idx_note].cross_cnt );
        DBGserial.print(" / ");
        //DBGserial.print(notes[idx_note].cross_cnt);
        //DBGserial.print(" / ");
      #endif  
      if ( notes[idx_note].level > max_level ) {
        max_level = notes[idx_note].level;
        max_idx = idx_note;
      }
    }
    #ifdef SHOW_GROUPS_HIDE
      DBGserial.println(); DBGserial.print("max ");DBGserial.println( notes[max_idx].kanal );
    #endif  
    // глушим все более слабые сигналы
    idx_note = tail_notes;
    while ( idx_note != head_notes) {
      if ( ++idx_note >= NOTES_CNT) idx_note=0;
      if ( idx_note != max_idx ) {
        notes[idx_note].level = 0;
        kanal[ notes[idx_note].kanal ].scan_cnt = cfg.scan_cnt+1; // mute fantom channel
        #ifdef SHOW_GROUPS_HIDE
          DBGserial.print("x ");DBGserial.println( notes[idx_note].kanal );
        #endif  
      }
    }
    #ifdef SHOW_GROUPS_HIDE
      DBGserial.println();
    #endif  
    return true; // можно играть  
}

