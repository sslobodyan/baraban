#include "arduino.h"
#include <libmaple/dma.h>
#include <MIDI.h>
#include <Wire.h>

#include "vars.h"
#include "adc.h"
#include "utils.h"
#include "midis.h"
#include "mpr.h"
#include "krutilki.h"

void setup() {
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); // relase PC3 and PC5 
  
  DBGserial.begin(230400);
  DBGserial.print("Started... ");
    
  setup_ADC();
  setup_DMA();

  pinMode(LED, OUTPUT);
  LED_OFF;
  pinMode(GREEN_LED, OUTPUT);
  GREEN_OFF;
  pinMode(RED_LED, OUTPUT);
  RED_OFF;

  pinMode(PC15, OUTPUT);
  digitalWrite(PC15, HIGH);
  
  stop_scan = false;
  setup_kanal();
  setup_krutilki();
  midiSetup();
  DBGserial.print(" Search MPRs... ");
  DBGserial.print( setup_touch() );
  DBGserial.println();
}

void main_loop(){  

  update_krutilki(); // положение педалей-крутилок
  // датчики касания

  while (head_notes != tail_notes) { // играть ноты из буфера нажатых нот
    if (++tail_notes >= NOTES_CNT) tail_notes=0;
    note_on(tail_notes);
  }
  
  for (byte i=0; i<NUM_CHANNELS; i++) { // проверка на время note_off
    if ( kanal[i].noteoff_time ) {
      if (kanal[i].noteoff_time < millis() ) {
        note_off(i);
        kanal[i].noteoff_time = 0;
      }
    }
  }

  MIDI.read();

  if ( millis() > 5000 ) digitalWrite(PC15, LOW); // debug
  
}

void set_autotreshold(){
  DBGserial.print("Gather noise.. ");
  delay (cfg.autotreshold_time);
  for (byte i=0; i<NUM_CHANNELS; i++) {
    kanal[i].treshold = kanal[i].adc_max + cfg.autotreshold_above;    
    DBGserial.print( kanal[i].treshold );
    DBGserial.print("  ");
    kanal[i].adc_max = 0;
  }
  DBGserial.println();
  DBGserial.println("ADC2 > ");
  for (byte i=0; i<NUM_MULTIPLEXORS; i++ ) {
    DBGserial.print( buf_krutilka[ i ][0] );  DBGserial.print("\t"); DBGserial.println( buf_krutilka[ i ][1] );
  }
  DBGserial.println();
  scan_autotreshold = false;
}

void loop() {
  scan_autotreshold = true;
  setup_new_scan(); // запускаем опрос в работу
  set_autotreshold(); // выставляем уровень шума
  while (1) main_loop();
}
