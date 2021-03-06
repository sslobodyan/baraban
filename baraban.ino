#include "arduino.h"
#include <libmaple/dma.h>
#include <MIDI.h>
#include <Wire.h>
#include <EEPROM.h>


#include "vars.h"
#include "adc.h"
#include "utils.h"
#include "eprom.h"
#include "sysex.h"
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
  pinMode(RED_LED, OUTPUT);
  GREEN_ON;
  RED_ON;

  delay(100);

  GREEN_OFF;
  RED_OFF;

  setup_module();
  byte pin_module = cfg.module; // запоминаем тип модуля по перемычкам 
  
  stop_scan = false;
  setup_kanal();
  setup_krutilki();
  midiSetup();

  multi_krutilka_idx = 0;
  multi_idx = 0;

/*  
  DBGserial.print(" Search MPRs... ");
  DBGserial.print( setup_mpr() );
  DBGserial.println();
  
  setup_touch(); // распределение датчиков касания по каналах
*/  

  setup_eeprom();
  DBGserial.print("Init");
  if ( check_eprom_inited() ){
    DBGserial.println("ed OK");
  } else {
    DBGserial.println(" -");
    init_flash();
  }
  DBGserial.print("Size ");
  DBGserial.print( sizeof(cfg)/2 );
  DBGserial.print(" / ");
  DBGserial.print( sizeof(kanal) );
  DBGserial.print(" / ");
  DBGserial.println( sizeof(krutilka)/2 );
  read_cfg_from_eprom();

  read_krutilka_from_eprom();
  read_kanal_from_eprom();

  if (pin_module != cfg.module) {
    // запомненный тип не совпадает с перемычками - надо сменить ноты
    setup_module();
    fill_notes(); 
  }

  send_version_17();
  
  DBGserial.println("Restored");
  
}

void setup_module() { // по перемычкам определить тип модуля и соотв. назначить номера нот  
  pinMode(SELECT_MODULE1, INPUT_PULLUP);
  pinMode(SELECT_MODULE2, INPUT_PULLUP);
  if ( digitalRead(SELECT_MODULE1) ) { // разомкнут
    if ( digitalRead(SELECT_MODULE2) ) { // разомкнут
      cfg.module = MODULE_72;
      cfg.start_note = MODULE_72; 
      cfg.end_note = cfg.start_note + 32; // ToDo 25 ?
      DBGserial.println( "Module_72" );
    } else {
      cfg.module = MODULE_60;
      cfg.start_note = MODULE_60; 
      cfg.end_note = cfg.start_note + 32; // ToDo 25 ?
      DBGserial.println( "Module_60" );
    }
  } else {
    if ( digitalRead(SELECT_MODULE2) ) { // разомкнут
      cfg.module = MODULE_48;
      cfg.start_note = MODULE_48; 
      cfg.end_note = cfg.start_note + 32; // ToDo 25 ?
      DBGserial.println( "Module_48" );
    } else {
      cfg.module = MODULE_36;
      cfg.start_note = MODULE_36; 
      cfg.end_note = cfg.start_note + 32; // ToDo 25 ?
      DBGserial.println( "Module_36" );
    }    
  }
  DBGserial.print( "Start Note " );
  DBGserial.print(cfg.start_note);
  DBGserial.print( "   End Note " );
  DBGserial.println(cfg.end_note);
  DBGserial.println();
  delay(30);
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
      if (  millis() - time_autotreshold >= cfg.autotreshold_time ) { // прошло время автотрешолда
        scan_autotreshold = false;
        RED_OFF;
        // вывод набранных порогов в отладку
        for (byte i=0; i<NUM_CHANNELS; i++) {
          kanal[i].treshold = kanal[i].adc_max + cfg.autotreshold_above;
          kanal[i].velocity1 = kanal[i].treshold + 50;
          if (i % 4 == 0) DBGserial.println();
          DBGserial.print("("); 
          if (i < 10) DBGserial.print(" ");          
          DBGserial.print( i ); 
          DBGserial.print("/"); 
          if (kanal[i].note < 10) DBGserial.print(" ");
          if (kanal[i].note < 100) DBGserial.print(" ");
          DBGserial.print( kanal[i].note ); 
          DBGserial.print(")");
          DBGserial.print("n="); 
          if (kanal[i].adc_max < 10) DBGserial.print(" ");
          if (kanal[i].adc_max < 100) DBGserial.print(" ");          
          if (kanal[i].adc_max < 1000) DBGserial.print(" ");          
          DBGserial.print( kanal[i].adc_max ); 
          DBGserial.print(",t="); 
          if (kanal[i].treshold < 10) DBGserial.print(" ");
          if (kanal[i].treshold < 100) DBGserial.print(" ");          
          if (kanal[i].treshold < 1000) DBGserial.print(" ");          
          DBGserial.print( kanal[i].treshold );
          DBGserial.print("\t");
          kanal[i].adc_max = 0;
        }
        DBGserial.println();
        show_krutilki_adc();
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
      if ( flag ) {
        if (cfg.show_debug) {
          DBGserial.println();    
        }
      }
    }
  }
  
  for (byte i=0; i<NUM_CHANNELS; i++) { // проверка на время note_off
    if ( kanal[i].noteoff_time ) {
      if (kanal[i].noteoff_time < millis() ) {
        note_off(i);
        kanal[i].noteoff_time = 0;
        // гасим метроном
        if ( i == NUM_CHANNELS-1 ) {
          if (( cfg.metronom > 0) && (cfg.metronom_volume >=  5)) {
            RED_OFF;  
          }
        }
        if ( i == NUM_CHANNELS-2 ) {
          if (( cfg.metronom > 0) && (cfg.metronom_volume >= 5)) {
            GREEN_OFF;  
          }
        }
      }
    }
  }

  if ( cfg.metronom_enable && ( cfg.metronom > 0) && (cfg.metronom_volume >= 5)) {
    if ( millis() - old_metronom >= cfg.metronom ) {
      #define METRONOM_HARD 35
      #define METRONOM_SOFT 25
      if ( millis() - old_metronom >= cfg.metronom * 2) {
        // долго не играли - стартуем отсюда
        old_metronom = millis();
      } else { // продолжаем - синхронизацию не нарушаем
        old_metronom += cfg.metronom;
      }
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
  } else { // без метронома светики показывают силу удара
    if ((time_green > 0) && (millis() > time_green)) { // тушим светодиод удара
      GREEN_OFF;
      time_green = 0;
    }
    if ((time_red > 0) && (millis() > time_red)) { // тушим светодиод переусиления
      RED_OFF;
      time_red = 0;
    }    
  }
  
  //show_krutilki();
  //show_krutilki_buf();
  
}

void loop() {

  setup_new_scan(); // запускаем опрос в работу

  while (1) main_loop();
}
