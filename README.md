# LVGL8_4_0
examples LVGL libraries

Puesta en marcha de Placa de desarrollo con LCD 2.8" 240x320, Arduino LVGL https://a.aliexpress.com/_mLngQsM
Soporte del fabricante:  http://pan.jczn1688.com/directlink/1/ESP32%20module/2.8inch_ESP32-2432S028R.rar

1. Usando el Arduino IDE v2.3.2, con el BOARDS MANAGER descargue el compilador esp32 by Espressif Systems v3.0.2

2. Usando el Arduino IDE v2.3.2, con el LIBRARY MANAGER descargue TFT_eSPI by Bodmer v2.5.43

3. Siguiendo lo indicado en https://docs.lvgl.io/8.4/get-started/quick-overview.html, y usando el Arduino IDE v2.3.2, con el LIBRARY MANAGER descargue lvgl by kisvegabor v8.4.0, posteriormente, copie las carpetas examples y demos en la carpeta src.

4. Usando el navegador de su preferencia, descargue TFT_touch v0.3 en el sitio https://github.com/Bodmer/TFT_Touch y copie la carpeta recién descargada en Arduino\libraries\

5. Usando el Arduino IDE v2.3.2, el ejemplo original se encuentra en lvgl -> Arduino -> LVGL Arduino, pero no trabajaremos sobre el ejemplo original ya que, se requiere agregar las funciones del touch screen.

6. Copie el archivo User_Setup.h que se proporciona en esta documentación en la carpeta Arduino\libraries\TFT_eSPI

7. Copie el archivo lv_config.h en Arduino\libraries\

8. Abra el ejemplo que se proporciona en ésta documentación, compile y descargue en su plataforma de trabajo. NO OLVIDE seleccionar la Board: "ESP32 Dev Module", Flash Mode: "DIO" y el puerto COM donde se encuentra su plataforma.

Más información en: https://lvgl.io
