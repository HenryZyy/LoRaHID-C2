#include "esp_mac.h" // Required for the MAC functions

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for monitor

  uint8_t baseMac[6];
  uint8_t bluetoothMac[6];

  // Get the WiFi/Base MAC
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  
  // Get the Bluetooth MAC specifically
  esp_read_mac(bluetoothMac, ESP_MAC_BT);

  Serial.println("\n--- ESP32-S3 MAC ADDRESSES ---");
  
  Serial.printf("WiFi MAC (STA):  %02X:%02X:%02X:%02X:%02X:%02X\n", 
                baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

  Serial.printf("Bluetooth MAC:   %02X:%02X:%02X:%02X:%02X:%02X\n", 
                bluetoothMac[0], bluetoothMac[1], bluetoothMac[2], bluetoothMac[3], bluetoothMac[4], bluetoothMac[5]);
}

void loop() {
}