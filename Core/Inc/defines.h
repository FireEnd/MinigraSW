#define SCR_WIDTH  400 // szerokość ekranu
#define SCR_HEIGHT 240 // wysokość ekranu
#define BACKGROUND_C BLACK // kolor tła (jak pierwszy i drugi bajt będą takie same to będzie szybciej)
#define DEF_FONT &FreeSansBoldOblique9pt7b // cionka domyślna
#define ILI9327 // czyli nie ten wyświetlacz z aparatury tylko ten dziwny

#define ADC_N 4 // liczba kanałów ADC
#define BUT_N 1 // liczba przycisków w urządzeniu
#define MINIGRA // włącza obecność minigry
//#define DEFAULT_DISP_INT 20 // domyślny interwał [ms] odświeżania wyświetlacza
#define DISPLAY_INT 25 // interwał [ms] odświeżania wyświetlacza
#define SYSTEM_CLOCK 84000 // częstotliwość [kHz] zegara systemowego

#define DEBOUNCE_TIME 20 // czas [ms] debouncingu
#define BUT_PIN 12 // pin przycisku
#define BUT_PORT GPIOB // port przycisku

#define HOME_INDEX 0 // numer ekranu głównego
#define GAME_INDEX 1 // numer ekranu gry

uint8_t menu[2] = {HOME_INDEX, GAME_INDEX}; // {ustawiony, aktualny} ekran
bool butLockFlag[BUT_N]; // czy dany przycisk jest zablokowany i ma zwracać 0 (1-tak, 0 - normalnie)
uint32_t scrTimer = 0; // timer odświeżania wyświetlacza
uint32_t ADC_INS[ADC_N]; // próbki wejściowe ADC

//uint32_t DISPLAY_INT = DEFAULT_DISP_INT; // zmienialny interwał wyświetlania
