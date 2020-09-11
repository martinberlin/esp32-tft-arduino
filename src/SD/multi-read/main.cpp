// ESP32 GIF/ MJPEG Reader demo

#define GIF_LOOP 2
#define MJPEG_BUFFER_SIZE (320 * 240 * 2 / 4)
#define FPS 24

#include <WiFi.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SD_MMC.h>
#include <Arduino_ESP32SPI_DMA.h>
#include <Arduino_Display.h>
#include "SPI.h"

SPIClass hspi(HSPI);

Arduino_ESP32SPI_DMA *bus = new Arduino_ESP32SPI_DMA(
    TFT_DC /* DC */, TFT_CS /* CS */, TFT_CLK /* SCK */, 
    TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST /* RST */, 1 /* rotation */);

#include "MjpegClass.h"
static MjpegClass mjpeg;
#include "gifdec.h"

void printCardInfo(uint8_t cardType, uint64_t cardSize){

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
  Serial.println("MMC");
  } else if(cardType == CARD_SD){
  Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
  Serial.println("SDHC");
  } else {
  Serial.println("UNKNOWN");
  }

  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

void setup(){
  WiFi.mode(WIFI_OFF);
  Serial.begin(115200);

  // Init Video
  gfx->begin();
  gfx->fillScreen(BLACK);

  
  // Init SPI for SD Card
  hspi.begin(SSD_CLK,SSD_MISO,SSD_MOSI,SSD_CS);
  if (!SD.begin(SSD_CS, hspi, 80000000)) /* SPI bus mode */
  {
    Serial.println(F("ERROR: SD card mount failed!"));
    gfx->println(F("ERROR: SD card mount failed!"));
    return;
  }
  
  
    uint8_t sdType = SD.cardType();
 
  if(sdType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
    }

 printCardInfo(sdType, SD.cardSize() / (1024 * 1024));

    File root = SD.open("/");
    uint8_t type=0; // 1 = GIF  2 = MJPG


    while (File vFile = root.openNextFile()) {
        if (!vFile || vFile.isDirectory())
        {
          Serial.printf("ERROR: Failed to open file %s for reading", vFile.name());
          gfx->printf("ERROR: Failed to open file %s for reading", vFile.name());
          return;
        }

        gd_GIF *gif = gd_open_gif(&vFile);
        if (!gif)
        {
            Serial.println(F("Not a GIF, trying MJPEG"));
            type = 2;
            //gfx->println(F("ERROR: gd_open_gif() failed!"));
        } else {
            type = 1;
        }

        if (type==1) {
          
          int32_t s = gif->width * gif->height;
          uint8_t *buf = (uint8_t *)malloc(s);

            if (!buf) {
              Serial.println(F("buf malloc failed!"));
              return;
            }
            else
            {
              Serial.println(F("GIF video start"));
              gfx->setAddrWindow((gfx->width() - gif->width) / 2, (gfx->height() - gif->height) / 2, gif->width, gif->height);
              int t_fstart, t_delay = 0, t_real_delay, res, delay_until;
              
              while (1)
              {
                t_fstart = millis();
                t_delay = gif->gce.delay * 10;
                res = gd_get_frame(gif, buf);
                if (res < 0)
                {
                  Serial.println(F("ERROR: gd_get_frame() failed!"));
                  break;
                }
                else if (res == 0)
                {
                  // Loop 
                  Serial.println(F("GIF ended"));
                  //gd_rewind(gif); //To loop gif
                  vFile.close();
                  continue;
                                  
                }

                gfx->startWrite();
                gfx->writeIndexedPixels(buf, gif->palette->colors, s);
                gfx->endWrite();

                t_real_delay = t_delay - (millis() - t_fstart);
                delay_until = millis() + t_real_delay;
                // do
                // {
                //   delay(1);
                // } while (millis() < delay_until);
              }
              Serial.println(F("GIF video end"));
              gd_close_gif(gif);
            }
        }

        if (type==2) {
          int next_frame = 0;
          int skipped_frames = 0;
          unsigned long total_sd_mjpeg = 0;
          unsigned long total_decode_video = 0;
          unsigned long total_remain = 0;
          unsigned long start_ms, curr_ms, next_frame_ms;
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
          }

      }


    }

    gfx->displayOff();
    //esp_deep_sleep_start();
  }

  

void loop()
{
}