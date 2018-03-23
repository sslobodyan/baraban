#define NUM_MULTIPLEXORS 4 // обычно 4
#define NUM_ADC 8 // обычно 8
#define NUM_CHANNELS NUM_ADC*NUM_MULTIPLEXORS

#define MX_A001 PB3 // A
#define MX_A010 PB4 // B
#define MX_A100 PA15 // C

#define DBGserial Serial1 // A9
#define MIDIserial Serial3 // B10

#define DRUMS 10 // номер MIDI-канала в который передаем

#define SELECT_MODULE1 PB5 // пайка
#define SELECT_MODULE2 PA8 // тумблер

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

#define MODULE_72 72
#define MODULE_60 60
#define MODULE_48 48
#define MODULE_36 36

#define GND_SENSOR 15
uint8_t ADC_1Sequence[4]={GND_SENSOR,6,GND_SENSOR,7};   
uint8_t ADC_2Sequence[6]={GND_SENSOR,3,GND_SENSOR,4,GND_SENSOR,5};   
uint8_t ADC_3Sequence[6]={GND_SENSOR,0,GND_SENSOR,1,GND_SENSOR,2};   // входы с 1 - см схему, 0-контрольный, 9-земля

#define BUFFER_CNT 10
volatile byte last_buf_idx, buf_idx; // BUFFER_CNT буферов - пока один обрабатываем, во второй сканируются входы
volatile uint16_t buf_adc[BUFFER_CNT][NUM_ADC*2*NUM_MULTIPLEXORS]; // ДМА буфер всех сканирований АЦП1

#define LAST_NOTE 109 // последняя нота как нота - следующие это крутилки

// на каких номерах нот какие обработчики крутилок
#define POT_VELOCITY1 110
#define POT_VELOCITY127 111
#define POT_LENGTH0 112   // 0x70
#define POT_LENGTH1 119   // 0x77
#define POT_VOLUME 113    // 0x71
#define POT_VOLUME_METRONOM 120    // 0x78
#define POT_MUTE_CNT 121    // 0x79
#define POT_SCAN_CNT 122    // 0x7A
#define POT_CROSS_CNT 123    // 0x7B
#define POT_CROSS_PRCNT 124    // 0x7C
#define POT_METRONOM 125    // 0x7D

#define PEDAL_AUTOTRESHOLD 109 // 0x6D
#define PEDAL_SUSTAIN 114 // 0x72
#define PEDAL_VOICE 115   // 0x73
#define PEDAL_OCTAVE 116  // 0x74
#define PEDAL_PROGRAM 117 // 0x75
#define PEDAL_PANIC 118   // 0x76
#define PEDAL_METRONOM1 126   // 0x7E
#define PEDAL_METRONOM10 127   // 0x7F

// состояние любой педали
#define PEDAL_DOWN 127
#define PEDAL_CENTER 64
#define PEDAL_UP 0

#define METRONOME_MIN 40 // минимум BPS
#define METRONOME_MAX 210 // максимально BPS

volatile int multi_idx; // номер включенного мультиплексора
volatile int last_milti_idx; // номер предыдущего (только что считанного) мультиплексора

bool scan_autotreshold = false; // признак сбора данных для автотрешолда

struct stChannel {
  // настройка
  uint32_t treshold; // порог уровня удара
  byte note; // номер ноты
  uint32_t velocity1; // уровень для громкости == 1
  uint32_t velocity127; // уровень для громкости == 127
  // рассчетные
  uint32_t adc_max; // максимум с момента превышения порога
  uint32_t noteoff_time; // когда посылать note_off ms
  bool pressed; // нажат
  uint8_t show; // (0-молчать,1-вывод уровня выше трешолда,2-вывод текущего уровня)
  int32_t scan_cnt;
  uint8_t group; // группа по кросстолку
  uint8_t port; // номер физического входа для упорядочивания
} kanal[NUM_CHANNELS];

struct stConfig {
  uint32_t autotreshold_above = 40; // порог выше уровня шумов
  uint32_t noteoff_time0 = 500; // ms звучания ноты без педали
  uint32_t noteoff_time1 = 2500; // ms звучания ноты с полупедалью
  uint32_t autotreshold_time = 1000; // ms настройки порогов после старта
  uint8_t pedal = 0; // состояние педали sustain 0 - 63 - 127
  uint8_t pedal_voice = 0; // состояние педали для добавления к номеру голоса
  uint8_t voice = 0; // текущий номер голоса
  uint8_t delta_voice = 4; // шаг добавления номера голоса педалью
  uint8_t module = MODULE_72; // номер модуля (меняется при включении)
  uint8_t start_note = MODULE_72; // номер ноты нулевого входа
  uint8_t end_note = MODULE_72+32; // номер ноты последнего входа
  uint8_t pedal_octave = PEDAL_CENTER; // состояние педали сдвига октавы (12==без сдвига)
  uint8_t pedal_program = PEDAL_CENTER; // состояние педали номера программы (0-отпущена, 1-вниз, 2-вверх)
  uint8_t curr_program = 0; // текущий номер программы
  uint8_t max_program = 10; // максимальный номер программы
  uint8_t volume=100;
  uint8_t cross_cnt = 6; // сколько опросов АЦП ждать кросстолк (1 опрос 133 мкс)
  uint8_t show_debug = true; // выводить отладку в порт
  uint16_t velocity1 = 0;
  uint16_t velocity127 = 0;
  int16_t scan_cnt = 12; // 6==800us количество полных сканирований АЦП после превышения трешолда
  int16_t mute_cnt = 560; // количество полных сканирований АЦП для игнора успокаивающегося датчика
  uint32_t metronom = 500; // 500 == 60000 / 120  интервал в миллисекундах, если 0 - то молчим
  uint8_t metronom_volume = 0; // громкость метронома
  uint8_t metronom_kanal = NUM_CHANNELS-1; // канал метронома
  uint8_t metronom_krat = 4; // кратность долей метронома
  uint8_t cross_percent = 30; // подавлять кросстолк до указанного уровня в процентах от самого сильного сигнала
  uint16_t max_level = 1800; // уровень сигнала, когда включаем красный светодиод
  uint8_t pedal_metronom1 = PEDAL_CENTER; // изменение метронома на 1 bps
  uint8_t pedal_metronom10 = PEDAL_CENTER; // изменение метронома на 10 bps
//  uint8_t pusto; // // выровнять размер структуры на 16 бит !!!
} cfg;

#define NOTES_CNT 10 // длина буфера нажатых нот 
struct stNotes {
  byte kanal;
  uint16_t level; // значение АЦП при сработке
  byte group; 
  uint32_t cross_cnt; // на каком сканировании запомнили
} notes[NOTES_CNT];

volatile byte head_notes, tail_notes; // указатели на голову и хвост буфера нот
bool stop_scan; // флаг остановки сканирования

#define KRUTILKI_CNT 16 // максимальное число крутилок
struct stKrutilka { 
  uint16_t velocity1; // АЦП для 1
  uint16_t velocity127; // АЦП для 127
  uint8_t value; // в пересчете от 1 до 127
  uint8_t mx; // номер мультиплексора ( 0..3 )
  uint8_t ch; // номер канала ( 0..1 )
  uint8_t gist; // гистерезис изменений 
  void (*onChange)(uint8_t idx); // обработчик при изменении значения крутилки IDX
  // обработчик изменения
  uint8_t show; // (0-молчать,1-вывод текущего уровня)
  uint8_t prevCC; // значение предыдущего мидиСС от этой крутилки
  uint16_t adc; // текущее значение по АЦП
  byte type; //
} krutilka[ KRUTILKI_CNT ];

uint8_t krutilka_idx; // текущая обрабатываемая крутилка
uint8_t multi_krutilka_idx; // индекс канала мультиплексора крутилок
volatile uint16_t buf_krutilka[ KRUTILKI_CNT/2 ][2]; // буфер всех сканирований АЦП2 канал 0, канал 9


void (*handl)(uint8_t tp); // глобальная переменная с адресом обработчика крутилки

struct stTouchModule {
  uint8_t kanal[8];
  uint16_t touched;  
} touch[4]; // 4 модуля касания по 8 входов - к какому аналогову входу привязан

volatile uint32_t adc_dma_cnt=0; // последовательный счетчик прерываний ДМА по АЦП
uint32_t old_metronom = 0; // время удара метронома
uint8_t metronom_krat = cfg.metronom_krat-1; // кратность метронома
volatile bool adc_new_cycle; // выставляется в прерывании при включении 0 мультиплексора (новый цикд)

uint32_t tm_time, time_green, time_red, time_autotreshold, time_voice;

const byte version[] = __DATE__;

/////////////////////////  Объявления функций //////////////////////////////////////

void add_note(byte ch, uint16_t level);
void store_autotreshold();
void update_krutilki();
void setup_krutilki();
void set_handl(uint8_t tp);
void krutilka_set_type(uint8_t idx, uint8_t tp);
void note_off(byte ch);
void send_SysEx(byte size, byte *arr);
bool check_groups();
void show_krutilki_adc();
void show_krutilki();
void show_krutilki_buf();

