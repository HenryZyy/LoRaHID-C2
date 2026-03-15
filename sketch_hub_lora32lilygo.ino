#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  LoRa.setPins(18, 14, 26);
  if (!LoRa.begin(433E6)) { Serial.println("LoRa Fail"); while(1); }
  
  // THE MAGIC SHIELD: Only listen to your specific LoRa network
  LoRa.setSyncWord(0x12); 
  
  Serial.println("HUB ONLINE. Commands: TXT:msg, WIN:R, ENTER");
}

void loop() {
  if (Serial.available()) {
    String msg = Serial.readStringUntil('\n');
    msg.trim(); // clean up the command
    if(msg.length() > 0) {
      Serial.println("Sent: " + msg);
      LoRa.beginPacket();
      LoRa.print(msg);
      LoRa.endPacket();
    }
  }
  
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("EXFIL: ");
    while(LoRa.available()) Serial.print((char)LoRa.read());
    Serial.println();
  }
}