// ESP32 GIF Video demo
#define GIF_FILENAME "/3.gif"
#define GIF_LOOP true

#include <WiFi.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SD_MMC.h>

#include <Arduino_ESP32SPI_DMA.h>
#include <Arduino_Display.h>
#include "SPI.h"
SPIClass hspi(HSPI);

#define TFT_BRIGHTNESS 128

// ILI9225 Display
#define TFT_BL 22

Arduino_ESP32SPI_DMA *bus = new Arduino_ESP32SPI_DMA(
    TFT_DC /* DC */, TFT_CS /* CS */, TFT_CLK /* SCK */, 
    TFT_MOSI /* MOSI */, TFT_MISO /* MISO */);
Arduino_ILI9341 *gfx = new Arduino_ILI9341(bus, TFT_RST /* RST */, 1 /* rotation */);

#include "gifdec.h"


void setup()
{
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
  }
  else
  {
    uint8_t cardType = SD.cardType();
 
if(cardType == CARD_NONE){
Serial.println("No SD card attached");
return;
}
 
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
 
uint64_t cardSize = SD.cardSize() / (1024 * 1024);
Serial.printf("SD Card Size: %lluMB\n", cardSize);
 

    File vFile = SD.open(GIF_FILENAME);
    
    if (!vFile || vFile.isDirectory())
    {
      Serial.println(F("ERROR: Failed to open "GIF_FILENAME" file for reading"));
      gfx->println(F("ERROR: Failed to open "GIF_FILENAME" file for reading"));
      return; // Stop here
    }

    gd_GIF *gif = gd_open_gif(&vFile);
      if (!gif)
      {
              Serial.println(F("ERROR: gd_open_gif() failed!"));
              gfx->println(F("ERROR: gd_open_gif() failed!"));
      }
      else
      {
        int32_t s = gif->width * gif->height;
        uint8_t *buf = (uint8_t *)malloc(s);
        if (!buf)
        {
          Serial.println(F("buf malloc failed!"));
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
              if (GIF_LOOP) {
                  Serial.println(F("gd_rewind()."));
                  gd_rewind(gif);
                  continue;
              }  else {
                break; // exit while loop instead of loop back
              }
              
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
  }
  

  gfx->displayOff();
  esp_deep_sleep_start();
}

void loop()
{
}
