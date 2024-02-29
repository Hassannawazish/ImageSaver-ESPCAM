#define CAMERA_MODEL_AI_THINKER // Your Board Info

#include "esp_camera.h" // Contains Declarations of Function Calls releated to ESP-IDF Library for camera acess
#include "camera_pins.h" // Defines the GPIO Pin Numbers of ESP32 for which OV2640 Camera Module is interfaced
void startCameraServer(); // External Function present in Application  
#include "FS.h"                // Contains Declarations of Functions required for File System Creation, Access and Operations
#include "SD_MMC.h"            // Contains Declarations of Functions required for SDCard / MMC Acess, Read, Write etc
#include "EEPROM.h"            // Contains Declarations of Functions required for EEPROM Acess, Read, Write etc
// define the number of bytes you want to access
#define EEPROM_SIZE 1

int pictureNumber = 0;


void setup() { //First Function That would Run when any Arduino Based Board Starts
 
  Serial.begin(115200);//Set Baud Rate For Serial Communication
  Serial.setDebugOutput(true);// Let Debug Messages Appear
  Serial.println();//Just Prints a NewLine

  static camera_config_t config; 
  // Config Object is created to fill settings and Pin numbers. These Macros are part of camera_pins.h
  // In NutShell its the Object Created to Configure the Sensor
  config.ledc_channel       = LEDC_CHANNEL_0;
  config.ledc_timer         = LEDC_TIMER_0;
  config.pin_d0             = Y2_GPIO_NUM;
  config.pin_d1             = Y3_GPIO_NUM;
  config.pin_d2             = Y4_GPIO_NUM;
  config.pin_d3             = Y5_GPIO_NUM;
  config.pin_d4             = Y6_GPIO_NUM;
  config.pin_d5             = Y7_GPIO_NUM;
  config.pin_d6             = Y8_GPIO_NUM;
  config.pin_d7             = Y9_GPIO_NUM;
  config.pin_xclk           = XCLK_GPIO_NUM;
  config.pin_pclk           = PCLK_GPIO_NUM;
  config.pin_vsync          = VSYNC_GPIO_NUM;
  config.pin_href           = HREF_GPIO_NUM;
  config.pin_sscb_sda       = SIOD_GPIO_NUM;
  config.pin_sscb_scl       = SIOC_GPIO_NUM;
  config.pin_pwdn           = PWDN_GPIO_NUM;
  config.pin_reset          = RESET_GPIO_NUM;
  config.xclk_freq_hz       = 20000000;//SPI Clock Frequency set to 20MHz
  config.pixel_format       = PIXFORMAT_JPEG;//Compression format from Sensor is selected as JPEG
  config.frame_size         = FRAMESIZE_SVGA;// Resolution SVGA 
  config.jpeg_quality       = 10;//lowest is better, lowest possible is 10
  config.fb_count           = 1;//Number of Frame Buffers at a time

  // camera init with required settings and pin numbers 
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {//This Flag is Set Internally by Init Function in case of failure
    Serial.printf("Camera init failed with error 0x%x", err);//error number would tell us type of error
    return;
  }
  #if 1
  //Acquire Access to the sensor
  sensor_t * s = esp_camera_sensor_get();// Get Acess to tweak camera settings 
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 1200);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 30);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)6);  // 0 to 6
  s->set_bpc(s, 1);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s,0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
  #endif

  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  // initialize EEPROM with predefined size, start at specified address.
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, pictureNumber);//Write Current Running File Number to First Byte of EEPROM

  
}

void loop() {
  // put your main code here, to run repeatedly:
  camera_fb_t * fb = NULL;
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  pictureNumber  = EEPROM.read(0) + 1;
  // Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";
  Serial.printf("Picture file name: %s\n", path.c_str());
  
  //Open the File in Write mode, Open will also Create the file if not present                                    
  File file = SD_MMC.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  } 
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.printf("Saved file to path: %s\n", path.c_str());//Path of File Saved
    EEPROM.write(0, pictureNumber);//Write Current Running File Number to First Byte of EEPROM
    EEPROM.commit();// Approve the Write Operation.
  }
  file.close();
  esp_camera_fb_return(fb);   

  delay(5000
  );


}
