#define START_EPROM_ADDR 1

uint16_t write_eeprom(uint16_t AddressWrite, uint16_t DataWrite) { // записать 16 бит
  return ( EEPROM.write(AddressWrite, DataWrite) );
}

uint16_t read_eeprom(uint16_t AddressWrite ) { // считать 16 бит
  uint16_t Data;
  EEPROM.read(AddressWrite, &Data);
  return ( Data );
}

uint16_t save_cfg_to_eprom(void) { // записать cfg
  uint16_t addr = START_EPROM_ADDR;
  uint16_t data;
  uint16_t* ptr;
  uint16_t stat;
  ptr = (uint16_t*) &cfg;
  for (int i = 0; i < sizeof(cfg) / sizeof(int16_t); i++) {
    data = *ptr;
    stat = write_eeprom( addr++, data);
    ptr++;
  }
  return stat;
}

void read_cfg_from_eprom(void) { // считать в структуру cfg
  uint16_t addr = START_EPROM_ADDR;
  uint16_t data;
  uint16_t* ptr;
  ptr = (uint16_t*) (&cfg);
  for (int i = 0; i < sizeof(cfg) / 2; i++) {
    data = read_eeprom( addr++ );
    *(ptr++) = data;
  }
}

uint16_t save_krutilka_to_eprom(void) { // записать cfg
  uint16_t addr = START_EPROM_ADDR + sizeof(cfg)/2 + 1;
  uint16_t data;
  uint16_t* ptr;
  uint16_t stat;
  ptr = (uint16_t*) krutilka;
  for (int i = 0; i < sizeof(krutilka) / sizeof(int16_t); i++) {
    data = *ptr;
    stat = write_eeprom( addr++, data);
    ptr++;
  }
  return stat;
}

void read_krutilka_from_eprom(void) { // считать в структуру cfg
  uint16_t addr = START_EPROM_ADDR + sizeof(cfg)/2 + 1;
  uint16_t data;
  uint16_t* ptr;
  ptr = (uint16_t*) krutilka;
  for (int i = 0; i < sizeof(krutilka) / 2; i++) {
    data = read_eeprom( addr++ );
    *(ptr++) = data;
  }
}

void init_flash(void) {
  EEPROM.format();
  write_eeprom(0, 1302);
  save_cfg_to_eprom();
  save_krutilka_to_eprom();
}

bool check_eprom_inited() {
  if ( read_eeprom(0) == 1302 ) {
    return true;
  } else return false;
}

uint16_t setup_eeprom(void) {
  EEPROM.PageSize = 0x400; // 1024
  EEPROM.PageBase0 = 0x8000000 + 126*EEPROM.PageSize;    //0x801F800;
  EEPROM.PageBase1 = 0x8000000 + 127*EEPROM.PageSize; // 134217728 + 125*1024 = 134345728 0x801F400
  return ( EEPROM.init() );
}



