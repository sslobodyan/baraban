
// MPR121 Register Defines
#define MHD_R  0x2B
#define NHD_R 0x2C
#define NCL_R   0x2D
#define FDL_R 0x2E
#define MHD_F 0x2F
#define NHD_F 0x30
#define NCL_F 0x31
#define FDL_F 0x32
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
#define FIL_CFG 0x5D
#define ELE_CFG 0x5E
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

// Global Constants
#define TOU_THRESH  0x06
#define REL_THRESH  0x0A

int irqpin1 = PB7;  
int irqpin2 = PB7;  
int irqpin3 = PB7;  
int irqpin4 = PB7;  

byte mpr_addr[4] = {0x5A,0x5B,0x5C,0x5D};

byte mpr121_setup_registers(byte addr);
boolean checkInterrupt(byte idx);
void set_register(int address, unsigned char r, unsigned char v);

byte setup_touch(){
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

void readTouchInputs(){
  for (byte i=0; i<sizeof(mpr_addr); i++) {
    if ( checkInterrupt(i) ) {  
        Wire.beginTransmission(mpr_addr[i]);
        if ( Wire.endTransmission(false) == 0) {
            Wire.requestFrom(mpr_addr[i],2); 
            byte LSB = Wire.read();
            byte MSB = Wire.read();
            
            uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states
        
            for (int i=0; i < 12; i++){  // Check what electrodes were pressed
              if(touched & (1<<i)){
              //ToDo проставить флаг в нужном канале    
              }
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
  
  // Section A - Controls filtering when data is > baseline.
  set_register(addr, MHD_R, 0x01);
  set_register(addr, NHD_R, 0x01);
  set_register(addr, NCL_R, 0x00);
  set_register(addr, FDL_R, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(addr, MHD_F, 0x01);
  set_register(addr, NHD_F, 0x01);
  set_register(addr, NCL_F, 0xFF);
  set_register(addr, FDL_F, 0x02);
  
  // Section C - Sets touch and release thresholds for each electrode
  set_register(addr, ELE0_T, TOU_THRESH);
  set_register(addr, ELE0_R, REL_THRESH);
 
  set_register(addr, ELE1_T, TOU_THRESH);
  set_register(addr, ELE1_R, REL_THRESH);
  
  set_register(addr, ELE2_T, TOU_THRESH);
  set_register(addr, ELE2_R, REL_THRESH);
  
  set_register(addr, ELE3_T, TOU_THRESH);
  set_register(addr, ELE3_R, REL_THRESH);
  
  set_register(addr, ELE4_T, TOU_THRESH);
  set_register(addr, ELE4_R, REL_THRESH);
  
  set_register(addr, ELE5_T, TOU_THRESH);
  set_register(addr, ELE5_R, REL_THRESH);
  
  set_register(addr, ELE6_T, TOU_THRESH);
  set_register(addr, ELE6_R, REL_THRESH);
  
  set_register(addr, ELE7_T, TOU_THRESH);
  set_register(addr, ELE7_R, REL_THRESH);
  
  set_register(addr, ELE8_T, TOU_THRESH);
  set_register(addr, ELE8_R, REL_THRESH);
  
  set_register(addr, ELE9_T, TOU_THRESH);
  set_register(addr, ELE9_R, REL_THRESH);
  
  set_register(addr, ELE10_T, TOU_THRESH);
  set_register(addr, ELE10_R, REL_THRESH);
  
  set_register(addr, ELE11_T, TOU_THRESH);
  set_register(addr, ELE11_R, REL_THRESH);
  
  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(addr, FIL_CFG, 0x04);
  
  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(addr, ELE_CFG, 0x00);  
  
  // Section F
  // Enable Auto Config and auto Reconfig
  /*set_register(0x5A, ATO_CFG0, 0x0B);
  set_register(0x5A, ATO_CFGU, 0xC9);  // USL = (Vdd-0.7)/vdd*256 = 0xC9 @3.3V   set_register(0x5A, ATO_CFGL, 0x82);  // LSL = 0.65*USL = 0x82 @3.3V
  set_register(0x5A, ATO_CFGT, 0xB5);*/  // Target = 0.9*USL = 0xB5 @3.3V
  
  set_register(addr, ELE_CFG, 0x0C); // Set to RUN mode with all 12 electrodes
  
  return 1;
}

boolean checkInterrupt(byte idx){
  switch (idx) {
    case 0: return digitalRead(irqpin1);
    case 1: return digitalRead(irqpin2);
    case 2: return digitalRead(irqpin3);
    case 3: return digitalRead(irqpin4);
    default: ;
  }
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

