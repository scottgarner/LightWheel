#include <FastLED.h>

#define DATA_PIN 7

#define LED_COUNT 241

#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define DATA_SIZE (LED_COUNT * 3)

#define HEADER_BYTE 0xAA
#define FOOTER_BYTE 0x55

#define FRAME_TIMEOUT 500

CRGB leds[LED_COUNT];

uint8_t serialBuffer[DATA_SIZE];
int bufferIndex = 0;

uint8_t headerCount = 0;
bool inFrame = false;

unsigned long lastDataTime = 0;

void setup() {

  // Configure pixels.
  {
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, LED_COUNT, 0);
    FastLED.setBrightness(32);
  }

  // Initial pixel state.
  {
    for (int i = 0; i < LED_COUNT; i++) {
      if (i % 2 == 0)
        leds[i].setRGB(255, 0, 0);
      else
        leds[i].setRGB(0, 255, 0);
    }
    FastLED.show();
  }

  // Serial setup.
  {
    for (int i = 0; i < LED_COUNT; i++) {
      if (i % 2 == 0)
        leds[i].setRGB(255, 0, 0);
      else
        leds[i].setRGB(0, 255, 0);
    }
    FastLED.show();
  }
}

void loop() {
  // Reset if timed out.
  if (inFrame && (millis() - lastDataTime > FRAME_TIMEOUT)) {
    inFrame = false;
    headerCount = 0;
    bufferIndex = 0;
  }

  // Read serial data.
  while (Serial.available() > 0) {
    uint8_t nextByte = Serial.read();
    lastDataTime = millis();

    // Watch for new frame.
    if (!inFrame) {

      if (nextByte == HEADER_BYTE) {
        headerCount++;

        // Start frame after two header bytes.
        if (headerCount == 2) {
          inFrame = true;
          bufferIndex = 0;
          headerCount = 0;
        }

      } else {
        // Reset on non-header bytes.
        headerCount = 0;
      }

      // Always continue if not in frame.
      continue;
    }

    // Accumulate data while in frame.
    if (bufferIndex < DATA_SIZE) {
      serialBuffer[bufferIndex++] = nextByte;

      // Continue while buffering.
      continue;
    }

    // Show LEDs if if the footer is valid.
    if (nextByte == FOOTER_BYTE) {
      memcpy(leds, serialBuffer, DATA_SIZE);
      FastLED.show();
    }

    // Complete frame.
    inFrame = false;
    headerCount = 0;
    bufferIndex = 0;
  }
}