#define NUM_MULTIPLEXORS 4 // обычно 4
#define NUM_ADC 8 // обычно 8
#define NUM_CHANNELS NUM_ADC*NUM_MULTIPLEXORS

#define MX_A001 PB3
#define MX_A010 PB4

#define DBGserial Serial1 // A9
#define MIDIserial Serial3 // B10
#define DRUMS 10 // номер MIDI-канала в который передаем

#define LED PC13
#define LED_ON digitalWrite(LED, LOW)
#define LED_OFF digitalWrite(LED, HIGH)
#define LED_TOGGLE digitalWrite(LED,!digitalRead(LED))

#define GREEN_LED PA12
#define GREEN_ON digitalWrite(GREEN_LED, HIGH);
#define GREEN_OFF digitalWrite(GREEN_LED, LOW);
#define GREEN_TOGGLE digitalWrite(GREEN_LED,!digitalRead(GREEN_LED))

#define RED_LED PA11
#define RED_ON digitalWrite(RED_LED, HIGH)
#define RED_OFF digitalWrite(RED_LED, LOW)
#define RED_TOGGLE digitalWrite(RED_LED,!digitalRead(RED_LED))

#define TEST_KANAL_GREEN 8
#define TEST_KANAL_RED 16

uint8_t ADC_1Sequence[4]={9,7,9,8};   
uint8_t ADC_2Sequence[6]={9,4,9,5,9,6};   
uint8_t ADC_3Sequence[6]={9,1,9,2,9,3};   // входы с 1 - см схему, 0-контрольный, 9-земля

#define BUFFER_CNT 10
volatile byte last_buf_idx, buf_idx; // BUFFER_CNT буферов - пока один обрабатываем, во второй сканируются входы
volatile uint16_t buf_adc[BUFFER_CNT][NUM_ADC*2*NUM_MULTIPLEXORS]; // ДМА буфер всех сканирований
volatile bool adc_block;

volatile int multi_idx; // номер включенного мультиплексора
volatile int last_milti_idx; // номер предыдущего (только что считанного) мультиплексора

bool scan_autotreshold = true; // признак сбора данных для автотрешолда

struct stChannel {
  uint16_t adc_max; // максимум с момента превышения порога
  uint16_t treshold; // порог уровня удара
  byte note; // номер ноты
  uint32_t scan_time; // до которого времени опрашивать на максимум
  uint32_t mute_time; // до которого времени не сканировать кнопку или 0 если можно сканировать
  uint32_t noteoff_time; // когда посылать note_off
  uint16_t velocity1; // уровень для громкости == 1
  uint16_t velocity127; // уровень для громкости == 127
} kanal[NUM_CHANNELS];

struct stConfig {
  uint16_t scan_time = 1000; // us ожидания после первого превышения порога == время набора максимума
  uint32_t mute_time = 60000; // us запрета сканирования после фиксации сработки кнопки (вычисление mute_time)
  uint16_t autotreshold_above = 50; // порог выше уровня шумов
  uint32_t noteoff_time = 500; // ms звучания ноты
} cfg;

#define NOTES_CNT 30 // длина буфера нажатых нот 
struct stNotes {
  byte kanal;
  uint16_t level; // значение АЦП при сработке
} notes[NOTES_CNT];

volatile byte head_notes, tail_notes; // указатели на голову и хвост буфера нот
bool stop_scan; // флаг остановки сканирования

/////////////////////////  Объявления функций //////////////////////////////////////

void add_note(byte ch, uint16_t level);
void store_autotreshold();


