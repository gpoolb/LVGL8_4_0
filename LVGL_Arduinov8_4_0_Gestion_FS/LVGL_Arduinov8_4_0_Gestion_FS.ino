
#include <mutex> // https://forum.arduino.cc/t/how-to-use-mutex/964924/15
#include <lvgl.h>
#include <TFT_eSPI.h>
/*If you want to use the LVGL examples,
  make sure to install the lv_examples Arduino library
  and uncomment the following line.
#include <lv_examples.h>
*/
//#include <demos/lv_demos.h>
//#include <examples/lv_examples.h>
#include <TFT_Touch.h>
//#include <lv_demo.h>

#include "earth.c"
#include "ServicioGestionArchivos.h"

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 320;//480;
static const uint16_t screenHeight = 240;//320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

// These are the pins used to interface between the 2046 touch controller and Arduino Pro
#define DOUT 39  /* Data out pin (T_DO) of touch screen */
#define DIN  32  /* Data in pin (T_DIN) of touch screen */
#define DCS  33  /* Chip select pin (T_CS) of touch screen */
#define DCLK 25  /* Clock pin (T_CLK) of touch screen */

/* Create an instance of the touch screen library */
TFT_Touch touch = TFT_Touch(DCS, DCLK, DIN, DOUT);

TaskHandle_t displayImageTaskHandler;
static std::mutex lvgl_mutex;

lv_obj_t *label = NULL;
lv_obj_t *imgSec = NULL;

/*
  IMPORTANTE: Para archivos crear los archivos:
  
  a) .bin use la herramienta en https://lvgl.io/tools/imageconverter
    LVGL v8 ... y use las opciones 
    "Color format" : CF_TRUE_COLOR_ALPHA
    "Output Format" : binary RGB565

  b) .bmp Se requiere del uso de un software gimp v2.10.12: https://www.gimp.org/
    cargue su imagen y exporte la imagen con las siguientes opciones:
    archivo -> exportar como... -> "Nombre" : myFile.bmp
    presione el boton exportar -> "Opciones avanzadas" -> 16 bits -> marque la opcion R5G5B5
    presione el boton exportar.

  c) para jpg y png no se encontraron complicaciones.

  d) para una imagen en una variable use la herramienta en 
    https://lvgl.io/tools/imageconverter LVGL v8 ... y use las opciones 
    "Color format" : CF_TRUE_COLOR_ALPHA
    "Output Format" : c array

  Para que LVGL pueda mostrar los archivos verifique lo siguiente en lv_conf.f

  - LV_MEM_CUSTOM 1
  - LV_USE_PNG 1 (para mostrar archivos png)
  - LV_USE_BMP 1 (para mostrar archivos bmp)
  - LV_USE_SJPG 1 (para mostrar archivos jpg)
  - ffat_drv.cache_size = 48 * 1024; // Asigne suficiente memoria para
    abrir los archivos usando la formula w * h * 4; (width y height en pixeles.)
*/
const char nameOfFile[5][20] = {"A:/rain.bin", "A:/cloudy.bmp", "A:/blue_flower.jpg", "A:/red_flower.png", "image from var"};
const long interval = 5000;           // interval in milliseconds


void displayImageTask(void *pvParameters) {
  static char index = 0;

  for(;;){
    
    Serial.println(String(&nameOfFile[index][0]));
    Serial.print("Memoria libre: ");
    Serial.println(String(ESP.getFreeHeap()));
    Serial.flush();
    {
      std::lock_guard<std::mutex> lck(lvgl_mutex);
      if (index < 4) {
        lv_img_set_src(imgSec, &nameOfFile[index][0]);
        lv_label_set_text( label, &nameOfFile[index][0] );
      } else {
        LV_IMG_DECLARE(earth);
        lv_img_set_src(imgSec, &earth);
        lv_label_set_text(label, "Imagen desde variable: &earth");
      }
    }

    index++;
    if (index > 4)
      index = 0;

    vTaskDelay(5000);
  }
}

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/

void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX, touchY;

    bool touched = touch.Pressed();//tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {   
        touchX = touch.X();
        touchY = touch.Y();
        
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void setup()
{
  Serial.begin( 115200 ); /* prepare for possible serial debug */
  Serial.setDebugOutput(true);

// if (!FFat.begin()) {
// if (!LittleFS.begin()) {
// if(!SD.begin(SD_CS, V_SPI)){
//     Serial.println("Storage Mount Failed");
// //    return;
// } else {
//     Serial.printf("Total space: %10u\n", FFat.totalBytes());
//     Serial.printf("Free space: %10u\n", FFat.freeBytes());
//     listDir(FFat, "/", 0);
// }

  
  String LVGL_Arduino = "Hello Arduino! ";
  LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

  Serial.println( LVGL_Arduino );
  Serial.println( "I am LVGL_Arduino" );



  lv_init();
  ffat_drv_init();

#if LV_USE_LOG != 0
  lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

  tft.begin();          /* TFT init */
  tft.setRotation(1);//( 3 ); /* Landscape orientation, flipped */

  /*Set the touchscreen calibration data,
    the actual data for your display can be acquired using
    the Generic -> Touch_calibrate example from the TFT_eSPI library*/
  //uint16_t calData[] = { 456, 3608, 469, 272, 3625, 3582, 3518, 263,  };
//  tft.setTouchCalibrate(calData);//   
//  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
//  tft.setTouch( calData );
  touch.setCal(526, 3443, 750, 3377, 320, 240, 1);

  lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * 10 );

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init( &disp_drv );
  /*Change the following line to your display resolution*/
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush; //刷新
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register( &disp_drv );

  /*Initialize the (dummy) input device driver*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init( &indev_drv );
  indev_drv.type = LV_INDEV_TYPE_POINTER;
  indev_drv.read_cb = my_touchpad_read;
  lv_indev_drv_register( &indev_drv );

#if 0
  /* Create simple label */
  lv_obj_t *label = lv_label_create( lv_scr_act() );
  lv_label_set_text( label, LVGL_Arduino.c_str() );
  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
#else
  /* Try an example from the lv_examples Arduino library
      make sure to include it as written above.
  lv_example_btn_1();
  */

//  lv_example_tabview_2();

  // uncomment one of these demos
//  lv_demo_widgets();            // OK
//  lv_demo_benchmark();          // OK
//  lv_demo_keypad_encoder();     // works, but I haven't an encoder
//  lv_demo_music();              // NOK
//  lv_demo_printer();
//  lv_demo_stress();             // seems to be OK

  imgSec = lv_img_create(lv_scr_act());
  lv_obj_center(imgSec);

  label = lv_label_create( lv_scr_act() );
  lv_obj_align( label, LV_ALIGN_BOTTOM_MID, 0, 0 );

#endif

  xTaskCreate(displayImageTask,
              "DisplayImageTask",
              4096,
              NULL,
              1,
              &displayImageTaskHandler);

  Serial.println( "Setup done" );
}

void loop()
{
  {
    std::lock_guard<std::mutex> lck(lvgl_mutex);

    lv_timer_handler(); /* let the GUI do its work */
  }
  delay( 5 );
}
//http://lvgl.100ask.org/8.2/index.html
//https://www.cnblogs.com/frozencandles/archive/2022/06/14/16375392.html
