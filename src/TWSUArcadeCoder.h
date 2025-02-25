#ifndef __ARCADE_CODER_CORE
#define __ARCADE_CODER_CORE

/**
  Core board operations:
    - hardcoded pin data
    - handling multiplexing
    - sending data to shift registers in batches
*/

/* Pin allocation */
#define HOME_BTN 2
#define LED_1   22
#define LED_2   23
// HC595 Pins
#define HC595_DATA 5 // GPIO5 (Data) // DS
#define HC595_CLK 17 // GPIO17 (Clock) // SHCP_PIN
#define HC595_LAT 16 // GPIO16 (Latch) // STCP_PIN
#define HC595_OE   4 // GPIO4 (Output Enable, LOW = enable) // THIS MAY BREAK WIFI depending on some factors around SPI usage
// ICN2012 Pins, I haven't been able to find out if pin E2 has a function (E1 is directly connected to 3.3v)
#define ICN_A0 19   // GPIO19 (Column Address Bit 0)
#define ICN_A1 18   // GPIO18 (Column Address Bit 1)
#define ICN_A2 21   // GPIO21 (Column Address Bit 2)

#define CHANNEL_COUNT    6 // the number of multiplexed channels
#define CHANNEL_ENTRIES 24 // the number of LED/Button nodes in a channel
#define HC595_CNT 9


#define WHITE   0b000
#define BLACK   0b111
#define GREEN   0b011
#define RED     0b101
#define BLUE    0b110
#define YELLOW  0b001
#define MAGENTA 0b100
#define CYAN    0b010

#define MATRIX_EMPTY { \
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,\
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK }

#define RAW_BLACK_DATA { 255, 255, 255, 255, 255, 255, 255, 255, 255 }

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 

void hc595Write(uint8_t data[]);
void setMuxChannel(int val);

/////// DISPLAY

void printMatrix(uint8_t mtrx[144]);
void setCurrentMatrix(uint8_t val[144]);
void getChannelLedData(uint8_t channel);
void fillCurrentMatrix(uint8_t val);
void renderChannel(uint8_t channel);
void refreshFields();

extern uint8_t rawLedData[6][9];

/////////

void renderOnePixel(uint8_t renderIdx);
bool isButtonInMultiplexPressed(uint8_t channel, uint16_t threshold);
bool isButtonPressed(uint8_t channel, uint8_t index, uint16_t threshold);


const int unknownPins[] = {
  12, 13, 14, 15, 25, 26, 27
}; // 26 and 27 read as 4095 (possibly the accelerometer?), pin 25 to ground triggers reset in stock firmware
const int possiblyUnused[] = {12, 13, 14, 15, 25 }; // no traces observed, potentially usable



// the physical wiring of LEDs to shift registers; two rows of 12, top left = 0, bottom right = 23
extern uint8_t indexing[];
extern uint8_t currentMatrix[144];
extern uint8_t multiplexedGroups[6][24];
extern uint8_t CHANNEL_OFF[];
extern const uint8_t matrixInputs[]; // all register values when pressed 

#endif
