## Repository goal

This is just for the moment a container of examples to use ESP32 as a video player. Part of a research I'm doing to showcase digital art using small devices like epapers and TFT screens. 
The research is sponsored by [CALE.es](https://cale.es) that is a website to generate images for small CPU's and epapers/TFT displays. 

### Compiling this

This repository is ready to be compiled with Platformio (VSCode). There will be a missing include file:

In file included from .pio/libdeps/esp32/Arduino_GFX/Arduino_ESP32SPI_DMA.cpp:7:0:
.pio/libdeps/esp32/Arduino_GFX/Arduino_ESP32SPI_DMA.h:19:19: fatal error: unity.h: No such file or directory

Just comment it out since we are not using Unity testing and this is just a demo. 

### Convert any format to Motion JPEG video

This command line cookbook comes from [RGB565 video library](https://github.com/moononournation/RGB565_video):

320x240@24fps

`ffmpeg -i input.mp4 -vf "fps=24,scale=-1:240:flags=lanczos,crop=320:in_h:(in_w-320)/2:0" -q:v 9 320_24fps.mjpeg`

320x240@20fps

`ffmpeg -i input.mp4 -vf "fps=20,scale=-1:240:flags=lanczos,crop=320:in_h:(in_w-320)/2:0" -q:v 9 320_20fps.mjpeg`

320x240@15fps

`ffmpeg -i input.mp4 -vf "fps=15,scale=-1:240:flags=lanczos,crop=320:in_h:(in_w-320)/2:0" -q:v 9 320_15fps.mjpeg`

### Sponsoring

If you like this project please consider becoming a sponsor where you can donate as little as 2 u$ per month. Just click on:
**❤ Sponsor**  on the top right

**♢** For cryptocurrency users is also possible to help this project transferring Ethereum:

0x68cEAB84F33776a7Fac977B2Bdc0D50933344086

## ♢

We are thankful for the support and contributions so far!
