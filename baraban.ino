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
  
  DBGserial.begin(115200);
  DBGserial.print("Started... ");
    
  setup_ADC();
  setup_DMA();

  pinMode(LED, OUTPUT);
  LED_OFF;
  pinMode(GREEN_LED, OUTPUT);
  GREEN_OFF;
  pinMode(RED_LED, OUTPUT);
  RED_OFF;

  setup_module();
  
  stop_scan = false;
  setup_kanal();
  setup_krutilki();
  midiSetup();
  DBGserial.print(" Search MPRs... ");
  DBGserial.print( setup_mpr() );
  DBGserial.println();
  setup_touch(); // распределение датчиков касания по каналах
}

void setup_module() { // по перемычкам определить тип модуля и соотв. назначить номера нот  
  pinMode(SELECT_MODULE1, INPUT_PULLUP);
  pinMode(SELECT_MODULE2, INPUT_PULLUP);
  if ( digitalRead(SELECT_MODULE1) ) {
    if ( digitalRead(SELECT_MODULE2) ) {
      cfg.module = MODULE_72;
    } else {
      cfg.module = MODULE_60;
    }
  } else {
    if ( digitalRead(SELECT_MODULE2) ) {
      cfg.module = MODULE_48;
    } else {
      cfg.module = MODULE_36;
    }    
  }
}

void main_loop(){  

#ifdef DEBUG_USART
  if ( millis() > tm_time ) {
    #define INST 38
    #define TEST_CH 20
    kanal[TEST_CH].note = INST;
    tm_time =  200 + random(200) + millis();
    MIDI_Master.sendNoteOn( kanal[TEST_CH].note , 120, DRUMS);      
    kanal[TEST_CH].noteoff_time = millis() + 200;
  }
#endif

  update_krutilki(); // положение текущей педали-крутилки на центральном модуле
  
  // датчики касания
  // readTouchInputs();

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

//  if (MIDIserial.available()) {
//    MIDIserial.print("In buffer 0x");
//    MIDIserial.print( MIDIserial.peek(), HEX );
//    MIDIserial.println(" byte");
//  }

  MIDI_Master.read();
  MIDI_Slave.read();
  
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
