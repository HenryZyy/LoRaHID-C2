#include <WiFi.h>
#include <esp_now.h>
#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

#define MAX_CMD_LEN 250
char cmdBuffer[MAX_CMD_LEN] = {0};
volatile bool newCommand = false;

// --- SWISS GERMAN TRANSLATION LAYER ---
void typeSwiss(String text) {
  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    switch (c) {
      case 'z': Keyboard.write('y'); break;
      case 'Z': Keyboard.write('Y'); break;
      case 'y': Keyboard.write('z'); break;
      case 'Y': Keyboard.write('Z'); break;
      case ':': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('.'); Keyboard.releaseAll(); break;
      case '/': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('7'); Keyboard.releaseAll(); break;
      case '(': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('8'); Keyboard.releaseAll(); break;
      case ')': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('9'); Keyboard.releaseAll(); break;
      case '?': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('\''); Keyboard.releaseAll(); break;
      case '=': Keyboard.press(KEY_LEFT_SHIFT); Keyboard.write('0'); Keyboard.releaseAll(); break;
      case '-': Keyboard.write('/'); break; 
      default:  Keyboard.write(c); break;
    }
  }
}

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

  delay(3000);
  typeSwiss("--- ESP32 HID READY (SG) ---");
  Keyboard.println();
}

void loop() {
  if (newCommand) {
    char tempBuf[MAX_CMD_LEN];
    memcpy(tempBuf, cmdBuffer, MAX_CMD_LEN);
    newCommand = false; 
    
    String safeCmd = String(tempBuf);
    safeCmd.trim();

    if (safeCmd.length() > 0) {
      
      // 1. DELAY COMMAND (Waits in milliseconds)
      if (safeCmd.startsWith("DELAY:")) {
        int delayTime = safeCmd.substring(6).toInt();
        if (delayTime > 0) {
          delay(delayTime);
        }
      }
      
      // 2. TEXT TYPING (Swiss Optimized)
      else if (safeCmd.startsWith("TXT:")) {
        typeSwiss(safeCmd.substring(4));
      } 
      
      // 3. MODIFIER KEYS
      else if (safeCmd.startsWith("CTRL:")) {
        Keyboard.press(KEY_LEFT_CTRL);
        Keyboard.press(safeCmd.substring(5).charAt(0));
        delay(100); Keyboard.releaseAll();
      }
      else if (safeCmd.startsWith("ALT:")) {
        Keyboard.press(KEY_LEFT_ALT);
        Keyboard.press(safeCmd.substring(4).charAt(0));
        delay(100); Keyboard.releaseAll();
      }
      else if (safeCmd.startsWith("SHIFT:")) {
        Keyboard.press(KEY_LEFT_SHIFT);
        Keyboard.press(safeCmd.substring(6).charAt(0));
        delay(100); Keyboard.releaseAll();
      }
      else if (safeCmd.startsWith("WIN:") || safeCmd.startsWith("GUI:")) {
        Keyboard.press(KEY_LEFT_GUI);
        Keyboard.press(safeCmd.substring(4).charAt(0));
        delay(100); Keyboard.releaseAll();
      }

      // 4. SINGLE BUTTONS
      else if (safeCmd == "ENTER")     Keyboard.write(KEY_RETURN);
      else if (safeCmd == "TAB")       Keyboard.write(KEY_TAB);
      else if (safeCmd == "ESC")       Keyboard.write(KEY_ESC);
      else if (safeCmd == "BS")        Keyboard.write(KEY_BACKSPACE);
      else if (safeCmd == "DEL")       Keyboard.write(KEY_DELETE);
      else if (safeCmd == "CAPS")      Keyboard.write(KEY_CAPS_LOCK);
      else if (safeCmd == "SPACE")     Keyboard.write(' ');
      
      // 5. ARROW KEYS
      else if (safeCmd == "UP")        Keyboard.write(KEY_UP_ARROW);
      else if (safeCmd == "DOWN")      Keyboard.write(KEY_DOWN_ARROW);
      else if (safeCmd == "LEFT")      Keyboard.write(KEY_LEFT_ARROW);
      else if (safeCmd == "RIGHT")     Keyboard.write(KEY_RIGHT_ARROW);
    }
    
    Keyboard.releaseAll(); 
  }
  delay(10); 
}