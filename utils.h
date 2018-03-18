
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
  const byte port[] = {
//    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31   

     24, 0, 8,16, 1,17,25, 9,26, 2,10,18, 3,19,27,11,28, 4,12,20, 5,21,29,13,30, 6,14,22, 7,23,31,15   
      
//    1, 4, 9,12,17,20,25,28, 2, 7,10,15,18,23,26,31,3 ,5 ,11,13,19,21,27,29,0 ,6 ,8 ,14,16,22,24,30 
//   30,24,26,28,0,4,6,214,8,10,12,16,20,22,18,7,1,3,5,17,21,23,19,31,25,27,29,9,13,15,11
  };
  fill_notes();
  DBGserial.println( "Channels:" );
  for (byte i=0; i<NUM_CHANNELS; i++) {
    kanal[i].treshold = 0xFFFF;
    kanal[i].velocity1 = 300;
    kanal[i].velocity127 = 1950;
    kanal[i].scan_cnt = -1;
    kanal[i].group = 1;
    kanal[i].port = port[i]*2+1;
    //kanal[i].port = i*2+1;
    
    if (i<10) DBGserial.print(" ");
    DBGserial.print(i);
    DBGserial.print("/");
    if (((kanal[i].port-1)/2)<10) DBGserial.print(" ");
    DBGserial.print( ((kanal[i].port-1)/2) );
    DBGserial.print("  ");
    if ( i%8 == 7 ) {
      DBGserial.println();
      delay(10);
    }
  }
  DBGserial.println();
}

void setup_touch() {
  for (byte mdl=0; mdl<4; mdl++) {
    for (byte n=0; n<8; n++) {
      touch[mdl].kanal[n] = n;
    }
  }
}

void add_note(byte ch, uint16_t level) { // из прерывания строим кольцевой буфер сработавших каналов
  byte idx = head_notes + 1;
  if ( idx >= NOTES_CNT ) idx = 0;
  notes[idx].kanal = ch;
  notes[idx].level = level;
  notes[idx].cross_cnt = adc_dma_cnt + cfg.cross_cnt ;
  notes[idx].group = kanal[ch].group; // группу запомним для бысрой фильтрации в кросстолке
  head_notes = idx;
  if ((cfg.metronom == 0) || (cfg.metronom_volume <5 )) { // отключен метроном - показываем удары
    GREEN_ON;
    time_green = millis() + 50;
    if (level > cfg.max_level) { // превышен максимальный уровень - надо снизить усиление
      RED_ON;
      time_red = millis() + 50;
    }
  }
}

void show_buf(){ // чисто отладка
  stop_scan = true;
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
  stop_scan = false;
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
byte idx_note=0;

#define SHOW_GROUPS_HIDE_

  if (tail_notes < NOTES_CNT-1) idx_note = tail_notes + 1;

  if ( adc_dma_cnt < notes[ idx_note ].cross_cnt ) return false; // еще не прошло время контроля кросстолка - играть запрещаем

  uint8_t group = notes[ idx_note ].group; // запомним группу этой ноты
  if ( group == 0 ) return true; // группа не установлена - без контроля кроссталка
  
    // ищем самый громкий сигнал
    int max_idx = 0;
    uint16_t max_level = 0;
    idx_note = tail_notes;
    while ( idx_note != head_notes) {
      if ( ++idx_note >= NOTES_CNT) idx_note = 0;
      if ( group != notes[ idx_note ].group ) continue; 
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
    // глушим все более слабые сигналы в той же группе, не превышающие процент
    uint16_t percent = max_level * cfg.cross_percent / 100;
    idx_note = tail_notes;
    while ( idx_note != head_notes) {
      if ( ++idx_note >= NOTES_CNT) idx_note=0;
      if (( idx_note != max_idx ) && (group == notes[ idx_note ].group)) {
        if ( notes[idx_note].level < percent ) {
          notes[idx_note].level = 0;
          kanal[ notes[idx_note].kanal ].scan_cnt = cfg.scan_cnt+1; // mute fantom channel
          kanal[ notes[idx_note].kanal ].adc_max = 0;
          #ifdef SHOW_GROUPS_HIDE
            DBGserial.print("x ");DBGserial.println( notes[idx_note].kanal );
          #endif            
        }
      }
    }
    #ifdef SHOW_GROUPS_HIDE
      DBGserial.println();
    #endif  
    return true; // можно играть  
}

void start_autotreshold() {
  scan_autotreshold = true;
  time_autotreshold = millis();
  DBGserial.println("Gather noise for treshold calculate .. ");
}

