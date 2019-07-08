

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>


#define NB_SLAVES 5
uint8_t remoteMac[][6] = {
   {0x36, 0x33, 0x33, 0x33, 0x33, 0x34},
 {0x36, 0x33, 0x33, 0x33, 0x33, 0x35},
 {0x36, 0x33, 0x33, 0x33, 0x33, 0x36},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x37},
  {0x36, 0x33, 0x33, 0x33, 0x33, 0x38}
};
esp_now_peer_info_t slave[NB_SLAVES];



struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};

DataStruct myData;
String serial_input; 


void sendCallBackFunction(const uint8_t* destination_mac_addr, esp_now_send_status_t uploadStatus) {
      Serial.println(uploadStatus == ESP_NOW_SEND_SUCCESS ? "Success" : "Fail");
}

void sendData() {
        myData.time = millis();
        uint8_t bs[sizeof(myData)];
        memcpy(bs, &myData, sizeof(myData));
        esp_now_send(NULL, bs, sizeof(myData)); // NULL means send to all peers
}



void setup() {
    Serial.begin(115200);
    Serial.setTimeout(10);
    
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    while(esp_now_init()!=ESP_OK) {
        Serial.println("*** ESP_Now init failed");
        delay(500);
    }

    for(int y = 0; y < NB_SLAVES; y++) {
        for (int ii = 0; ii < 6; ++ii ) {
                slave[y].peer_addr[ii] = (uint8_t) remoteMac[y][ii];
              }
        esp_now_add_peer(&slave[y]);
    }
    esp_now_register_send_cb(sendCallBackFunction);
}


void loop() {

  while(Serial.available()) {
    serial_input = Serial.readString();
    strcpy(myData.text, (char *)serial_input.c_str());
    sendData();
  }

}
