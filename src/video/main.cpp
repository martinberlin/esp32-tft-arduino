// ESP32 Video demo
// This demo will just read the first file of SPIFFS, that has a max. size with this partition setup of:
// 2031616 (About 2 Mb)   Make sure to upload a mjpeg and not any other video format

//Adjust for different width size
//#define MJPEG_BUFFER_SIZE (220 * 176 * 2 / 4)
// ILI9341 has 320*240:
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 4)
#define FPS 24
#include <WiFi.h>
#include <SPIFFS.h>


#include <Arduino_ESP32SPI.h>
#include <Arduino_Display.h>
#define TFT_BRIGHTNESS 128

// ILI9225 Display
#define TFT_BL 22
Arduino_ESP32SPI *bus = new Arduino_ESP32SPI(
    TFT_DC /* DC */, TFT_CS /* CS */, TFT_CLK /* SCK */, 
    TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST /* RST */, 1 /* rotation */);

#include "MjpegClass.h"
static MjpegClass mjpeg;

int next_frame = 0;
int skipped_frames = 0;
unsigned long total_sd_mjpeg = 0;
unsigned long total_decode_video = 0;
unsigned long total_remain = 0;
unsigned long start_ms, curr_ms, next_frame_ms;


void setup()
{
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  // Init Video
  gfx->begin();
  gfx->fillScreen(BLACK);

#ifdef TFT_BL
  ledcAttachPin(TFT_BL, 1);     // assign TFT_BL pin to channel 1
  ledcSetup(1, 12000, 8);       // 12 kHz PWM, 8-bit resolution
  ledcWrite(1, TFT_BRIGHTNESS); // brightness 0 - 255
#endif

  // Init SPIFFS
  if (!SPIFFS.begin())
  {
    Serial.println(F("ERROR: SD card mount failed!"));
    gfx->println(F("ERROR: SD card mount failed!"));
  }
  else
  {

    File root = SPIFFS.open("/");
    File vFile = root.openNextFile();
    
    if (!vFile || vFile.isDirectory())
    {
      Serial.printf("ERROR: Failed to open file %s for reading", vFile.name());
      gfx->printf("ERROR: Failed to open file %s for reading", vFile.name());
      return;
    }
    else
    {
      uint8_t *mjpeg_buf = (uint8_t *)malloc(MJPEG_BUFFER_SIZE);
      if (!mjpeg_buf)
      {
        Serial.println(F("mjpeg_buf malloc failed!"));
      }
      else
      {
        Serial.println(F("MP3 audio MJPEG video start"));
        start_ms = millis();
        curr_ms = millis();
        next_frame_ms = start_ms + (++next_frame * 1000 / FPS / 2);

        mjpeg.setup(vFile, mjpeg_buf, gfx, false);
        

        unsigned long start = millis();
        // Read video
        while (mjpeg.readMjpegBuf())
        {
          total_sd_mjpeg += millis() - curr_ms;
          curr_ms = millis();

          if (millis() < next_frame_ms) // check show frame or skip frame
          {
            // Play video
            mjpeg.drawJpg();
            total_decode_video += millis() - curr_ms;

            int remain_ms = next_frame_ms - millis();
            if (remain_ms > 0)
            {
              total_remain += remain_ms;
              delay(remain_ms);
            }
          }
          else
          {
            ++skipped_frames;
            Serial.println(F("Skip frame"));
          }

          curr_ms = millis();
          next_frame_ms = start_ms + (++next_frame * 1000 / FPS);
        }
        Serial.println(F("MP3 audio MJPEG video end"));
        vFile.close();

        int time_used = millis() - start_ms;
        Serial.println(F("End audio video"));
        int played_frames = next_frame - 1 - skipped_frames;
        float fps = 1000.0 * played_frames / time_used;
        Serial.printf("Played frames: %d\n", played_frames);
        Serial.printf("Skipped frames: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / played_frames);
        Serial.printf("Time used: %d ms\n", time_used);
        Serial.printf("Expected FPS: %d\n", FPS);
        Serial.printf("Actual FPS: %0.1f\n", fps);
        Serial.printf("Read MJPEG: %d ms (%0.1f %%)\n", total_sd_mjpeg, 100.0 * total_sd_mjpeg / time_used);
        Serial.printf("Play video: %d ms (%0.1f %%)\n", total_decode_video, 100.0 * total_decode_video / time_used);
        Serial.printf("Remain: %d ms (%0.1f %%)\n", total_remain, 100.0 * total_remain / time_used);

#define CHART_MARGIN 24
#define LEGEND_A_COLOR 0xE0C3
#define LEGEND_B_COLOR 0x33F7
#define LEGEND_C_COLOR 0x4D69
#define LEGEND_D_COLOR 0x9A74
#define LEGEND_E_COLOR 0xFBE0
#define LEGEND_F_COLOR 0xFFE6
#define LEGEND_G_COLOR 0xA2A5
        gfx->setCursor(0, 0);
        gfx->setTextColor(WHITE);
        gfx->printf("Played Frames: %d\n", played_frames);
        gfx->printf("Skipped: %d (%0.1f %%)\n", skipped_frames, 100.0 * skipped_frames / played_frames);
        gfx->printf("Actual FPS: %0.1f\n\n", fps);
        int16_t r1 = ((gfx->height() - CHART_MARGIN - CHART_MARGIN) / 2);
        int16_t r2 = r1 / 2;
        int16_t cx = gfx->width() - gfx->height() + CHART_MARGIN + CHART_MARGIN - 1 + r1;
        int16_t cy = r1 + CHART_MARGIN;

        /* gfx->fillArc(cx, cy, r2 - 1, 0, 0.0, 360.0, LEGEND_C_COLOR);
        gfx->setTextColor(LEGEND_C_COLOR);
        gfx->printf("Play MP3:\n%0.1f %%\n", 100.0); */

        float arc_start = 0;
        float arc_end = 360.0 * total_sd_mjpeg / time_used;
        for (int i = arc_start + 1; i < arc_end; i += 2)
        {
          gfx->fillArc(cx, cy, r1, r2, arc_start - 90.0, i - 90.0, LEGEND_A_COLOR);
        }
        gfx->fillArc(cx, cy, r1, r2, arc_start - 90.0, arc_end - 90.0, LEGEND_A_COLOR);
        gfx->setTextColor(LEGEND_A_COLOR);
        gfx->printf("Read MJPEG:\n%0.1f %%\n", 100.0 * total_sd_mjpeg / time_used);

        arc_start = arc_end;
        arc_end += 360.0 * total_decode_video / time_used;
        for (int i = arc_start + 1; i < arc_end; i += 2)
        {
          gfx->fillArc(cx, cy, r1, r2, arc_start - 90.0, i - 90.0, LEGEND_B_COLOR);
        }
        gfx->fillArc(cx, cy, r1, r2, arc_start - 90.0, arc_end - 90.0, LEGEND_B_COLOR);
        gfx->setTextColor(LEGEND_B_COLOR);
        gfx->printf("Play MJPEG:\n%0.1f %%\n", 100.0 * total_decode_video / time_used);
      }
    }
  }
#ifdef TFT_BL
  delay(60000);
  ledcDetachPin(TFT_BL);
#endif
  gfx->displayOff();
  esp_deep_sleep_start();
}

void loop()
{
}
