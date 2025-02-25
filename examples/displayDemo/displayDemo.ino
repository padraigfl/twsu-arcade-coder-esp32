#include <TWSUArcadeCoder.h>

// example of the board displaying things

// Shorthand to make easier to read
#define W   WHITE
#define K   BLACK
#define G   GREEN
#define R   RED
#define B   BLUE
#define Y   YELLOW
#define M   MAGENTA
#define C   CYAN

uint8_t HEART[144] = {
  K,K,W,W,W,K,K,W,W,W,K,K,
  K,W,R,R,R,W,W,R,R,R,W,K,
  W,R,R,R,R,R,R,R,R,R,R,W,
  R,R,R,R,R,R,R,R,R,R,R,R,
  R,R,R,R,R,R,R,R,R,R,R,R,
  W,R,R,R,R,R,R,R,R,R,R,W,
  K,W,R,R,R,R,R,R,R,R,W,K,
  K,K,W,R,R,R,R,R,R,W,K,K,
  K,K,K,W,R,R,R,R,W,K,K,K,
  K,K,K,K,W,R,R,W,K,K,K,K,
  K,K,K,K,W,R,W,K,K,K,K,K,
  K,K,K,K,W,W,K,K,K,K,K,K,
};

uint8_t HEART_SMALL[144] = {
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,W,W,K,K,W,W,K,K,K,
  K,K,W,R,R,W,W,R,R,W,K,K,
  K,W,R,R,R,R,R,R,R,R,W,K,
  W,R,R,R,R,R,R,R,R,R,R,W,
  W,R,R,R,R,R,R,R,R,R,R,W,
  K,W,R,R,R,R,R,R,R,R,W,K,
  K,K,W,R,R,R,R,R,R,W,K,K,
  K,K,K,W,R,R,R,R,W,K,K,K,
  K,K,K,K,W,R,R,W,K,K,K,K,
  K,K,K,K,W,R,W,K,K,K,K,K,
  K,K,K,K,W,W,K,K,K,K,K,K,
};
uint8_t OFF[144] = {
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
};
uint8_t TEXT_MAYBE[144] = { // extremely compact text, does not work
  K,W,K,R,K,K,K,K,K,K,K,G,
  W,K,W,R,R,K,K,C,C,K,G,G,
  W,W,W,R,K,R,C,K,K,G,K,G,
  W,K,W,R,R,K,K,C,C,K,G,G,
  K,M,K,K,B,B,M,M,M,Y,K,Y,
  M,M,M,B,K,K,M,K,K,Y,K,Y,
  M,K,K,B,B,B,M,K,M,Y,Y,Y,
  K,M,M,B,K,K,M,M,M,Y,K,Y,
  K,W,K,K,K,G,B,K,B,R,K,K,
  K,W,K,K,K,G,B,B,K,R,K,K,
  K,W,K,G,K,G,B,K,B,R,K,K,
  K,W,K,K,G,K,B,K,B,R,R,K,
};
uint8_t TEXT_MAYBE_2[144] = {
  K,W,K,B,B,K,K,C,K,G,G,K,
  W,K,W,B,K,B,C,K,C,G,K,G,
  W,K,W,B,B,K,C,K,K,G,K,G,
  W,W,W,B,K,B,C,K,C,G,K,G,
  W,K,W,B,B,K,K,C,K,G,G,K,
  K,K,K,K,K,K,K,K,K,K,K,K,
  R,R,R,W,W,W,K,G,G,C,K,C,
  R,K,K,W,K,K,G,K,K,C,K,C,
  R,R,R,W,W,W,G,K,K,C,C,C,
  R,K,K,W,K,K,G,K,G,C,K,C,
  R,R,R,W,K,K,K,G,G,C,K,C,
  K,K,K,K,K,K,K,K,K,K,K,K,
};
uint8_t TIME_1[144] = { // very basic time display
  K,W,W,W,K,K,K,W,W,W,K,K,
  W,K,K,K,W,K,W,K,K,K,W,K,
  K,K,K,K,W,K,K,K,K,K,W,K,
  K,K,W,W,K,K,K,K,K,W,K,K,
  K,W,K,K,K,K,K,K,W,K,K,K,
  W,W,W,W,W,K,K,W,W,W,W,K,
  K,K,G,G,G,K,K,K,G,G,G,K,
  K,G,K,K,K,G,K,G,K,K,K,G,
  K,K,K,K,K,G,K,K,K,K,K,G,
  K,K,K,G,G,K,K,K,K,G,G,K,
  K,K,G,K,K,K,K,K,G,K,K,K,
  K,G,G,G,G,G,K,G,G,G,G,G,
};

uint8_t TIME_2[144] = { // possible 12 hour clock display with weather and thermometer
  C,K,M,M,M,K,C,C,C,M,M,M,
  C,K,K,K,M,W,C,K,K,K,K,M,
  C,K,M,M,M,K,C,C,C,M,M,M,
  C,K,M,K,K,W,K,K,C,K,K,M,
  C,K,M,M,M,K,C,C,C,M,M,M,
  K,K,K,K,K,K,K,K,K,K,K,K,
  K,Y,Y,K,K,K,K,K,K,R,R,K,
  K,Y,Y,W,W,K,K,K,K,R,R,K,
  K,W,W,W,W,W,W,K,K,R,R,K,
  K,W,W,W,W,W,W,K,K,M,M,K,
  K,K,W,W,W,W,K,K,K,M,M,K,
  K,K,K,K,K,K,K,K,K,B,B,K,
};

void setupPinModes() {
  // Configure HC595 pins
  pinMode(HC595_DATA, OUTPUT);
  pinMode(HC595_CLK, OUTPUT);
  pinMode(HC595_LAT, OUTPUT);
  pinMode(HC595_OE, OUTPUT); // possible switch to ledcAttachChannel?
  // Configure ICN2012 pins
  pinMode(ICN_A0, OUTPUT);
  pinMode(ICN_A1, OUTPUT);
  pinMode(ICN_A2, OUTPUT);
}

void setInitialValues() {
  // Initialize all columns to OFF
  digitalWrite(ICN_A0, LOW); // Bit 0
  digitalWrite(ICN_A1, LOW); // Bit 1
  digitalWrite(ICN_A2, LOW); // Bit 2
  digitalWrite(HC595_OE, LOW);

  fillCurrentMatrix(BLACK);
}

void setup() {
  Serial.begin(115200);

  setupPinModes();
  delay(10);
  setInitialValues();
  setCurrentMatrix(HEART);

  delay(1000);
}

// each loop goes through one multiplex channel
// renders current RGB values for channel
// checks inputs on current channel
// if inputs caught, updates the RGB value of the corresponding pixel
uint8_t channel = 0;
uint8_t loopCount = 0;

void loop() {

  if (isButtonInMultiplexPressed(channel, 200)) {// simplistic means of checking switching between images
    switch(channel) {
      case 0:
        setCurrentMatrix(HEART);
        break;
      case 1:
        setCurrentMatrix(HEART_SMALL);
        break;
      case 2:
        setCurrentMatrix(TEXT_MAYBE);
        break;
      case 3:
        setCurrentMatrix(TIME_1);
        break;
      case 4:
        setCurrentMatrix(TIME_2);
        break;
      default:
        setCurrentMatrix(OFF);
        break;
    }
  }
  for (int i = 0; i < 4; i++) {
      setMuxChannel(channel);
      delayMicroseconds(10);
      renderChannel(channel);
      delayMicroseconds(5);
      hc595Write(CHANNEL_OFF); // reduce ghosting on neighboring pixels
      delayMicroseconds(2);
  }
  channel++;
  channel = channel % 6;
}
