
// MPR121 Register Defines
#define MHD_R  0x2B
#define NHD_R 0x2C
#define NCL_R   0x2D
#define FDL_R 0x2E
#define MHD_F 0x2F
#define NHD_F 0x30
#define NCL_F 0x31
#define FDL_F 0x32

#define NHDT 0x33
#define NCLT 0x34
#define FDLT 0x35


#define ELE0_T  0x41
#define ELE0_R  0x42
#define ELE1_T  0x43
#define ELE1_R  0x44
#define ELE2_T  0x45
#define ELE2_R  0x46
#define ELE3_T  0x47
#define ELE3_R  0x48
#define ELE4_T  0x49
#define ELE4_R  0x4A
#define ELE5_T  0x4B
#define ELE5_R  0x4C
#define ELE6_T  0x4D
#define ELE6_R  0x4E
#define ELE7_T  0x4F
#define ELE7_R  0x50
#define ELE8_T  0x51
#define ELE8_R  0x52
#define ELE9_T  0x53
#define ELE9_R  0x54
#define ELE10_T 0x55
#define ELE10_R 0x56
#define ELE11_T 0x57
#define ELE11_R 0x58

#define DEBOUNCE  0x5B
#define AFE1  0x5C
#define AFE2  0x5D


#define FIL_CFG 0x5D
#define ELE_CFG 0x5E

#define ECR  0x5E

#define GPIO_CTRL0  0x73
#define GPIO_CTRL1  0x74
#define GPIO_DATA 0x75
#define GPIO_DIR  0x76
#define GPIO_EN   0x77
#define GPIO_SET  0x78
#define GPIO_CLEAR  0x79
#define GPIO_TOGGLE 0x7A
#define ATO_CFG0  0x7B
#define ATO_CFGU  0x7D
#define ATO_CFGL  0x7E
#define ATO_CFGT  0x7F

#define SOFTRESET  0x80

// Global Constants
#define TOU_THRESH  6
#define REL_THRESH  12

int irqpin1 = PC14;  
int irqpin2 = PC15;  
int irqpin3 = PB7;  
int irqpin4 = PB6;  

byte mpr_addr[4] = {0x5A,0x5B,0x5C,0x5D};

byte mpr121_setup_registers(byte addr);
boolean checkInterrupt(byte idx);
void set_register(int address, unsigned char r, unsigned char v);

byte setup_mpr(){
  pinMode(irqpin1, INPUT_PULLUP);
  pinMode(irqpin2, INPUT_PULLUP);
  pinMode(irqpin3, INPUT_PULLUP);
  pinMode(irqpin4, INPUT_PULLUP);
  
  Wire.scl_pin = PB8;
  Wire.sda_pin = PB9;
  Wire.i2c_delay = 0; //0-fast   27-standart
  
  Wire.begin();
  byte res=0;
  for (byte i=0; i<sizeof(mpr_addr); i++) {
    res += mpr121_setup_registers(mpr_addr[i]) ;
  }
  return res;
}

uint8_t get_register(uint8_t module, uint8_t reg) {
  Wire.beginTransmission(mpr_addr[module]);
  Wire.write(reg);
  if ( Wire.endTransmission(false) == 0) {
      Wire.requestFrom(mpr_addr[module],1); 
      return Wire.read();
  }
  return 255;
}

void show_regs(uint8_t module) {
  uint8_t res;
  Serial1.println();
  for (uint8_t i=0; i<0x80; i++) {
    res = get_register(module, i);
    Serial1.print("$"); Serial1.print(i, HEX); 
    Serial1.print(": 0x"); Serial1.print(res,HEX);
    Serial1.print("\t"); Serial1.print(res);
    Serial1.print("\t 0b"); Serial1.println(res,BIN);
  }
}

void readTouchInputs(){
  for (byte module=0; module<sizeof(mpr_addr); module++) {
    if ( checkInterrupt(module) ) {  
        Wire.beginTransmission(mpr_addr[module]);
        if ( Wire.endTransmission(false) == 0) {
            Wire.requestFrom(mpr_addr[module],2); 
            byte LSB = Wire.read();
            byte MSB = Wire.read();
            touch[module].touched = ((MSB << 8) | LSB); //запомним состояние входов
            for (int n=0; n < 8; n++){  // проставить признак нажатия
              kanal[ touch[module].kanal[n] ].pressed = touch[module].touched & (1<<n);
            }
        } else {
          // ToDo модуль не ответил
        }
    } //if  
  } //for
}


byte mpr121_setup_registers(byte addr){
  
  Wire.beginTransmission(addr);
  if ( Wire.endTransmission(false) != 0) return 0;

  set_register(addr, ELE_CFG, 0x00); 

///////////////////////////////////

  set_register(addr, SOFTRESET, 0x63);
  delay(2);

  for (uint8_t i=0; i<12; i++) {
    set_register(addr, ELE0_T + 2*i, 15);
    set_register(addr, ELE0_R + 2*i, 10);
  }

  set_register(addr,0x2B, 0x01);
  set_register(addr,0x2C, 0x01);
  set_register(addr,0x2D, 0x0E);
  set_register(addr,0x2E, 0x00);

  set_register(addr,0x2F, 0x01);
  set_register(addr,0x30, 0x05);
  set_register(addr,0x31, 0x01);
  set_register(addr,0x32, 0x00);



  set_register(addr,0x33, 0x00);
  set_register(addr,0x34, 0x00);
  set_register(addr,0x35, 0x00);

  set_register(addr,0x5B, 0);//DEBOUNCE
  set_register(addr,0x5C, 0x10); //AFE1 default, 16uA charge current
  set_register(addr,0x5D, 0x20); // AFE2 0.5uS encoding, 1ms period

  set_register(addr,0x5C, 0b1100000+1); // CDC 1ma
  set_register(addr,0x5D, 0b00100000); // CDT=0.5us SFI=4 ESI=1ms
  set_register(addr,0x7D, 201); 
  set_register(addr,0x7F, 181); 
  set_register(addr,0x7E, 131); 
  set_register(addr,0x7B, 0b1011); 

  //delay(1500);
  //show_regs(0);

  set_register(addr,0x5E, 1);  // start with first 5 bits of baseline tracking
  //show_regs(0);
  
  for (byte i=40; i<45; i++) get_register(0, i);
  
  set_register(addr,0x5E, 1);  // start with first 5 bits of baseline tracking

  return 1;
}

boolean checkInterrupt(byte idx){
  switch (idx) {
    case 0: return !digitalRead(irqpin1);
    case 1: return !digitalRead(irqpin2);
    case 2: return !digitalRead(irqpin3);
    case 3: return !digitalRead(irqpin4);
    default: ;
  }
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

