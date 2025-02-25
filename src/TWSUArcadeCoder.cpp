#include <Arduino.h>
#include "TWSUArcadeCoder.h"

/* Util values */
uint8_t i_8 = 0;
uint8_t j_8 = 0;
uint8_t latVal = 0;
uint8_t oeVal = 0;
uint8_t a0 = 0;
uint8_t a1 = 0;
uint8_t a2 = 0;
uint16_t btnVal = 0;
uint16_t validatedBtnVal = 0;

/**
  CORE OPERATIONS
*/

// the physical wiring of LEDs to shift registers; two rows of 12, top left = 0, bottom right = 23
uint8_t indexing[] = {
  11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
  15,14,13,12,23,22,21,20,19,18,17,16
};

// dumps all 72 bit of data off the shift register at once
void hc595Write(uint8_t data[]) { // Takes an array of bytes
  digitalWrite(HC595_OE, LOW); // Enable HC595 outputs (active low)
  digitalWrite(HC595_LAT, LOW); // Prepare for data transfer
  for (int i = 0; i < HC595_CNT; i++) { // Iterate through all the chips
    shiftOut(HC595_DATA, HC595_CLK, MSBFIRST, data[i]); // Send data for each chip
  }
  digitalWrite(HC595_LAT, HIGH); // Latch the data after all bytes are sent
  digitalWrite(HC595_OE, HIGH); // Disable HC595 outputs to prevent ghosting
}

// sets the values on the ICN2012/LS138 chip to determine the multiplex channel
void setMuxChannel(int stage) {
  // digitalWrite(E2, HIGH);
  // initial PIN values
  a0 = LOW; a1 = LOW; a2 = LOW; // reset values
  if (stage == 5)      { a1 = HIGH; a2 = HIGH; } // bottom row
  else if (stage == 4) { a0 = HIGH; } // 2: second from bottom?
  else if (stage == 3) { a2 = HIGH; } // 3: 3rd from bottom
  else if (stage == 2) { a0 = HIGH; a2 = HIGH; } // 4: 3rd from top
  else if (stage == 1) { a0 = HIGH; a1 = HIGH; } // 5: second from top
  else if (stage == 0) { a1 = HIGH; }  // 6: top row
  else if (stage == 7) { a0 = HIGH; a1 = HIGH; a2 = HIGH; }
  digitalWrite(ICN_A0, a0); digitalWrite(ICN_A1, a1); digitalWrite(ICN_A2, a2);
  // digitalWrite(E2, LOW);
}

/**
  DISPLAY LOGIC
*/

uint8_t currentMatrix[144] = {};
uint8_t multiplexedGroups[6][24] = {
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
  {0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff},
};
uint8_t rawLedData[6][9] = {
  RAW_BLACK_DATA,
  RAW_BLACK_DATA,
  RAW_BLACK_DATA,
  RAW_BLACK_DATA,
  RAW_BLACK_DATA,
  RAW_BLACK_DATA,
};

uint8_t CHANNEL_OFF[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255 };
const uint8_t matrixInputs[] = { 39, 36, 35, 34, 33, 32 }; // all register values when pressed  // ,32, 33, 34, 35, 36, 39

uint8_t testMatrix[144] = {
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  RED, RED, RED, RED, RED, RED, RED, RED, BLACK, BLACK, GREEN, GREEN,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
  GREEN, GREEN, BLACK, BLACK, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE, BLUE,
};

// Prints the current matrix in a format that can be easily parsed in the terminal
void printMatrix(uint8_t mtrx[144]) {
  printf("matrix\n");
  char cell = '_';
  int bashColor = 0;
  for (i_8 = 0; i_8 < 144; i_8++) {
    cell = '_';
    bashColor = 0;
    if (mtrx[i_8] == RED) {
      cell = 'R'; bashColor = 31;
    } else if (mtrx[i_8] == GREEN) {
      cell = 'G'; bashColor = 32;
    } else if (mtrx[i_8] == BLUE) {
      cell = 'B'; bashColor = 34;
    } else if (mtrx[i_8] == MAGENTA) {
      cell = 'M'; bashColor = 35;
    } else if (mtrx[i_8] == YELLOW) {
      cell = 'Y'; bashColor = 33;
    } else if (mtrx[i_8] == CYAN) {
      cell = 'C'; bashColor = 36;
    } else if (mtrx[i_8] == WHITE) {
      cell = 'W';
    } else if (mtrx[i_8] == BLACK) {
      cell = ' ';
    }
    printf("\e[%dm%c|\e[0m", bashColor, cell);
    if (i_8 % 12 == 11) {
      printf("\n");
    }
  }
}

uint8_t indexValue = 0;
uint8_t relevantIndex = 0;
uint8_t channel_8 = 0;
void refreshFields() {
  for (channel_8 = 0; channel_8 < CHANNEL_COUNT; channel_8++) {
    for (j_8 = 0; j_8 < CHANNEL_ENTRIES; j_8++) {
      indexValue = indexing[j_8]; // need to correct for ordering not being linear
      relevantIndex = (channel_8 * 12) + indexValue;
      if (indexValue > 11) {
        relevantIndex+=60;
      }
      Serial.printf("Index: %d;%d;%d,%d", relevantIndex, channel_8, j_8, CHANNEL_COUNT);
      multiplexedGroups[channel_8][j_8] = currentMatrix[relevantIndex];
    }
    Serial.println();
    getChannelLedData(channel_8);
  }
}

// Updates the "currentMatrix" value for the display
void setCurrentMatrix(uint8_t val[144]) {
  for (i_8 = 0; i_8 < 144; i_8++) {
    currentMatrix[i_8] = val[i_8];
  }
  printMatrix(currentMatrix);
  refreshFields();
}
void fillCurrentMatrix(uint8_t val) {
  uint8_t mtrx[144] = {};
  for (i_8 = 0; i_8 < 144; i_8++) {
    mtrx[i_8] = val;
  }
  setCurrentMatrix(mtrx);
}

uint8_t byteStartPoint = 0;
uint8_t relevantBit = 0;
uint8_t green = 0;
uint8_t red = 0;
uint8_t blue = 0;
uint8_t dataStartIdx = 0;
uint8_t byteIdx = 0;
uint8_t led = 0;
void getChannelLedData(uint8_t channel) {
  // uint8_t *output = (uint8_t*)malloc( sizeof(uint8_t) * HC595_CNT);
  for (i_8 = 0; i_8 < HC595_CNT; i_8++) {
    rawLedData[channel][i_8] = 0;
  }
  for (byteIdx = 0; byteIdx < 3; byteIdx++) { // each LED is spread across 3 shift registers at the same index
    dataStartIdx = byteIdx * 8; // the point in the multiplexed group to begin from
    for (led = 0; led < 8; led++) { // meaning 3 3-byte groupings of related data (1 bit per LED in each entry)
      byteStartPoint = byteIdx * 3;
      relevantBit = multiplexedGroups[channel][dataStartIdx + led];
      green = ((uint8_t)(relevantBit << 5)) >> 7;
      red =   ((uint8_t)(relevantBit << 6)) >> 7;
      blue =  ((uint8_t)(relevantBit << 7)) >> 7;
      rawLedData[channel][byteStartPoint] |= (green << (7 - led));
      rawLedData[channel][byteStartPoint + 1] |= (red << (7 - led));
      rawLedData[channel][byteStartPoint + 2] |= (blue << (7 - led));
    }
  }
}

void renderChannel(uint8_t channel) {
  hc595Write(rawLedData[channel]);
}

// int main() {
//   uint8_t arr[HC595_CNT] = {};
//   fillCurrentMatrix(CYAN);
//   getChannelLedData(arr, 0);

//   printf("\nLeading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(arr[3]));
//   printf("\nLeading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(arr[4]));
//   printf("\nLeading text "BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(arr[5]));
// }


//////// INPUT


// scans through pixels to find which buttons are pressed
uint8_t onePixelArr[9] = {};
uint8_t updateIdx = 25;
uint8_t bitToUpdate = 0;
void renderOnePixel(uint8_t renderIdx){
  for (i_8 = 0; i_8 < HC595_CNT; i_8++) {
    onePixelArr[i_8] = 255;
  }
  for (i_8 = 0; i_8 < 24; i_8++) {
    if (indexing[i_8] == renderIdx) {
      updateIdx = i_8;
      break;
    }
  }
  // we need to update the red pixels
  bitToUpdate = (updateIdx % 8) + 1;
  onePixelArr[
    updateIdx < 8
      ? 1
      : updateIdx < 16
        ? 4
        : 7
  ] = (0b11111111 >> bitToUpdate) | (0b11111111 << (9 - bitToUpdate));
  // if (updateIdx > 23) {
  //   printf("Invalid specification");
  //   return;
  // }
  hc595Write(onePixelArr);
}

// check if a button within a multiplexed group has been pressed
bool isButtonInMultiplexPressed(uint8_t channel, uint16_t threshold) {
  latVal = digitalRead(HC595_LAT); // get previous state

  digitalWrite(HC595_LAT, LOW); // needs to be low to get reliable values
  delayMicroseconds(20);
  btnVal = analogRead(matrixInputs[channel]);
  
  digitalWrite(HC595_LAT, latVal); // restore previous state
  return btnVal > threshold;
};

// checks if a given button has been pressed
// threshold will vary depending on number of buttons pressed
bool buttonPressed = false;
bool isButtonPressed(uint8_t channel, uint8_t index, uint16_t threshold) {
  latVal = digitalRead(HC595_LAT);
  oeVal = digitalRead(HC595_OE);
  a0 = digitalRead(ICN_A0);
  a1 = digitalRead(ICN_A1);
  a2 = digitalRead(ICN_A2);

  setMuxChannel(channel);
  renderOnePixel(index);
  digitalWrite(HC595_LAT, LOW); // needs to be low to get button data
  digitalWrite(HC595_OE, LOW);
  delayMicroseconds(2);
  btnVal = analogRead(matrixInputs[channel]);
  buttonPressed = false;
  if (btnVal > threshold) { // TODO refine thresholds
    // Switch to HIGH/HIGH/HIGH to validate button input
    setMuxChannel(7);
    delayMicroseconds(2);
    validatedBtnVal = analogRead(matrixInputs[channel]);
    if ((btnVal - validatedBtnVal) > 400) { // TODO optimise thresholds, this check is mainly for catching multiple buttons in a channel at once
      buttonPressed = true;
    }
    setMuxChannel(channel);
    delayMicroseconds(2);
    hc595Write(CHANNEL_OFF); // clear matrix
    delayMicroseconds(2);
    hc595Write(CHANNEL_OFF); // clear matrix
    delayMicroseconds(2);
  }

  // return to previous values
  digitalWrite(HC595_LAT, latVal);
  digitalWrite(HC595_OE, oeVal);
  digitalWrite(ICN_A0, a0);
  digitalWrite(ICN_A1, a1);
  digitalWrite(ICN_A2, a2);
  return buttonPressed;
}

// slightly faster means of scanning all buttons within a channel at once
bool* multiplexButtonsPressed(uint8_t channel, uint16_t threshold) {
  bool buttonsStatus[24] = {};
  for (i_8 = 0; i_8 < 24; i_8++) {
    buttonsStatus[i_8] = isButtonPressed(channel, i_8, threshold);
  }
  return buttonsStatus;
}
