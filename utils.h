
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
    kanal[i].group = 1;
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
  byte idx = head_notes + 1;
  if ( idx >= NOTES_CNT ) idx = 0;
  notes[idx].kanal = ch;
  notes[idx].level = level;
  notes[idx].cross_cnt = adc_dma_cnt + cfg.cross_cnt ;
  notes[idx].group = kanal[ch].group; // группу запомним для бысрой фильтрации в кросстолке
  head_notes = idx;
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



#include "libmaple/usart.h"
void putc_serial3( uint8_t ch ) {
  // отключить прерывание по ТХ, блокирующе передать байт
  while(!(USART3->regs->SR & USART_SR_TXE))
    ;
  USART3->regs->DR = (uint8)ch;
  while(!(USART3->regs->SR & USART_SR_TXE))
    ; 
}

void MIDI_Master_sendNoteOn( byte note , byte vel, byte chan){
  putc_serial3( 0x90 - 1 + ( chan & 0x0F ) );
  putc_serial3( note & 0x7F );
  putc_serial3( vel  & 0x7F );
}

void MIDI_Master_sendNoteOff( byte note , byte vel, byte chan){
  putc_serial3( 0x80 - 1 + ( chan & 0x0F) );
  putc_serial3( note & 0x7F );
  putc_serial3( vel  & 0x7F );
}

void MIDI_Master_sendControlChange( byte note , byte vel, byte chan) {
  putc_serial3( 0xB0 - 1 + ( chan & 0x0F ) );
  putc_serial3( note & 0x7F );
  putc_serial3( vel  & 0x7F );  
}

