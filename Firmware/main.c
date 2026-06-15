////////////////using blynk////////////////////////////

#define BLYNK_PRINT Serial
#include <WiFi.h>
// #include <arduino.h>

//-------- Blynk Credentials --------
#define BLYNK_TEMPLATE_ID " BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "BLYNK_TEMPLATE_NAME "
#define BLYNK_DEVICE_NAME " BLYNK_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN " BLYNK_AUTH_TOKEN"

#include <BlynkSimpleEsp32.h>

//-------- WiFi Credentials --------
char ssid[] = "Wokwi-GUEST";
char pass[] = "";

//-------- Variables --------
#define MAX_BINARY_COLS 64
byte binaryCols[MAX_BINARY_COLS];
int binaryColCount = 0;
bool binaryMode = false;
bool textmode = true;
bool displayEnabled = false;
String msg = "HELLO ";
int scrollspeed = 15;
bool msg_changed = false;

// Columns(anodes)
int clockPin1 = 18;
int latchPin1 = 5;
int dataPin1 = 23;

// Rows(cathodes)
int clockPin2 = 14;
int latchPin2 = 27;
int dataPin2 = 19;

// BITMAP
byte bitmap[8][2];
int numZones = sizeof(bitmap) / 8; // one zone=8X8 matrix
int maxZoneIndex = numZones - 1;
int numCols = numZones * 8;

int refresh_time = 300;

// FONT
byte alphabets[][8] = {
    {0, 0, 0, 0, 0}, //@ as SPACE
    //{8,28,54,99,65},//<<
    {31, 36, 68, 36, 31},   // A
    {127, 73, 73, 73, 54},  // B
    {62, 65, 65, 65, 34},   // C
    {127, 65, 65, 34, 28},  // D
    {127, 73, 73, 65, 65},  // E
    {127, 72, 72, 72, 64},  // F
    {62, 65, 65, 69, 38},   // G
    {127, 8, 8, 8, 127},    // H
    {0, 65, 127, 65, 0},    // I
    {2, 1, 1, 1, 126},      // J
    {127, 8, 20, 34, 65},   // K
    {127, 1, 1, 1, 1},      // L
    {127, 32, 16, 32, 127}, // M
    {127, 32, 16, 8, 127},  // N
    {62, 65, 65, 65, 62},   // O
    {127, 72, 72, 72, 48},  // P
    {62, 65, 69, 66, 61},   // Q
    {127, 72, 76, 74, 49},  // R
    {50, 73, 73, 73, 38},   // S
    {64, 64, 127, 64, 64},  // T
    {126, 1, 1, 1, 126},    // U
    {124, 2, 1, 2, 124},    // V
    {126, 1, 6, 1, 126},    // W
    {99, 20, 8, 20, 99},    // X
    {96, 16, 15, 16, 96},   // Y
    {67, 69, 73, 81, 97},   // Z
};

// Clear bitmap
void clear()
{
  for (int row = 0; row < 8; row++)
  {
    for (int zone = 0; zone <= maxZoneIndex; zone++)
    {
      bitmap[row][zone] = 0;
    }
  }
}

void on()
{
  for (int row = 0; row < 8; row++)
  {
    for (int zone = 0; zone <= maxZoneIndex; zone++)
    {
      bitmap[row][zone] = 0xFF;
    }
  }
}

// Displays bitmap array in the matrix
void RefreshDisplay()
{
  for (int row = 0; row < 8; row++)
  { // blank all rows
    digitalWrite(latchPin2, LOW);
    shiftOut(dataPin2, clockPin2, MSBFIRST, 0xFF);
    digitalWrite(latchPin2, HIGH);

    int rowbit = 1 << row;
    digitalWrite(latchPin2, LOW); // for transmitting data
    shiftOut(dataPin2, clockPin2, MSBFIRST, ~rowbit);

    // Start sending column bytes
    digitalWrite(latchPin1, LOW);

    // Shift out to each matrix
    for (int zone = maxZoneIndex; zone >= 0; zone--)
    {
      shiftOut(dataPin1, clockPin1, MSBFIRST, bitmap[row][zone]);
    }
    digitalWrite(latchPin1, HIGH);
    digitalWrite(latchPin2, HIGH);
    delayMicroseconds(refresh_time);
  }
}

//-------- Blynk Text Input --------
BLYNK_WRITE(V1)
{
  clear();
  RefreshDisplay();
  binaryMode = false;
  textmode = true;
  scrollspeed = 15;
  refresh_time = 300;
  msg = param.asStr();
  msg.toUpperCase();
  msg += " ";
  msg_changed = true;
}

void setup()
{
  pinMode(latchPin1, OUTPUT);
  pinMode(clockPin1, OUTPUT);
  pinMode(dataPin1, OUTPUT);
  pinMode(latchPin2, OUTPUT);
  pinMode(clockPin2, OUTPUT);
  pinMode(dataPin2, OUTPUT);
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  clear();
}

void parseBinaryText(String input)
{
  input.replace(" ", "");
  input.replace("\n", ",");
  String rows[8];
  int rowCount = 0;
  int start = 0;

  // Split rows
  while (rowCount < 8)
  {
    int comma = input.indexOf(',', start);
    if (comma == -1)
      rows[rowCount++] = input.substring(start);
    else
      rows[rowCount++] = input.substring(start, comma);

    if (comma == -1)
      break;
    start = comma + 1;
  }

  if (rowCount != 8)
    return;

  int cols = rows[0].length();
  if (cols > MAX_BINARY_COLS)
    cols = MAX_BINARY_COLS;

  binaryColCount = cols;

  // Convert rows → columns
  for (int col = 0; col < cols; col++)
  {
    byte value = 0;
    for (int row = 0; row < 8; row++)
    {
      if (rows[row].charAt(col) == '1')
        bitSet(value, 7 - row);
    }
    binaryCols[col] = value;
  }

  binaryMode = true;
  textmode = false;
  msg_changed = true;
}

//---------Blynk binary no. input----------
BLYNK_WRITE(V2)
{
  String input = param.asStr();
  parseBinaryText(input);
  msg_changed = true;
  textmode = false;
  binaryMode = true;
  clear();
  RefreshDisplay();
  scrollspeed = 22;
  refresh_time = 700;
}

//-------- Blynk Slider (Scroll Speed) --------
BLYNK_WRITE(V3)
{
  scrollspeed = 56 - param.asInt();
}

// Converts row and colum to bitmap bit and turn it off/on
void Plot(int col, int row, bool isOn)
{
  int zone = col / 8;
  int colBitIndex = col % 8;
  byte colBit = 1 << colBitIndex;
  if (isOn)
    bitmap[row][zone] = bitmap[row][zone] | colBit;
  else
    bitmap[row][zone] = bitmap[row][zone] & (~colBit);
}

//-------- Blynk Button (ON/OFF) --------
bool started = false;
BLYNK_WRITE(V0)
{
  displayEnabled = param.asInt();

  if (!displayEnabled)
  {
    started = false;
    clear();
    RefreshDisplay();
  }
  else
  {
    if (!started)
    {
      started = true;
      refresh_time = 100;
      scrollspeed = 10;
      clear();
      for (int row = 0; row < 8; row++)
      {
        for (int col = 0; col < numCols; col++)
        {
          Plot(col, row, true);
        }
        for (int i = 0; i < 50; i++)
          RefreshDisplay();
      }
      refresh_time = 300;
      for (int i = 0; i < 4; i++)
      {
        on();
        for (int refreshCount = 0; refreshCount < 30; refreshCount++)
          RefreshDisplay();

        clear();
        for (int refreshCount = 0; refreshCount < 30; refreshCount++)
          RefreshDisplay();
      }
    }
  }
}

// Plot each character of the message one column at a time, updated the display, shift bitmap left.
void XProcess()
{
  if (binaryMode)
  {
    int binarycol = binaryColCount;
    // Draw one character of the message
    for (int col = 0; col < binarycol; col++)
    {
      for (int row = 0; row < 8; row++)
      {
        bool isOn = 0;
        if (col < binarycol)
          isOn = bitRead(binaryCols[col], 7 - row) == 1;
        Plot(numCols - 1, row, isOn); // Draw on the rightmost column, the shift loop below will scroll it leftward.
      }
      for (int refreshCount = 0; refreshCount < scrollspeed; refreshCount++)
        RefreshDisplay();
      // Shift the bitmap one column to left
      for (int row = 0; row < 8; row++)
      {
        for (int zone = 0; zone < numZones; zone++)
        {
          // This right shift would show as a left scroll on display because leftmost column is represented by least significant bit of the byte.
          bitmap[row][zone] = bitmap[row][zone] >> 1;
          // Shift over lowest bit from the next zone as highest bit of this zone.
          if (zone < maxZoneIndex)
            bitWrite(bitmap[row][zone], 7, bitRead(bitmap[row][zone + 1], 0));
        }
      }
      msg_changed = false;
      Blynk.run();
      if (msg_changed || !displayEnabled)
      {
        on();
        for (int refreshCount = 0; refreshCount < 100; refreshCount++)
          RefreshDisplay();
        clear();
        for (int refreshCount = 0; refreshCount < 30; refreshCount++)
          RefreshDisplay();
        msg_changed = false;
        break;
      }
    }
  }

  if (textmode)
  {
    for (int charIndex = 0; charIndex < msg.length(); charIndex++)
    {
      int alphabetIndex = msg[charIndex] - '@';
      if (alphabetIndex < 0)
        alphabetIndex = 0;
      int binarycol = sizeof(alphabets[alphabetIndex]);
      // Draw one character of the message
      for (int col = 0; col < binarycol; col++)
      {
        for (int row = 0; row < 8; row++)
        {
          bool isOn = 0;
          if (col < binarycol)
            isOn = bitRead(alphabets[alphabetIndex][col], 7 - row) == 1;
          Plot(numCols - 1, row, isOn); // Draw on the rightmost column, the shift loop below will scroll it leftward.
        }
        for (int refreshCount = 0; refreshCount < scrollspeed; refreshCount++)
          RefreshDisplay();
        // Shift the bitmap one column to left
        for (int row = 0; row < 8; row++)
        {
          for (int zone = 0; zone < numZones; zone++)
          {
            // This right shift would show as a left scroll on display because leftmost column is represented by least significant bit of the byte.
            bitmap[row][zone] = bitmap[row][zone] >> 1;
            // Shift over lowest bit from the next zone as highest bit of this zone.
            if (zone <= maxZoneIndex)
              bitWrite(bitmap[row][zone], 7, bitRead(bitmap[row][zone + 1], 0));
          }
        }
      }
      msg_changed = false;
      Blynk.run();
      if (msg_changed || !displayEnabled)
      {
        on();
        for (int refreshCount = 0; refreshCount < 100; refreshCount++)
          RefreshDisplay();
        clear();
        for (int refreshCount = 0; refreshCount < 30; refreshCount++)
          RefreshDisplay();
        msg_changed = false;
        break;
      }
    }
  }
}

void loop()
{
  Blynk.run();
  if (displayEnabled)
  {
    XProcess();
  }
}
