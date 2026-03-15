#include <SPI.h>
#include <LoRa.h>
#include <esp_now.h>
#include <WiFi.h>

// MAC address of your ESP32-S3 Executor
uint8_t s3Mac[] = {0x1C, 0xDB, 0xD4, 0xBC, 0x74, 0xF4};

// Callback when Bridge receives EXFIL data from the ESP32-S3
// (Formatted specifically for ESP32 Core v3!)
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  // Security check: Ignore random Wi-Fi noise
  if (memcmp(info->src_addr, s3Mac, 6) != 0) return;
  
  Serial.print("[BRIDGE] Received EXFIL from S3: ");
  for(int i = 0; i < len; i++) {
    Serial.print((char)data[i]);
  }
  Serial.println();

  LoRa.beginPacket();
  LoRa.write(data, len);
  LoRa.endPacket();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[BRIDGE] ESP-NOW Init Failed");
    return;
  }
  
  // Register Receive Callback
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register the ESP32-S3 as a peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, s3Mac, 6);
  peerInfo.channel = 0; 
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[BRIDGE] Failed to add S3 Peer");
    return;
  }
  
  // Init LoRa
  LoRa.setPins(18, 14, 26);
  if(!LoRa.begin(433E6)) {
    Serial.println("[BRIDGE] LoRa Init Fail");
    while(1);
  }
  
  // THE MAGIC SHIELD: Match the Hub's network password
  LoRa.setSyncWord(0xF3); 
  
  Serial.println("[BRIDGE] ONLINE. Listening for commands...");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  // When a LoRa message arrives from the Hub...
  if (packetSize) {
    String p = "";
    while(LoRa.available()) {
      p += (char)LoRa.read();
    }
    
    Serial.print("[BRIDGE] Heard Command: ");
    Serial.println(p);
    
    // Forward the message to the S3 via ESP-NOW
    esp_now_send(s3Mac, (uint8_t*)p.c_str(), p.length());
  }
}