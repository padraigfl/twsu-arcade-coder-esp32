
// // Notes
// - Swap HC595_OE to another pin to use wifi (e.g. pin 22 or 23 are safe to reuse as functionality is clear)
// - Make some visual demos

/* Matrix color variables */

// individual color codes
const uint8_t WHITE = 0b000;
const uint8_t BLACK = 0b111;
const uint8_t GREEN = 0b011;
const uint8_t RED   = 0b101;
const uint8_t BLUE  = 0b110;
const uint8_t YELLOW= 0b001;
const uint8_t PURPLE= 0b100;
const uint8_t CYAN  = 0b010;

uint8_t oneRed[] = {
  0b11111111,
  0b01111111,
  0b11111111,
  255, 255, 255, 255, 255, 255
};
uint8_t oneWhite[] = {
  0b01111111,
  0b01111111,
  0b01111111,
  255, 255, 255, 255, 255, 255
};
uint8_t oneBlack[] = {
  0b1000000,
  0b1000000,
  0b1000000,
  0,0,0,0,0,0
};

uint8_t whiteRed[] = {
  0b01010101,
  0b00000000,
  0b01010101,
  255,
  255,
  255,
  255,
  255,
  255,
};
uint8_t redWhite[] = {
  0b10101010,
  0b00000000,
  0b10101010,
  255,
  255,
  255,
  255,
  255,
  255,
};
// exact bit data passed into HC595 chain to output these colurs
uint8_t greenAll[] = {
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
};
uint8_t redAll[] = {
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111
};
uint8_t blueAll[] = {
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000,
  0b11111111,
  0b11111111,
  0b00000000
};
uint8_t rgbPattern[] = {
  0b01101101,// 12A/11A/10A/9A/8A/7A/6A/5A/
  0b10110110,
  0b11011011,
  0b10111011,//  4A/ 3A/ 2A/1A/4B/3B/2B/1B/
  0b11011101,
  0b01100110,
  0b01101101,// 12B/11B/10B/9B/8B/7B/6B/5B
  0b10110110,
  0b11011011
};
uint8_t fullRange[] = {
  0b01001011,
  0b00101101,
  0b00010111,
  0b01001011,
  0b00101101,
  0b00010111,
  0b01001011,
  0b00101101,
  0b00010111,
};
uint8_t off[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255 };

// indexes on basis of USB port at bottom of board
// viewed from front (i.e. top leff to bottom right)
uint8_t indexing[] = {
  11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  16,17,18,19,20,21,22,23,15,14,13,12
};

/* Pin allocation */
const int HOME_BTN = 2; 
const int LED_1 = 22;
const int LED_2 = 23;
// HC595 Pins
const int HC595_DATA = 5;    // GPIO5 (Data) // DS
const int HC595_CLK = 17; // GPIO17 (Clock) // SHCP_PIN
const int HC595_LAT = 16; // GPIO16 (Latch) // STCP_PIN
const int HC595_OE = 4;    // GPIO4 (Output Enable, LOW = enable) // THIS MAY BREAK WIFI
// ICN2012 Pins, I haven't been able to find out if pin E2 has a function (E1 is directly connected to 3.3v)
const int ICN_A0 = 19;   // GPIO19 (Column Address Bit 0)
const int ICN_A1 = 18;   // GPIO18 (Column Address Bit 1)
const int ICN_A2 = 21;   // GPIO21 (Column Address Bit 2)

const int unknownPins[] = {
  12, 13, 14, 15, 25, 26, 27
}; // 26 and 27 read as 4095 (possibly the accelerometer?), pin 25 to ground triggers reset in stock firmware
const int possiblyUnused[] = {12, 13, 14, 25 }; // no traces observed, potentially usable
const int inputOnly[] = { 34, 33, 35, 36, 39 }; // all (plus 32) appear to be connected, occasional drops to 2600
const int matrixInputs[] = { 39, 36, 35, 34, 33, 32 }; // all register values when pressed

// utility values
const int intSize = sizeof(int);

/* process trackers */
uint32_t loopCount = 0; // number of loops processed
uint8_t icn = 1; // current stage of multiplexing
uint8_t homeButtonPressCount = 0; // for verifying if the home button is pressed as value is unstable

bool display = true; // display matrix
bool useMatrixInputs = true; // read input values (does not work if matrix is on, not sure why)
bool autoSwap = false; // switches between modes dynamically to debug multiple things

int buttonRangeCounter[6] = { 0, 0, 0, 0, 0, 0 };

void setup() {
  Serial.begin(115200);
  // Configure HC595 pins
  pinMode(HC595_DATA, OUTPUT);
  pinMode(HC595_CLK, OUTPUT);
  pinMode(HC595_LAT, OUTPUT);
  pinMode(HC595_OE, OUTPUT); // possible switch to ledcAttachChannel?
  digitalWrite(HC595_OE, LOW);
  // Configure ICN2012 pins
  pinMode(ICN_A0, OUTPUT);
  pinMode(ICN_A1, OUTPUT);
  pinMode(ICN_A2, OUTPUT);
  // Initialize all columns to OFF
  digitalWrite(ICN_A0, LOW); // Bit 0
  digitalWrite(ICN_A1, LOW); // Bit 1
  digitalWrite(ICN_A2, LOW); // Bit 2

  // home button input
  pinMode(HOME_BTN, INPUT);
  // statuus LEDs
  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);

  if (useMatrixInputs) {
    for (int i = 0; i < sizeof(matrixInputs) / intSize; i++) {
      pinMode(matrixInputs[i], INPUT);
    }
  }
  Serial.println("Starting1");

  if (display) {
    Serial.println("Matrix output enableddd and more stuff too");
  }
  if (useMatrixInputs) {
    Serial.println("Button inputs debugging enabled");
  }
  delay(1000);
}

int homeButtonValidator = 0;
void loop() {
  if (display) {
    refreshMatrix();
  }
  if (useMatrixInputs && icn % 8 == 2 ) {
    buttonRead();
  }

  if (autoSwap) { autoModeSwap(); }
  homeButtonAction();
  loopCount++;
}

/* 
  Can read if a button is pressed on a given multiplexed group
  Can somewhat gauge if multiple are pressed (higher analog values for more buttons)
  Cannot determine which of the 24 buttons has been pressed
*/
void buttonRead() { // button matrix values seem to be
  int inputsCount = sizeof(matrixInputs) / intSize;
  bool pinPress[inputsCount] = { false };
  digitalWrite(HC595_LAT, LOW); // needs to be low to get reliable values
  delayMicroseconds(50);
  bool pressPause = false; // visual feedback that at least one button has been held down
  for (int i = 0; i < inputsCount; i++) {
    uint16_t btnVal = analogRead(matrixInputs[i]);

    for (int k = 0; k < 9; k++) {
      btnVal += analogRead(matrixInputs[i]);
      delayMicroseconds(10);
    }
    if (btnVal > 100) {
      pinPress[i] = true;
      buttonRangeCounter[i]++;
    }

    btnVal = static_cast<int>(std::round(btnVal *0.1));

    // OE low and iterating through these actions seems to observe whether a button has been pressed on IC channels 0 and 7
    digitalWrite(HC595_OE, LOW); // Enable HC595 outputs (active low)
    if (buttonRangeCounter[i] > 50) {
      for (int ic = 0; ic < 8; ic++) {
        setICN2012Column(ic);
        for (int c = 0; c < 8; c++) {
          digitalWrite(HC595_DATA, (0b01111111 >> i & 0b11111111));
          for (int clk = 0; clk < 8; clk++) {
            digitalWrite(HC595_CLK, HIGH);
            digitalWrite(HC595_CLK, LOW);
          }
          int nextBtnVal = analogRead(matrixInputs[i]);
          delay(2);
          if (nextBtnVal > 0) {
            Serial.printf("index: %d, icn: %d, btnVal: %04u\n", c, ic, nextBtnVal);
          }
        }
        buttonRangeCounter[i] = 0;
        int pixelRange1 = (int) (5 - i) * 12;
        int pixelRange2 = pixelRange1 + 72;
        pressPause = true;
      }
      // Serial.printf(
      //   "\nA button in multiplex group %d (%d - %d, %d - %d ) has been pressed\n",
      //   6 - i,
      //   pixelRange1, pixelRange1 + 12,
      //   pixelRange2, pixelRange2 + 12
      // );
    }
  }
  if (pressPause) {
    delay(1000);  Serial.printf("  %d:  ", (icn % 6) + 1);
  Serial.println();
  }
  digitalWrite(HC595_LAT, HIGH); // needs to be low to get reliable values

  // return pinPress;
}

void refreshMatrix() {
  // iterates through multiplexed rows
  setICN2012Column(icn++);
  hc595Write(oneRed, 9);
}

// TODO replace function with RGB custom shift register function so full matrix can be animated
void hc595Write(uint8_t data[], int num_chips) { // Takes an array of bytes
  digitalWrite(HC595_OE, LOW); // Enable HC595 outputs (active low)
  digitalWrite(HC595_LAT, LOW); // Prepare for data transfer
  for (int i = 0; i < num_chips; i++) { // Iterate through all the chips
    shiftOut(HC595_DATA, HC595_CLK, MSBFIRST, data[i]); // Send data for each chip
  }
  delayMicroseconds(2);
  digitalWrite(HC595_LAT, HIGH); // Latch the data after all bytes are sent
  digitalWrite(HC595_OE, HIGH); // Disable HC595 outputs to prevent ghosting
}


// TODO investigate ditching all LOW and all HIGH states
void setICN2012Column(int val) {
  // digitalWrite(E2, HIGH);
  // initial PIN values
  uint8_t a0 = LOW; uint8_t a1 = LOW; uint8_t a2 = LOW;
  int stage = (val % 8);
  if (stage == 1)      { a1 = HIGH; a2 = HIGH; } // bottom row
  else if (stage == 2) { a0 = HIGH; } // 2: second from bottom?
  else if (stage == 3) { a2 = HIGH; } // 3: 3rd from bottom
  else if (stage == 4) { a0 = HIGH; a2 = HIGH; } // 4: 3rd from top
  else if (stage == 5) { a0 = HIGH; a1 = HIGH; } // 5: second from top
  else if (stage == 6) { a1 = HIGH; }  // 6: top row
  else if (stage == 7) { a0 = HIGH; a1 = HIGH; a2 = HIGH; }
  digitalWrite(ICN_A0, a0); digitalWrite(ICN_A1, a1); digitalWrite(ICN_A2, a2);
  // digitalWrite(E2, LOW);
}

void autoModeSwap() {
  // if (autoSwap && loopCount % 1000 == 0) {
  //   if (display && useMatrixInputs) {
  //     display = true;
  //     useMatrixInputs = false;
  //     Serial.println("Display only");
  //   } else if (display) {
  //     display = false;
  //     useMatrixInputs = true;
  //     Serial.println("Inputs only");
  //   } else {
  //     display = true;
  //     Serial.println("Attempting both");
  //   }
  //   delay(1000);
  // }
}

// hijacks code to verify home button is being pressed, needs a debouncer for being released
void homeButtonAction() {
  // !! gpio2 behaviour is buggy, possibly wrong pin or bad board?
  // // use home button to jump through settings
  // while(analogRead(2) == 0 && homeButtonValidator < 20) {
  //   if (homeButtonValidator == 0)
  //     Serial.print("Home button pressed");
  //   Serial.print(".");
  //   delay(20);
  //   homeButtonValidator++;
  // }
  // if (homeButtonValidator > 20) {
  // // homeButtonActionGoesHere
  // }
  // homeButtonValidator = 0;
}