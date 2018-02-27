/*
0x01 Назначить тип крутилки на аналоговый вход
      0x01
      номер_входа (127-все)
      тип_крутилки(0,PEDAL_SUSTAIN,PEDAL_VOICE...)
0x02 Состояние вывода информации сработавшего канала
      0x02
      номер_входа (127-все)
      состояние (0-молчать,1-вывод уровня выше трешолда,2-вывод текущего уровня)
0x03 Scan Time (время ожидания максимума с пьеза после превышения порога, мкс)
      0x03
      старших 7 бит
      младших 7 бит (итого 14 бит - до 16535 мкс)
0x04 Mute Time (время успокоения пьеза после удара, мс)
      0x03
      старших 7 бит
      младших 7 бит (итого 14 бит - до 16535 мс)
0x05 Состояние вывода информации сработавшей крутилки
      0x05
      номер_входа (127-все)
      состояние (0-молчать,1-вывод текущего уровня)
0x06 Задать уровень порога для пьезо-входа (treshold)
      0x06
      номер_входа (127-все)
      уровень трешолда старших 7 бит
      уровень трешолда младших 7 бит
0x07 Текущий уровень с пьезо-входа 
      0x07
      номер_модуля 
      номер_входа 
      уровень старших 7 бит
      уровень младших 7 бит
0x08 Текущий уровень с крутилки-педали
      0x08
      номер_модуля
      номер_входа
      уровень крутилки
0x09 Максимальный номер программы
      0x08
      максимальный номер      

//
0xF0
0x7D - non commercial
72 - ID (OR 127 for all)
data bytes
0xF7
//

*/

#define SYSEX_ID 0x7D

void set_type_krutilka(byte * array, unsigned array_size) { // 0x01 Назначить тип крутилки на аналоговый вход
  if (array[4] == 127) {
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      krutilka_set_type(i, array[5]);
    }
  } else {
    if ( array[4] < KRUTILKI_CNT ) krutilka_set_type(array[4], array[5]);
  }
}

void set_show_analog(byte * array, unsigned array_size) { // 0x02 Состояние вывода информации сработавшего канала
      // состояние (0-молчать, 1-вывод уровня при сработке, 2-вывод текущего уровня)
  if (array[4] == 127) {
    for (byte i=0; i<NUM_CHANNELS; i++) {
      kanal[i].show = array[5];
    }
  } else {
    if ( array[4] < NUM_CHANNELS ) kanal[ array[4] ].show = array[5];
  }      
}

void set_show_krutilka(byte * array, unsigned array_size) { // 0x05 Состояние вывода информации сработавшей крутилки
  if (array[4] == 127) {
    for (byte i=0; i<KRUTILKI_CNT; i++) {
      krutilka[i].show = array[5];
    }
  } else {
    if ( array[4] < KRUTILKI_CNT ) krutilka[ array[4] ].show = array[5];
  }
}

void send_sysex_krutilka(uint8_t idx) { // выслать состояние крутилки-педали
/*
0x08 Текущий уровень с крутилки-педали
      0x08
      номер_модуля
      номер_входа
      уровень крутилки
*/  
  byte arr[]={SYSEX_ID, cfg.module, idx, krutilka[idx].value};
  send_SysEx(sizeof(arr), arr);
}



///////////////////////////////////////////////////////////////////////////////////////

void sysexHanlerMaster(byte * array, unsigned array_size) {
  if ( array[1] != SYSEX_ID ) return;
  if ( (array[2] != cfg.module) && (array[2] != 127) ) return;
  switch ( array[3] ) {
    case 0x01: set_type_krutilka(array,array_size); break;
    case 0x05: set_show_krutilka(array,array_size); break; // Состояние вывода информации сработавшей крутилки
    default : break;
  }
  DBGserial.print("SysEx = 0x");DBGserial.println( array[3], HEX ); // ToDo Debug
}

void sysexHanlerSlave(byte * array, unsigned array_size) {
}

