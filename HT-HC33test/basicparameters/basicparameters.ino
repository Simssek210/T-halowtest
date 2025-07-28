#include <HaLow.h>

const char* ssid = "HTgateway";
const char* password = "heltec123";


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);

  HaLow.init("US");
  HaLow.begin(ssid, password);

  // Block here until we have an IP lease
  while (HaLow.status() != WL_CONNECTED) {
    delay(200);
  }

  Serial.println("HaLow connected!");
  Serial.print("IP: ");
  Serial.println(HaLow.localIP());
  Serial.print("Netmask:  ");
  Serial.println(HaLow.subnetMask());
  Serial.print("Gateway:  ");
  Serial.println(HaLow.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(HaLow.dnsIP());
}


void loop() {
  // put your main code here, to run repeatedly:
}
