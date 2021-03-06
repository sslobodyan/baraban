/*
Принимаемые команды:

0x01_ Назначить тип крутилки на аналоговый вход и ее параметры
      0x01
      номер_входа (127-все)
      тип_крутилки(0,PEDAL_SUSTAIN,PEDAL_VOICE...)
      velocity1 старший, младший
      velocity127 старший, младший
      gist
      show
0x02_ Задать тип вывода информации сработавшего канала
      0x02
      номер_входа (127-все)
      состояние (0-молчать,1-вывод уровня выше трешолда)
0x03_ Scan Cnt (время ожидания максимума с пьеза после превышения порога, количество сканирований)
      0x03
      количество
0x04_ Mute Cnt (время успокоения пьеза после удара, количество сканирований*8)
      0x03
      количество
0x05_ Задать тип вывода информации сработавшей крутилки
      0x05
      номер_входа (127-все)
      состояние (0-молчать,1-вывод текущего уровня)
0x06_ Задать параметры для пьезо-входа 
      0x06
      номер_входа (127-все)
      уровень трешолда старших 7 бит, уровень трешолда младших 7 бит
      velocity1 старших 7 бит, velocity1 младших 7 бит
      velocity127 старших 7 бит, velocity127 младших 7 бит
      нота
      группа
      show
0x09_ Максимальный номер программы
      0x09
      максимальный номер      
0x0A_ CrossTalk Cnt (время ожидания фантомных ударов, количество сканирований)
      0x0A
      количество
0x0B_ Группа для CrossTalk
      0x0B
      номер_входа (127-все)
      номер группы 
0x0C_ Задать номер ноты для канала
      0x0C
      номер_входа 
      нота 
0x0D_ Задать темп метронома в ударах в минуту, громкость и  кратность долей такта
      0x0D
      темп старший, младший
      громкость
      кратность
      enable
0x0E_ Запросить параметры аналогового входа по номеру
      0x0E
      номер входа (127 - все)
0x0F_ Запросить параметры крутилки по номеру
      0x0F
      номер крутилки (127 - все)
0x12_ Показывать отладку в порт DBG
      0x12
      0-отключить, 1-включить
0x13_ EEPROM      
      0x13
      0-считать, 1-записать, 2-version, 13-инит модуля, (72,60,48,36) - задать тип модуля, 20 - dbg_on, 21-dbg_off, 22-metro_enable, 23-metro_disable
0x14_ Запуск автотрешолда
      0x14
      127-запустить
0x15_ Запрос конфигурации (cfg)
      0x15
      127-заппрос

///////////////////////////////////////////////////////////////
Отсылаемые команды:

0x07_ Текущий уровень с пьезо-входа 
      номер_модуля 
      0x07
      номер_входа 
      уровень старших 7 бит
      уровень младших 7 бит
0x08_ Текущий уровень с крутилки-педали
      номер_модуля
      0x08
      номер_входа
      уровень крутилки
      уровень АЦП старший
      уровень АЦП младший      
0x10_ Возврат параметров аналогового входа
      номер модуля
      0x10
      номер входа
      порог старший, порог младший
      номер ноты
      velocity1 старший, velocity1 младший
      velocity127 старший, velocity127 младший
      группа
      show
      глобальный velocity1 старший, velocity1 младший
      глобальный velocity127 старший, velocity127 младший
0x11_ Возврат параметров крутилки входа
      номер модуля
      0x11
      номер входа
      текущее АЦП старший, текущее АЦП младший
      value
      velocity1 старший, velocity1 младший
      velocity127 старший, velocity127 младший
      гистерезис
      тип крутилки
      show
0x16_ Возврат конфигурации cfg
      noteoff_time0 = 500; // ms звучания ноты без педали
      noteoff_time1 = 2500; // ms звучания ноты с полупедалью
      pedal = 0; // состояние педали sustain 0 - 63 - 127
      pedal_voice = 0; // состояние педали для добавления к номеру голоса
      voice = 0; // текущий номер голоса
      pedal_octave = PEDAL_CENTER; // состояние педали сдвига октавы (12==без сдвига)
      pedal_program = PEDAL_CENTER; // состояние педали номера программы (0-отпущена, 1-вниз, 2-вверх)
      curr_program = 0; // текущий номер программы
      volume=100;
      cross_cnt = 6; // сколько опросов АЦП ждать кросстолк (1 опрос 133 мкс)
      velocity1 = 0;
      velocity127 = 0;
      scan_cnt = 12; // 6==800us количество полных сканирований АЦП после превышения трешолда
      mute_cnt = 560; // количество полных сканирований АЦП для игнора успокаивающегося датчика
      metronom = 500; // 500 == 60000 / 120  интервал в миллисекундах, если 0 - то молчим
      metronom_volume = 0; // громкость метронома
      metronom_kanal = NUM_CHANNELS-1; // канал метронома
      metronom_krat = 4; // кратность долей метронома
      cross_percent = 30; // подавлять кросстолк до указанного уровня в процентах от самого сильного сигнала
      max_level = 1800; // уровень сигнала, когда включаем красный светодиод
      pedal_metronom1 = PEDAL_CENTER; // изменение метронома на 1 bps
      pedal_metronom10 = PEDAL_CENTER; // изменение метронома на 10 bps
0x17  Version      
      номер модуля
      0x17
      строка с версией
      
//
0xF0
0x7D - non commercial
72 - ID (OR 127 for all)
data bytes
0xF7
//

*/

#define SYSEX_ID 0x7D

void set_type_krutilka_01(byte * array, unsigned array_size) { // 0x01 Назначить тип крутилки на аналоговый вход
  if (array[4] == 127) {
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      krutilka_set_type(i, array[5]);
      krutilka[i].velocity1 = array[6] << 7 | array[7];
      krutilka[i].velocity127 = array[8] << 7 | array[9];
      krutilka[i].gist = array[10];
      krutilka[i].show = array[11];
    }
  } else {
    if ( array[4] < KRUTILKI_CNT ) {
      krutilka_set_type(array[4], array[5]);
      krutilka[array[4]].velocity1 = array[6] << 7 | array[7];
      krutilka[array[4]].velocity127 = array[8] << 7 | array[9];
      krutilka[array[4]].gist = array[10];
      krutilka[array[4]].show = array[11];
    }
  }
}

void set_show_analog_02(byte * array, unsigned array_size) { // 0x02 Состояние вывода информации сработавшего канала
      // состояние (0-молчать, 1-вывод уровня при сработке, 2-вывод текущего уровня)
  if (array[4] == 127) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      kanal[i].show = array[5];
    }
  } else {
    if ( array[4] < NUM_CHANNELS ) kanal[ array[4] ].show = array[5];
  }      
}

void set_show_krutilka_05(byte * array, unsigned array_size) { // 0x05 Состояние вывода информации сработавшей крутилки
  if (array[4] == 127) {
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      krutilka[i].show = array[5];
    }
  } else {
    if ( array[4] < KRUTILKI_CNT ) krutilka[ array[4] ].show = array[5];
  }
}

void send_sysex_krutilka_08(uint8_t idx) { // выслать состояние крутилки-педали
/*
0x08 Текущий уровень с крутилки-педали
      номер_модуля
      0x08
      номер_входа
      уровень крутилки
      уровень АЦП старший
      уровень АЦП младший
*/  
  byte arr[]={SYSEX_ID, cfg.module, 0x08, idx, krutilka[idx].value, krutilka[idx].adc >> 7, krutilka[idx].adc & 0b01111111};
  send_SysEx(sizeof(arr), arr);
}

void send_sysex_kanal_07(uint8_t idx, uint16_t level) { // Текущий уровень с пьезо-входа 
/*  
      номер_модуля 
      0x07
      номер_входа 
      уровень старших 7 бит
      уровень младших 7 бит
*/  
  byte arr[]={SYSEX_ID, cfg.module, 0x07, idx, level >> 7, level & 0b01111111};
  send_SysEx(sizeof(arr), arr);
}

void set_scan_cnt_03(byte * array, unsigned array_size) { 
    cfg.scan_cnt = array[4];
}

void set_mute_cnt_04(byte * array, unsigned array_size) { 
    cfg.mute_cnt = (uint16_t) array[4]*8;
}

void set_treshold_06(byte * array, unsigned array_size) { // 0x06 Задать уровень порога для пьезо-входа (treshold)
  if (array[4] == 127) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      kanal[i].treshold = array[5] << 7 | array[6];
      kanal[i].velocity1 = array[7] << 7 | array[8];
      kanal[i].velocity127 = array[9] << 7 | array[10];
      kanal[i].note = array[11];
      kanal[i].group = array[12];
      kanal[i].show = array[13];
    }
  } else {
    if ( array[4] < NUM_CHANNELS ) {
      kanal[ array[4] ].treshold = array[5] << 7 | array[6];
      kanal[ array[4] ].velocity1 = array[7] << 7 | array[8];
      kanal[ array[4] ].velocity127 = array[9] << 7 | array[10];
      kanal[ array[4] ].note = array[11];
      kanal[ array[4] ].group = array[12];
      kanal[ array[4] ].show = array[13];
    }
  }      
}

void set_max_program_09(byte * array, unsigned array_size) { //0x09 Максимальный номер программы
    cfg.max_program = array[4];
}

void set_cross_cnt_0A(byte * array, unsigned array_size) { //0x0A CrossTalk Cnt (время ожидания фантомных ударов, количество сканирований)
    cfg.cross_cnt = array[4];
}

void set_group_0B(byte * array, unsigned array_size) { // 0x0B Группа для CrossTalk
  if (array[4] == 127) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      kanal[i].group = array[5];
    }
  } else {
    if ( array[4] < NUM_CHANNELS ) kanal[ array[4] ].group = array[5];
  }      
}

void set_note_0C(byte * array, unsigned array_size) { // 0x0C Номер ноты в канале
  if ( array[4] < NUM_CHANNELS ) kanal[ array[4] ].note = array[5];
}

void set_metronome_0D(byte * array, unsigned array_size) { // 0x0D_ Задать темп метронома в ударах в минуту, громкость и кратность
  RED_OFF;
  GREEN_OFF;
  uint16_t m = array[4] << 7 + array[5];
  if ( m == 0 ) {
    cfg.metronom = 0;
    return;
  }
  cfg.metronom = (uint32_t) 60000 / m;
  cfg.metronom_volume = array[6] & 0x7F;
  cfg.metronom_krat = array[7] & 0x0F;
  cfg.metronom_enable = array[8];
  metronom_krat = cfg.metronom_krat - 1; // начинаем всегда с сильной доли
  old_metronom = millis() - cfg.metronom;
}

void send_analog_params_10( byte idx ) {
/*
номер модуля, 0x10, номер входа
порог старший, порог младший
номер ноты
velocity1 старший, velocity1 младший
velocity127 старший, velocity127 младший
группа
show
глобальный velocity1 старший, velocity1 младший
глобальный velocity127 старший, velocity127 младший
*/  
  byte arr[]={SYSEX_ID, 
  cfg.module, 0x10, idx, 
  kanal[idx].treshold >> 7 & 0b01111111 , kanal[idx].treshold & 0b01111111,
  kanal[idx].note,
  kanal[idx].velocity1 >> 7 & 0b01111111, kanal[idx].velocity1 & 0b01111111,
  kanal[idx].velocity127 >> 7 & 0b01111111, kanal[idx].velocity127 & 0b01111111,
  kanal[idx].group,
  kanal[idx].show,
  cfg.velocity1 >> 7 & 0b01111111, cfg.velocity1 & 0b01111111,
  cfg.velocity127 >> 7 & 0b01111111, cfg.velocity127 & 0b01111111
  };
  send_SysEx(sizeof(arr), arr);  
  delay(2);  // ToDo пока просто ждем время, но надо контролировать буфер передачи
}

void get_analog_params_0E(byte * array, unsigned array_size) { //  Запросить параметры аналогового входа по номеру
  if ( array[4] == 0x7F ) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      send_analog_params_10( i );  
    }
  } else 
  if ( array[4] < NUM_CHANNELS ) send_analog_params_10( array[4] );
}

void send_krutilka_params_11( byte idx ) {
/*
номер модуля, 0x11, номер входа
такущее АЦП, value
velocity1 старший, velocity1 младший
velocity127 старший, velocity127 младший
гистерезис
*/  
  byte arr[]={SYSEX_ID,
  cfg.module, 0x11, idx, 
  krutilka[idx].adc >> 7, krutilka[idx].adc & 0b01111111, 
  krutilka[idx].value,
  krutilka[idx].velocity1 >> 7, krutilka[idx].velocity1 & 0b01111111,
  krutilka[idx].velocity127 >> 7, krutilka[idx].velocity127 & 0b01111111,
  krutilka[idx].gist,
  krutilka[idx].type,
  krutilka[idx].show
  };
  send_SysEx(sizeof(arr), arr);  
  delay(2);  // ToDo пока просто ждем время, но надо контролировать буфер передачи
}

void get_krutilka_params_0F(byte * array, unsigned array_size) { // Запросить параметры крутилки по номеру
  if ( array[4] == 0x7F ) {
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      send_krutilka_params_11( i );  
    }
  } else 
  if ( array[4] < KRUTILKI_CNT ) send_krutilka_params_11( array[4] );
}

void set_show_debug_12(byte * array, unsigned array_size) { //0x12 cfg.show_debug
    cfg.show_debug = array[4];
}

void set_eprom_13(byte * array, unsigned array_size) { //0x13 0-читать,1-записать конфигурацию в епром, 2-version, 13-init, 72-36 - set type
    switch (array[4]) {
      case 0:
              read_cfg_from_eprom();
              read_krutilka_from_eprom();
              read_kanal_from_eprom();
              if (cfg.show_debug) {
                DBGserial.println("Config restored");
              }
              break;
      case 1:
              save_cfg_to_eprom();
              save_krutilka_to_eprom();
              save_kanal_to_eprom();
              if (cfg.show_debug) {
                DBGserial.println("Config saved");
              }
              break;
      case 2:
              send_version_17();
              break;
      case 13: // 0D
              setup_module();
              fill_notes();
              setup_krutilki();
              cfg.scan_cnt = 12;
              cfg.mute_cnt = 560; // количество полных сканирований АЦП для игнора успокаивающегося датчика
              cfg.cross_percent = 50; // подавлять кросстолк до указанного уровня в процентах от самого сильного сигнала
              cfg.cross_cnt = 6; // сколько опросов АЦП ждать кросстолк (1 опрос 133 мкс)
              if (cfg.show_debug) {
                DBGserial.println("Fill notes");
              }
              break;
      case 20: // 0x14
              cfg.show_debug = true;
              break;
      case 21: // 0x15
              cfg.show_debug = false;
              break;
      case 22: // 0x16
              cfg.metronom_enable = true;
              break;
      case 23: // 0x17
              cfg.metronom_enable = false;
              break;
      case MODULE_72:
              cfg.module = MODULE_72; cfg.start_note = MODULE_72; cfg.end_note = cfg.start_note + NUM_CHANNELS;
              fill_notes();
              break;
      case MODULE_60:
              cfg.module = MODULE_60; cfg.start_note = MODULE_60; cfg.end_note = cfg.start_note + NUM_CHANNELS;
              fill_notes();
              break;
      case MODULE_48:
              cfg.module = MODULE_48; cfg.start_note = MODULE_48; cfg.end_note = cfg.start_note + NUM_CHANNELS;
              fill_notes();
              break;
      case MODULE_36:
              cfg.module = MODULE_36; cfg.start_note = MODULE_36; cfg.end_note = cfg.start_note + NUM_CHANNELS;
              fill_notes();
              break;
      default: ;
    }
}

void set_autotreshold_14(byte * array, unsigned array_size) { //0x14 1-запустить
    if (array[4] == 127) start_autotreshold();
}

void send_config_16() {
  byte metro = map(60000 / cfg.metronom, METRONOME_MIN, METRONOME_MAX, 0, 127);
  byte arr[]={
  SYSEX_ID,
  cfg.module, 0x16, 
  cfg.noteoff_time0 / 100,
  cfg.noteoff_time1 / 100,
  cfg.pedal,
  cfg.pedal_voice,
  cfg.voice,
  cfg.pedal_octave,
  cfg.pedal_program,
  cfg.curr_program,
  cfg.volume,
  cfg.cross_cnt * 16 > 127 ? 127 : cfg.cross_cnt * 16,
  cfg.velocity1 / 8,
  cfg.velocity127 / 8,
  cfg.scan_cnt * 8 > 127 ? 127 : cfg.scan_cnt * 8,
  cfg.mute_cnt / 8,
  metro,
  cfg.metronom_volume,
  cfg.metronom_krat,
  cfg.cross_percent,
  cfg.max_level >> 7, cfg.max_level & 0b1111111,
  cfg.pedal_metronom1,
  cfg.pedal_metronom10
  };
  send_SysEx(sizeof(arr), arr);  
  delay(10);
}

void get_config_15(byte * array, unsigned array_size) { //0x14 1-запустить
    if (array[4] == 127) {
      send_config_16();
    }
}

void send_version_17(){ //0x17  Version  номер модуля, 0x17, строка с версией
  byte arr[20]={
  SYSEX_ID,
  cfg.module, 0x17 
  };
  for (byte i=0; i<sizeof(version); i++) arr[i+3] = version[i];
  send_SysEx(sizeof(version)+3, arr);   
}


///////////////////////////////////////////////////////////////////////////////////////

void sysexHanlerMaster(byte * array, unsigned array_size) {
  if ( array[1] != SYSEX_ID ) return;
  if ( (array[2] != cfg.module) && (array[2] != 0x7F) ) return;
  switch ( array[3] ) {
    case 0x01: set_type_krutilka_01(array,array_size); break;
    case 0x02: set_show_analog_02(array,array_size); break;
    case 0x03: set_scan_cnt_03(array,array_size); break;
    case 0x04: set_mute_cnt_04(array,array_size); break;
    case 0x05: set_show_krutilka_05(array,array_size); break; // Состояние вывода информации сработавшей крутилки
    case 0x06: set_treshold_06(array,array_size); break;
    case 0x09: set_max_program_09(array,array_size); break;
    case 0x0A: set_cross_cnt_0A(array,array_size); break;
    case 0x0B: set_group_0B(array,array_size); break;
    case 0x0C: set_note_0C(array,array_size); break;
    case 0x0D: set_metronome_0D(array,array_size); break;
    case 0x0E: get_analog_params_0E(array,array_size); break; //  Запросить параметры аналогового входа по номеру
    case 0x0F: get_krutilka_params_0F(array,array_size); break; // Запросить параметры крутилки по номеру
    case 0x12: set_show_debug_12(array,array_size); break; // Запросить параметры крутилки по номеру
    case 0x13: set_eprom_13(array,array_size); break; // 0-читать,1-записать конфигурацию в епром
    case 0x14: set_autotreshold_14(array,array_size); break; // 1-запустить
    case 0x15: get_config_15(array,array_size); break; // 127-запустить
    
    default : 
      DBGserial.print("SysEx = 0x");DBGserial.println( array[3], HEX ); // ToDo Debug
      break;
  }
}

void sysexHanlerSlave(byte * array, unsigned array_size) {
  if (cfg.show_debug) {
    DBGserial.print("DBG SysEx = 0x");DBGserial.println( array[3], HEX ); // ToDo Debug
  }
  glo_SysExMaster = false;
  sysexHanlerMaster(array,array_size); 
  glo_SysExMaster = true;
}

