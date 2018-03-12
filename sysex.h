/*
Принимаемые команды:

0x01_ Назначить тип крутилки на аналоговый вход
      0x01
      номер_входа (127-все)
      тип_крутилки(0,PEDAL_SUSTAIN,PEDAL_VOICE...)
0x02_ Задать тип вывода информации сработавшего канала
      0x02
      номер_входа (127-все)
      состояние (0-молчать,1-вывод уровня выше трешолда,2-вывод текущего уровня)
0x03_ Scan Cnt (время ожидания максимума с пьеза после превышения порога, количество сканирований)
      0x03
      количество
0x04_ Mute Cnt (время успокоения пьеза после удара, количество сканирований*2)
      0x03
      количество
0x05_ Задать тип вывода информации сработавшей крутилки
      0x05
      номер_входа (127-все)
      состояние (0-молчать,1-вывод текущего уровня)
0x06_ Задать уровень порога для пьезо-входа (treshold)
      0x06
      номер_входа (127-все)
      уровень трешолда старших 7 бит
      уровень трешолда младших 7 бит
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
      темп
      громкость
      кратность
0x0E_ Запросить параметры аналогового входа по номеру
      0x0E
      номер входа (127 - все)
0x0F_ Запросить параметры крутилки по номеру
      0x0E
      номер крутилки (127 - все)

///////////////////////////////////////////////////////////////
Отсылаемые команды:

0x07_ Текущий уровень с пьезо-входа 
      0x07
      номер_модуля 
      номер_входа 
      уровень старших 7 бит
      уровень младших 7 бит
0x08_ Текущий уровень с крутилки-педали
      0x08
      номер_модуля
      номер_входа
      уровень крутилки
0x10_ Возврат параметров аналогового входа
      номер модуля, номер входа
      порог старший, порог младший
      номер ноты
      velocity1 старший, velocity1 младший
      velocity127 старший, velocity127 младший
      группа
0x11  Возврат параметров крутилки входа
      номер модуля, номер входа
      такущее АЦП, value
      velocity1 старший, velocity1 младший
      velocity127 старший, velocity127 младший
      гистерезис
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
    }
  } else {
    if ( array[4] < KRUTILKI_CNT ) krutilka_set_type(array[4], array[5]);
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
      0x08
      номер_модуля
      номер_входа
      уровень крутилки
*/  
  byte arr[]={SYSEX_ID, 0x08, cfg.module, idx, krutilka[idx].value};
  send_SysEx(sizeof(arr), arr);
}

void send_sysex_kanal_07(uint8_t idx, uint16_t level) { // Текущий уровень с пьезо-входа 
/*  
      0x07
      номер_модуля 
      номер_входа 
      уровень старших 7 бит
      уровень младших 7 бит
*/  
  byte arr[]={SYSEX_ID, 0x07, cfg.module, idx, level >> 7, level & 0b01111111};
  send_SysEx(sizeof(arr), arr);
}

void set_scan_cnt_03(byte * array, unsigned array_size) { 
    cfg.scan_cnt = array[4];
}

void set_mute_cnt_04(byte * array, unsigned array_size) { 
    cfg.mute_cnt = array[4];
}

void set_treshold_06(byte * array, unsigned array_size) { // 0x06 Задать уровень порога для пьезо-входа (treshold)
  if (array[4] == 127) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      kanal[i].treshold = array[5] << 7 | array[6];
    }
  } else {
    if ( array[4] < NUM_CHANNELS ) kanal[ array[4] ].treshold = array[5] << 7 | array[6];
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
  if ( array[4] == 0 ) {
    cfg.metronom = 0;
    return;
  }
  cfg.metronom = (uint32_t) 60000 / array[4];
  cfg.metronom_volume = array[5] & 0x7F;
  cfg.metronom_krat = array[6] & 0x0F;
  metronom_krat = cfg.metronom_krat - 1; // начинаем всегда с сильной доли
}

void send_analog_params_10( byte idx ) {
/*
номер модуля, номер входа
порог старший, порог младший
номер ноты
velocity1 старший, velocity1 младший
velocity127 старший, velocity127 младший
группа
*/  
  byte arr[]={SYSEX_ID, 0x10, 
  cfg.module, idx, 
  kanal[idx].treshold >> 7, kanal[idx].treshold & 0b01111111,
  kanal[idx].note,
  kanal[idx].velocity1 >> 7, kanal[idx].velocity1 & 0b01111111,
  kanal[idx].velocity127 >> 7, kanal[idx].velocity127 & 0b01111111,
  kanal[idx].group
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
номер модуля, номер входа
такущее АЦП, value
velocity1 старший, velocity1 младший
velocity127 старший, velocity127 младший
гистерезис
*/  
  byte arr[]={SYSEX_ID, 0x11, 
  cfg.module, idx, 
  krutilka[idx].adc, krutilka[idx].value,
  krutilka[idx].velocity1 >> 7, krutilka[idx].velocity1 & 0b01111111,
  krutilka[idx].velocity127 >> 7, krutilka[idx].velocity127 & 0b01111111,
  krutilka[idx].gist
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
    default : 
      DBGserial.print("SysEx = 0x");DBGserial.println( array[3], HEX ); // ToDo Debug
      break;
  }
}

void sysexHanlerSlave(byte * array, unsigned array_size) {
}

