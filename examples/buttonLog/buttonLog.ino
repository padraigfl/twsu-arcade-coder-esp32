#include <TWSUArcadeCoder.h>

/**
  NOT FULLY OPERATIONAL
  This example should update the RGB value of a button when it is pressed
  Currently it sort of does that sometimes
*/

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
  // home button input
  pinMode(HOME_BTN, INPUT);
  // status LEDs
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  // setup matrix inputs
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    pinMode(matrixInputs[i], INPUT);
  }
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

  delay(1000);
}

int homeButtonValidator = 0;

uint8_t indexToChange = 0;
uint8_t row = 0;
uint8_t col = 0;

// each loop goes through one multiplex channel
// renders current RGB values for channel
// checks inputs on current channel
// if inputs caught, updates the RGB value of the corresponding pixel
uint8_t channel = 0;
uint8_t buttonLogIterator = 0;

uint8_t getNextColor(uint8_t color) {
  switch(color) {
    case BLACK:
      return RED;
    case RED:
      return GREEN;
    case GREEN:
      return BLUE;
    case BLUE:
      return CYAN;
    case CYAN:
      return MAGENTA;
    case MAGENTA:
      return YELLOW;
    case YELLOW:
      return WHITE;
    case WHITE:
    default:
      return BLACK;
  }
}

void loop() {
  for (channel = 0; channel < 6; channel++) {
    setMuxChannel(channel);
    delayMicroseconds(100);
    renderChannel(channel);
    delayMicroseconds(200);
    hc595Write(CHANNEL_OFF);
    delayMicroseconds(2);
  }

  for (channel = 0; channel < 6; channel++) {
    if (isButtonInMultiplexPressed(channel, 200)) {
      Serial.print("Buttons pressed: ");
      for (buttonLogIterator = 0; buttonLogIterator < CHANNEL_ENTRIES; buttonLogIterator++) {
        if (isButtonPressed(channel, buttonLogIterator, 200)) {
          indexToChange = buttonLogIterator < 12
            ? (channel * 12) + buttonLogIterator
            : (channel * 12) + buttonLogIterator + 60;
          row = buttonLogIterator < 12 ? channel : channel + 6;
          col = indexToChange % 12;
          Serial.printf("R%02dC%02d:%03d,", row, col, indexToChange);
          currentMatrix[indexToChange] = getNextColor(currentMatrix[indexToChange]);
        }
      }
      refreshFields();
      Serial.println();
      for (buttonLogIterator = 0; buttonLogIterator < 9; buttonLogIterator++) {
        Serial.printf("%03u,", rawLedData[channel][buttonLogIterator]);
      }
      Serial.println();
    }
  }
  homeButtonAction();
}


// hijacks code to verify home button is being pressed, needs a debouncer for being released
void homeButtonAction() {
  // !! gpio2 behaviour is buggy, possibly wrong pin or bad board?
  // // use home button to jump through settings
  // while(analogRead(2) == 0 && homeButtonValidator < 20) {
  //   if (homeButtonValidator == 0)
  //     Serial.print("Home button pressed");
  //     Serial.print(".");
  //     delay(20);
  //     homeButtonValidator++;
  //   }
  //   if (homeButtonValidator > 20) {
  //    // homeButtonActionGoesHere
  //   }
  // homeButtonValidator = 0;
}