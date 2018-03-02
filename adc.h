uint32 calc_adc_SQR3(uint8 adc_sequence[6]);
uint32 calc_adc_SQR2(uint8 adc_sequence[6]);
uint32 calc_adc_SQR1(uint8 adc_sequence[4]);

inline void store_maximum() {
  uint32_t idx=1;
  uint32_t buf;
  for( uint32_t i=0; i<NUM_CHANNELS; i++) { // 0..32

    if (kanal[i].note == 0) continue; // если нота не назначена, то обработку канала пропускаем

    buf = buf_adc[last_buf_idx][idx];
    
    if ( kanal[i].scan_cnt == -1 ) { // еще не начинали сканировать - порог пока не превышен
        if ( buf > kanal[i].treshold ) { // превысили порог - начало удара
          kanal[i].scan_cnt = 0; // начали поиска максимума
          kanal[i].adc_max = buf;
        }
    } else { // уже ловим максимум
        kanal[i].scan_cnt++;
        if ( kanal[i].adc_max < buf ) kanal[i].adc_max = buf;
        if ( buf < kanal[i].treshold ) {
          kanal[i].scan_cnt = -1; // приняли короткий шумовой всплеск - перезапускаем сканирование     
        }
        if ( kanal[i].scan_cnt == cfg.scan_cnt  ) { // просканировали нужное количество раз
            add_note( i, kanal[i].adc_max ); // запоминаем ноту с какого канала надо проиграть        
        }
        if ( kanal[i].scan_cnt == cfg.mute_cnt  ) { // выждали конец колебаний
            kanal[i].scan_cnt = -1; // готовы к следующему удару
            //digitalWrite( PC15,  !digitalRead(PC15) );
        }
    }
    idx += 2;
  } // for
}



void store_autotreshold() {
  byte idx=1;
  for( byte i=0; i<NUM_CHANNELS; i++) {
    if ( kanal[i].adc_max < buf_adc[last_buf_idx][idx] ) {
      kanal[i].scan_cnt += 1;
      if (kanal[i].scan_cnt >= cfg.scan_cnt) {
        kanal[i].adc_max = buf_adc[last_buf_idx][idx];  
        kanal[i].scan_cnt = 0;
      }
    } else {
      kanal[i].scan_cnt = 0;
    }
    idx += 2;
  } 
}

static bool bit_c;
void  next_multiplexor(){ // выбрать следующий мультиплексор
  last_milti_idx = multi_idx; // запоминаем какой мультиплексор просканировали

  //if (++multi_krutilka_idx >= KRUTILKI_CNT) multi_krutilka_idx = 0;
  
  if (++multi_idx >= NUM_MULTIPLEXORS) {
    multi_idx = 0; // новый проход по всем мультиплексорам
    last_buf_idx = buf_idx;
    bit_c = !bit_c;
    if (++buf_idx >= BUFFER_CNT) {
      buf_idx=0;
    }
  }
  
  multi_krutilka_idx = multi_idx * 2 + bit_c;
  
  switch (multi_idx) {
    case 0:
      digitalWrite(MX_A001, 0); digitalWrite(MX_A010, 0); digitalWrite(MX_A100, bit_c); break;
    case 1:
      digitalWrite(MX_A001, 1); digitalWrite(MX_A010, 0); digitalWrite(MX_A100, bit_c); break;
    case 2:
      digitalWrite(MX_A001, 0); digitalWrite(MX_A010, 1); digitalWrite(MX_A100, bit_c); break;
    default:
      digitalWrite(MX_A001, 1); digitalWrite(MX_A010, 1); digitalWrite(MX_A100, bit_c);     
  }
}

void setup_new_scan() {
  DMA1->regs->CMAR1 = (uint32)&buf_adc[buf_idx][ multi_idx * 2 * NUM_ADC ]; // повторно адрес куда начнем писать
  DMA1->regs->CNDTR1 = NUM_ADC*2; // повторно количество транзакций
  
  dma_enable(DMA1, DMA_CH1); // Enable the channel   
  ADC1->regs->CR2 |= ADC_CR2_SWSTART; // запускаем регулярное преобразование АЦП
}
 
static void DMA1_CH1_Event() { // ПРЕРЫВАНИЕ ДМА закончили сбор - буфер одного мультиплексора (8 входов) заполнен 
  LED_OFF;
  
  dma_disable(DMA1, DMA_CH1); 

  // состояние 0 и 9 каналов по АЦП2 запоминаем до смены мультиплексора!
  buf_krutilka[ multi_krutilka_idx  ][0] = ADC2->regs->JDR1; //
  buf_krutilka[ multi_krutilka_idx  ][1] = ADC2->regs->JDR3; //

  next_multiplexor();

  if (multi_idx == 0) { // прошли по всем 4 мультиплексорам - отсканированы все 32 датчика
    adc_dma_cnt++;
    if ( scan_autotreshold ) {
      store_autotreshold();
    }
    else {
      store_maximum();
    }  
  }

  if (! stop_scan ) setup_new_scan(); // чисто отладка
  
  LED_ON;
}

void setup_DMA() {
  dma_init(DMA1);
  dma_disable(DMA1, DMA_CH1); 
  dma_setup_transfer( DMA1, DMA_CH1, 
                      &ADC1->regs->DR, DMA_SIZE_16BITS, // из какой периферии и размерность
                      &buf_adc[buf_idx][0], DMA_SIZE_16BITS, // в какую память и размерность
                      (DMA_MINC_MODE | DMA_TRNS_CMPLT) // увеличивать память и вызвать прерывание по окончании
                     ); 
  dma_attach_interrupt(DMA1, DMA_CH1, DMA1_CH1_Event);
  DMA1->regs->CCR1 &= ~( DMA_CCR_HTIE | DMA_CCR_TEIE ) ; // отключить прерывание половины буфера и ошибки обмена
}

void setup_ADC() {
  // включаем все аналоговые входы
  pinMode(PA0,INPUT_ANALOG);
  pinMode(PA1,INPUT_ANALOG);
  pinMode(PA2,INPUT_ANALOG);
  pinMode(PA3,INPUT_ANALOG);
  pinMode(PA4,INPUT_ANALOG);
  pinMode(PA5,INPUT_ANALOG);
  pinMode(PA6,INPUT_ANALOG);
  pinMode(PA7,INPUT_ANALOG);
  pinMode(PB0,INPUT_ANALOG);
  pinMode(PB1,INPUT_ANALOG);
  
  // управление мультиплексором
  pinMode(MX_A001, OUTPUT);
  pinMode(MX_A010, OUTPUT);
  pinMode(MX_A100, OUTPUT);
  digitalWrite(MX_A001, LOW);
  digitalWrite(MX_A010, LOW);
  digitalWrite(MX_A100, LOW);

  last_buf_idx, buf_idx = 0;
  adc_set_prescaler(ADC_PRE_PCLK2_DIV_6); // 12 МГц тактовая АЦП (максимум 14)
  adc_set_sample_rate(ADC1, ADC_SMPR_7_5); // 7.5+12.5 = 20 такта на выборку 1/12*20=1.667мкс выборка 1.5 7.5 13.5 28.5 41.5 55.5 71.5 239.5
  adc_set_sample_rate(ADC2, ADC_SMPR_7_5); // 
  // для 1.5 один опрос сенсоров 14 тактов 19мкс ==1          t=124 us 32 ch
  // для 7.5 один опрос сенсоров 20 тактов 27мкс ==1 129us     t=155 us 32 канала
  // для 13.5 один опрос сенсоров 26 тактов 35мкс ==1 155us 32ch
  // для 28.5 один опрос сенсоров 41 тактов 55мкс 1/12000000*41*16 228us 32ch
  // для 41.5 один опрос сенсоров 54 тактов 73мкс 300us 32ch
  // для 55.5 один опрос сенсоров 68 тактов 91мкс 380us
  // минимально 28,5 тактов иначе тянет хвост сигнала на следующий канал
  
  adc_calibrate(ADC1);
  adc_calibrate(ADC2);
  // загружаем последовательность сканирования каналов АЦП1
  ADC1->regs->SQR3 |= calc_adc_SQR3(ADC_3Sequence);
  ADC1->regs->SQR2 |= calc_adc_SQR2(ADC_2Sequence);
  ADC1->regs->SQR1 |= calc_adc_SQR1(ADC_1Sequence);
  adc_set_reg_seqlen(ADC1, NUM_ADC*2); // кол-во опрашиваемых каналов - через один заземленные

  ADC1->regs->CR1 |= ADC_CR1_SCAN; // сканировать все регулярные каналы
  ADC1->regs->CR2 |= ADC_CR2_DMA; // Set DMA 

  ADC2->regs->CR2     =  ADC_CR2_JEXTSEL;      //выбрать источником запуска разряд  JSWSTART
  ADC2->regs->CR2    |=  ADC_CR2_JEXTTRIG;     //разр. внешний запуск инжектированной группы
  ADC2->regs->CR1    |=  ADC_CR1_SCAN;         //режим сканирования (т.е. несколько каналов)
  ADC2->regs->CR1    |=  ADC_CR1_JAUTO;        //автомат. запуск инжектированной группы
  ADC2->regs->CR2    |=  ADC_CR2_CONT;         //режим непрерывного преобразования 
  ADC2->regs->JSQR    =  (uint32_t)(4-1)<<20;  //задаем количество каналов в инжектированной группе
  ADC2->regs->JSQR   |=  (uint32_t)0<<(5*0);   //номер канала для первого преобразования             
  ADC2->regs->JSQR   |=  (uint32_t)0<<(5*1);   //номер канала для первого преобразования             
  ADC2->regs->JSQR   |=  (uint32_t)9<<(5*2);   //номер канала для второго преобразования
  ADC2->regs->JSQR   |=  (uint32_t)9<<(5*3);   //номер канала для первого преобразования             
  ADC2->regs->CR2    |=  ADC_CR2_ADON;         //включить АЦП
  ADC2->regs->CR2    |=  ADC_CR2_JSWSTART;     //запустить процес преобразования
}

// calculate values for SQR3. Function could be extended to also work for SQR2 and SQR1. As is, you can sequence only 6 sequences per ADC
uint32 calc_adc_SQR3(uint8 adc_sequence[6]){
  int SQR3=0;
  for (int i=0;i<6;i++)     // There are 6 available sequences in SQR3 (SQR2 also 6, and 4 in SQR1).
  {
    SQR3 |= adc_sequence[i] << ((i*5));  
  } 
  return SQR3;
} 

// calculate values for SQR2. Function could be extended to also work for SQR3 and SQR1. As is, you can sequence only 6 sequences per ADC
uint32 calc_adc_SQR2(uint8 adc_sequence[6]){
  int SQR2=0;
  for (int i=0;i<6;i++)     // There are 6 available sequences in SQR2 (SQR3 also 6, and 4 in SQR1).
  {
    SQR2 |= adc_sequence[i] << ((i*5));  
  } 
  return SQR2;
} 

// calculate values for SQR1.
uint32 calc_adc_SQR1(uint8 adc_sequence[4]){
  int SQR1=0;
  for (int i=0;i<4;i++) {
    SQR1 |= adc_sequence[i] << ((i*5));  
  } 
  return SQR1;
} 

