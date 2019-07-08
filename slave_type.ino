
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <Servo.h>


int DC_SPEED = 0;
int SERVO_POS = 90;
int dc_pin = 12;
int servopin = 13;
Servo servomotor;
uint8_t mac[] = {0x36, 0x33, 0x33, 0x33, 0x33, 0x34};

struct __attribute__((packed)) DataStruct {
    char text[32];
    unsigned int time;
};

DataStruct myData;

void initVariant() {
  WiFi.mode(WIFI_AP);
  esp_wifi_set_mac(ESP_IF_WIFI_AP, &mac[0]);
}


void receiveCallBackFunction(const uint8_t *senderMac, const uint8_t *incomingData, int len) {
  
  char *ptr;
  char *i;
  memcpy(&myData, incomingData, sizeof(myData));
  
  ptr = strtok_r(myData.text, ":", &i);

  if(strcmp("dc",ptr) == 0) { // dc motor speed
      char *speed_nb = strtok_r(NULL, ":", &i);
      DC_SPEED = atoi(speed_nb);
      
  } else if(strcmp("sv",ptr) == 0) { // servo position
      char *pos_nb = strtok_r(NULL,":",&i);
      SERVO_POS = atoi(pos_nb);
      
  }
  
}

void setup() {
    pinMode(dc_pin, OUTPUT); // Set pin for output to control TIP120 Base pin
    servomotor.attach(servopin);
 
    while(esp_now_init()!=ESP_OK) {
        delay(500);
    }

    esp_now_register_recv_cb(receiveCallBackFunction);

    DC_SPEED = 0;
    SERVO_POS = 0;

}


void loop() {
  digitalWrite(dc_pin, DC_SPEED);
  servomotor.write(SERVO_POS);
}

