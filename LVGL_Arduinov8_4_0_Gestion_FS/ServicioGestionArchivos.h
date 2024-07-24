#ifndef _ServicioGestionArchivos_h
#define _ServicioGestionArchivos_h

#include "FS.h"

/*
  Opciones: USE_LITTLEFS, USE_SPIFFS, USE_FFAT o USE_SD
*/
#define USE_SD 1
/*
  IMPORTANTE:
  a)Requiere de insertar la funcion ffat_drv_init();  en su código
  b) Sólo funciona con LVGL
*/

/*
  For fatfs, you have to use the old Arduino IDE to upload data.
  https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
*/
#ifdef USE_FFAT
  #include "FFat.h"
#endif

/*
  ***Recomendado***
  Requiere instalacion del plugin para arduino IDE v2
  https://randomnerdtutorials.com/arduino-ide-2-install-esp32-littlefs/
  https://github.com/earlephilhower/arduino-littlefs-upload/releases
*/
#ifdef USE_LITTLEFS
  #include <LittleFS.h>
#endif

/*
  Requiere instalacion del plugin para arduino IDE v2
  https://github.com/espx-cz/arduino-spiffs-upload?tab=readme-ov-file **No funciona**
  you have to use the old Arduino IDE to upload data.
  https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
*/
#ifdef USE_SPIFFS
  #include "SPIFFS.h"
#endif

/*
  microSD
*/
#ifdef USE_SD
  #include <SD.h>
  #include <SPI.h>

  // Declaraciones para el uso de la micro SD
  SPIClass V_SPI(VSPI); //SPI bus normally mapped to pins 12 - 15, but can be matrixed to any pins
  static const uint8_t SD_CS = 5;
#endif

 static void *ffat_fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
  fs::File *file;

//  LV_UNUSED(drv);
  
//  Serial.println("\nFILE OPEN");
//  Serial.flush();

  char buf[256];
  sprintf(buf, "/%s", path);
//  drv->cache_size = 48 *1024;

//  Serial.printf("\nbuf: %s\n", path);
//  Serial.printf("mode: %s\n", String (mode));
//  Serial.flush();

   if (mode == LV_FS_MODE_WR)
  {
    // Open a file for write

    #ifdef USE_FFAT
      file = new fs::File (FFat.open(path, FILE_WRITE));
    #endif
    #ifdef USE_LITTLEFS
      file = new fs::File (LittleFS.open(path, FILE_WRITE));
    #endif
    #ifdef USE_SPIFFS
      file = new fs::File (SPIFFS.open(path, FILE_WRITE));
    #endif
    #ifdef USE_SD
      file = new fs::File (SD.open(path, FILE_WRITE));
    #endif
  }
  else if (mode == LV_FS_MODE_RD)
  {
    // Open a file for read
    #ifdef USE_FFAT
      file = new fs::File (FFat.open(path, FILE_READ));
    #endif
    #ifdef USE_LITTLEFS
      file = new fs::File (LittleFS.open(path, FILE_READ));
    #endif
    #ifdef USE_SPIFFS
      file = new fs::File (SPIFFS.open(path, FILE_READ));
    #endif
    #ifdef USE_SD
      file = new fs::File (SD.open(path, FILE_READ));
    #endif
  }
  else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD))
  {
    // Open a file for read and write
    #ifdef USE_FFAT
      file = new fs::File (FFat.open(path, FILE_WRITE));
    #endif
    #ifdef USE_LITTLEFS
      file = new fs::File (LittleFS.open(path, FILE_WRITE));
    #endif
    #ifdef USE_SPIFFS
      file = new fs::File (SPIFFS.open(path, FILE_WRITE));
    #endif
    #ifdef USE_SD
      file = new fs::File (SD.open(path, FILE_WRITE));
    #endif
  }

  if(file == NULL){
    Serial.println("FILE NOT FOUND");
    delete file;
    return NULL;
  }

  // make sure at the beginning
  file->seek(0);

  return file;
}

static lv_fs_res_t ffat_fs_close(lv_fs_drv_t *drv, void *file_p)
{
  LV_UNUSED(drv);
  
//  Serial.println("\nFILE CLOSE");
  fs::File *file = (fs::File *)file_p;

  file->close();
  free(file);

  return LV_FS_RES_OK;
}

static lv_fs_res_t ffat_fs_read(lv_fs_drv_t *drv, void *file_p, void *fileBuf, uint32_t btr, uint32_t *br)
{
//  Serial.println("\nFILE READ");
  LV_UNUSED(drv);
  lv_fs_res_t res = LV_FS_RES_OK;

  fs::File *file = (fs::File *)file_p;

  if (!file || file->isDirectory()) {
    Serial.println("- failed to open file for reading");
    return LV_FS_RES_FS_ERR;
  }

//  Serial.print("Read init: ");
//  Serial.print(file->position());

  if (file->available())
    *br = file->read((uint8_t*)fileBuf, btr);  //btr

//  Serial.print("Read end: ");
//  Serial.print(file->position());
//  Serial.print("Read: ");
//  Serial.print(String(*br));
//  Serial.println(" bytes.");
//  Serial.flush();
  return res;

}

static lv_fs_res_t ffat_fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{

//  Serial.println("FILE SEEK");
  String requestFile[3] = {"LV_FS_SEEK_SET","LV_FS_SEEK_CUR","LV_FS_SEEK_END"};

  uint32_t new_pos;
  fs::File *file = (fs::File *)file_p;

  if(pos > file->size()){
    Serial.println("Error en la solicitud de posicion del archivo");
    return LV_FS_RES_INV_PARAM;
  }

  switch (whence)
  {
    case LV_FS_SEEK_SET:
      new_pos = pos;
      break;
    case LV_FS_SEEK_CUR:
      new_pos = file->position() + pos;
      break;
    case LV_FS_SEEK_END:
      new_pos = file->size() - pos;
      break;
    default:
      return LV_FS_RES_INV_PARAM;
  }

//  Serial.print("actual position: ");
//  Serial.print(file->position());


  if (!file->seek(new_pos))
  {
    Serial.println("Error en !file->seek(new_pos)");
    return LV_FS_RES_UNKNOWN;
  }

//  Serial.print(" -> request pos: ");
//  Serial.print(pos);
//  Serial.print(" -> new_pos: ");
//  Serial.print(new_pos);
//  Serial.print(" -> file size: ");
//  Serial.print(file->size());
//  Serial.print(" -> whence: ");
//  Serial.println(String(requestFile[whence]));
//  Serial.flush();

  return LV_FS_RES_OK;
}

bool ffat_fs_ready(lv_fs_drv_t *drv)
{
  #ifdef USE_FFAT
    bool flag = FFat.begin();
  #endif
  #ifdef USE_LITTLEFS
    bool flag = LittleFS.begin();
  #endif
  #ifdef USE_SPIFFS
    bool flag = SPIFFS.begin();
  #endif
  #ifdef USE_SD
    bool flag = SD.begin(SD_CS, V_SPI);
  #endif
  
//  Serial.print("FFat ready?... ");
//  Serial.println(flag? "True":"False");
  return flag;
}

void ffat_init()
{
//  Serial.println("Initializing FS ...");
#ifdef USE_FFAT
  if (!FFat.begin())
#endif
#ifdef USE_LITTLEFS
  if (!LittleFS.begin())
#endif
#ifdef USE_SPIFFS
  if (!SPIFFS.begin())
#endif
#ifdef USE_SD
    if(!SD.begin(SD_CS, V_SPI))
#endif
  {
      Serial.println("Storage Mount Failed");
      return;
  }

  Serial.println("Storage Mount success!!");
}

static lv_fs_res_t ffat_fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
//  Serial.println("FILE TELL");

  fs::File *file = (fs::File *)file_p;
  *pos_p = file->position();

  return LV_FS_RES_OK;
}

void ffat_drv_init()
{
  
  ffat_init();
  // create the drive  
  static lv_fs_drv_t ffat_drv;
//  lv_fs_drv_t ffat_drv;
//  memset(&ffat_drv, 0, sizeof(lv_fs_drv_t));

  lv_fs_drv_init(&ffat_drv);
  ffat_drv.letter = 'A';
  ffat_drv.cache_size = 64 * 1024; // w * h * 4;
  ffat_drv.open_cb = ffat_fs_open;
  ffat_drv.close_cb = ffat_fs_close;
  ffat_drv.read_cb = ffat_fs_read;
  ffat_drv.seek_cb = ffat_fs_seek;
  ffat_drv.tell_cb =ffat_fs_tell;
//  ffat_drv.write_cb = NULL;
  ffat_drv.ready_cb = ffat_fs_ready;
//  ffat_drv.dir_close_cb = NULL;
//  ffat_drv.dir_open_cb = NULL;
//  ffat_drv.dir_read_cb = NULL;
  lv_fs_drv_register(&ffat_drv);
  Serial.println("Registred Driver");
  Serial.flush();

  return;
}

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  fs::File root = fs.open(dirname);
  if (!root) {
    Serial.println("- failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" - not a directory");
    return;
  }

  fs::File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

#endif
