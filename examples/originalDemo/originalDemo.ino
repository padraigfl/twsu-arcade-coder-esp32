// This was the original demo for verifying various behaviours and is entirely self contained with no library dependence

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

uint8_t off[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255 };

// indexes on basis of USB port at bottom of board
// viewed from front (i.e. top leff to bottom right)
uint8_t indexing[] = {
  11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  15,14,13,12,23,22,21,20,19,18,17,16
};

// scans through pixels to find which buttons are pressed
void renderOnePixel(uint8_t renderIdx){
  uint8_t arr[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  uint8_t updateIdx = 25;
  for (int i = 0; i < 24; i++) {
    if (indexing[i] == renderIdx) {
      updateIdx = i;
      break;
    }
  }
  // we need to update the red pixels
  uint8_t arrayIdx = 
    updateIdx < 8
      ? 1
      : updateIdx < 16
        ? 4
        : 7;

  uint8_t bitToUpdate = (updateIdx % 8) + 1;
  arr[arrayIdx] = (0b11111111 >> bitToUpdate) | (0b11111111 << (9 - bitToUpdate));
  if (updateIdx > 23) {
    Serial.printf("Invalid specification");
    return;
  }
  hc595Write(arr, 9);
}


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
bool useMatrixInputs = true; // read input values

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
  // as we cannot determine both channel and index of pressed button, we may as well only check once per full cycle
  if (useMatrixInputs && icn % 8 == 1) {
    buttonRead();
  }

  homeButtonAction();
  loopCount++;
}

/* 
  Pins 32-36 and 39 record if buttons have been pressed
  If a button has been pressed, it's possible to verify it via some scans of red matrixes and checks of the relative pin value across multiple channels
  
  - Can read if a button is pressed on a given multiplexed group
  - Can determine the column of the pressed button
  - Can determine grouping of pressed button
  - Can approximate multiple buttons which have been pressed (introduces unreliability which grows when more buttons are pressed)

  TODO: find minimum delays and minimise amount of visual noise
*/
void buttonRead() { // button matrix values seem to be
  if (icn % 8 == 0 || icn % 8 == 7) {
    // we can only determine the pressed button if icn is in 6 channel multiplexing range
    return;
  }
  int inputsCount = sizeof(matrixInputs) / intSize;
  bool pinPress[inputsCount] = { false };
  bool pressPause = false; // visual feedback that at least one button has been held down
  for (int i = 0; i < inputsCount; i++) {
    digitalWrite(HC595_LAT, LOW); // needs to be low to get reliable values
    delayMicroseconds(50);
    uint16_t btnVal = analogRead(matrixInputs[i]);
    if (btnVal > 100) {
      // Serial.printf("Btn: %u; icn: %d, cnt: %d\n", btnVal, icn % 8, buttonRangeCounter[i]);
      pinPress[i] = true;
      buttonRangeCounter[i]++;
      // OE low and iterating through these actions seems to observe whether a button has been pressed
      // as ICN channels 0 and 7 (all high, all low) only register button pressed on LEDs which aren't lit up
      if (buttonRangeCounter[i] > 10) {
        for (int pixel = 0; pixel < 24; pixel++) {
          renderOnePixel(pixel);
          digitalWrite(HC595_LAT, LOW); 
          digitalWrite(HC595_OE, LOW); // Enable HC595 outputs (active low)
          delayMicroseconds(20);
          int nextBtnVal = analogRead(matrixInputs[i]);
          if (nextBtnVal > 400) { // TODO refine thresholds
            setICN2012Column(7);
            delayMicroseconds(20);
            int validatedBtnVal = analogRead(matrixInputs[i]);
            if ((nextBtnVal - validatedBtnVal) > 600) { // this will get less reliable the more buttons that are pressed
              Serial.printf("%d on %d has been pressed.|", pixel, i);
            }
            setICN2012Column(icn);
            hc595Write(off, 9);
            delayMicroseconds(250);
          }
          digitalWrite(HC595_OE, HIGH);
        }
      }
    } else {
      buttonRangeCounter[i] = 0;
    }
  }
  Serial.printf("\n");
  if (pressPause) {
    delay(1000);  Serial.printf("  %d:  ", (icn % 6) + 1);
    Serial.println();
  }
  digitalWrite(HC595_LAT, HIGH);

  // return pinPress;
}

void refreshMatrix() {
  // iterates through multiplexed rows
  setICN2012Column(++icn);
  delayMicroseconds(10);
  hc595Write(oneRed, 9);
}

// TODO replace function with RGB custom shift register function so full matrix can be animated
void hc595Write(uint8_t data[], int num_chips) { // Takes an array of bytes
  digitalWrite(HC595_OE, LOW); // Enable HC595 outputs (active low)
  digitalWrite(HC595_LAT, LOW); // Prepare for data transfer
  for (int i = 0; i < num_chips; i++) { // Iterate through all the chips
    shiftOut(HC595_DATA, HC595_CLK, MSBFIRST, data[i]); // Send data for each chip
  }
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