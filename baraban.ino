#include "arduino.h"
#include <libmaple/dma.h>
#include <MIDI.h>
#include <Wire.h>

#include "vars.h"
#include "adc.h"
#include "utils.h"
#include "midis.h"
#include "mpr.h"

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
  
  stop_scan = false;
  setup_kanal();
  midiSetup();
  DBGserial.print(" Search MPRs... ");
  DBGserial.print( setup_touch() );
  DBGserial.println();
}

void main_loop(){  
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
  scan_autotreshold = false;
}

void loop() {
  scan_autotreshold = true;
  setup_new_scan(); // запускаем опрос в работу
  set_autotreshold(); // выставляем уровень шума
  while (1) main_loop();
}
