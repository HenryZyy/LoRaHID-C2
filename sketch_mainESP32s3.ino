#include <WiFi.h>
#include <esp_now.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

#define MAX_CMD_LEN 250
char cmdBuffer[MAX_CMD_LEN] = {0};
volatile bool newCommand = false;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  if (len <= 0 || len >= MAX_CMD_LEN) return;
  memcpy(cmdBuffer, data, len);
  cmdBuffer[len] = '\0'; 
  newCommand = true;
}

void setup() {
  Keyboard.begin();
  USB.begin(); 

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);
  
  if (esp_now_init() != ESP_OK) return;
  esp_now_register_recv_cb(OnDataRecv);

  // Boot Test
  delay(3000);
  Keyboard.println();
  Keyboard.println("--- ESP32 HID READY ---");
}

void loop() {
  if (newCommand) {
    char tempBuf[MAX_CMD_LEN];
    memcpy(tempBuf, cmdBuffer, MAX_CMD_LEN);
    newCommand = false; 
    
    // Filter the command so only valid text gets passed to the Keyboard
    String rawCmd = String(tempBuf);
    String safeCmd = "";
    for(int i = 0; i < rawCmd.length(); i++) {
      char c = rawCmd[i];
      if(c >= 32 && c <= 126) safeCmd += c;
    }

    if (safeCmd.length() > 0) {
      if (safeCmd.startsWith("TXT:")) {
        String textToType = safeCmd.substring(4);
        Keyboard.print(textToType);
      } 
      else if (safeCmd == "WIN:R") {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press('r');
        delay(150);             
        Keyboard.releaseAll();  
      } 
      else if (safeCmd == "ENTER") {
        Keyboard.write(KEY_RETURN);
      }
    }
    Keyboard.releaseAll(); // Safety release
  }
  delay(10); 
}