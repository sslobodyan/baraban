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
  
  DBGserial.begin(MIDI_SPEED);
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

void show_buf_krutilka() {
  DBGserial.println("buf_krutilka > ");
  for (byte i=0; i<KRUTILKI_CNT/2; i++ ) {
    DBGserial.print("["); DBGserial.print(i);DBGserial.print("][0] ");
    DBGserial.print( buf_krutilka[ i ][0] );

    DBGserial.print("\t["); DBGserial.print(i);DBGserial.print("][1] ");
    DBGserial.print( buf_krutilka[ i ][1] );
    DBGserial.println();
  }  
}


void main_loop(){  

  if (adc_new_cycle) { // отсканировали все входы
    adc_new_cycle = false;
    if ( scan_autotreshold ) {
      store_autotreshold(); // набираем максимумы по каналам
      if (  millis() >= cfg.autotreshold_time ) { // прошло время автотрешолда
        scan_autotreshold = false;
        for (byte i=0; i<NUM_CHANNELS; i++) {
          kanal[i].treshold = kanal[i].adc_max + cfg.autotreshold_above;
          kanal[i].velocity1 = kanal[i].treshold + 10;
          kanal[i].adc_max = 0;
          DBGserial.print( kanal[i].treshold );
          DBGserial.print("  ");
        }
        DBGserial.println();
      }
    }
    else {
      store_maximum();
    }  
  }

  MIDI_Master.read();
  MIDI_Slave.read();  

  update_krutilki(); // положение текущей педали-крутилки на центральном модуле
  
  // датчики касания
  // readTouchInputs();

  if (head_notes != tail_notes) { // что-то новое в буфере активных нот
    if ( check_groups() ) { // контроль кросстолка
      tail_notes++;
      if (tail_notes >= NOTES_CNT) tail_notes=0;
      bool flag = note_on(tail_notes); // играть ноту из буфера нажатых нот
      if ( flag ) DBGserial.println();    
    }
  }
  
  for (byte i=0; i<NUM_CHANNELS; i++) { // проверка на время note_off
    if ( kanal[i].noteoff_time ) {
      if (kanal[i].noteoff_time < millis() ) {
        note_off(i);
        kanal[i].noteoff_time = 0;
      }
    }
  }

  if (( cfg.metronom > 0) && (cfg.metronom_volume > 1)) {
    if ( millis() - old_metronom >= cfg.metronom ) {
      #define METRONOM_HARD 35
      #define METRONOM_SOFT 25
      old_metronom += cfg.metronom;
      if ( cfg.metronom_krat ) {
        if ( ++metronom_krat == cfg.metronom_krat ) { // сильная доля
          metronom_krat = 0;
          MIDI_Master.sendNoteOn( kanal[cfg.metronom_kanal].note , cfg.metronom_volume, DRUMS);      
          kanal[cfg.metronom_kanal].noteoff_time = millis() + METRONOM_HARD;
          RED_ON;
        } else { // слабые доли
          MIDI_Master.sendNoteOn( kanal[cfg.metronom_kanal-1].note , cfg.metronom_volume, DRUMS);           
          kanal[cfg.metronom_kanal-1].noteoff_time = millis() + METRONOM_SOFT;
          GREEN_ON;
        }
      } else {
        MIDI_Master.sendNoteOn( kanal[cfg.metronom_kanal].note , cfg.metronom_volume, DRUMS); 
        kanal[cfg.metronom_kanal].noteoff_time = millis() + METRONOM_HARD;
        RED_ON;
      }    
    }
  }
  
}

void loop() {
  scan_autotreshold = true;
  DBGserial.println("Gather noise.. ");

  setup_new_scan(); // запускаем опрос в работу
  
  //stop_scan = true; // ToDo debug

  while (1) main_loop();
}
