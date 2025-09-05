#include <WiFi.h>
#include "DHT.h"
#include <Adafruit_MLX90640.h>
#define WIFI_SSID "hambining"
#define WIFI_PASSWORD "92992942"
#define HOMECAM_SERIAL "B1"

#define DHTPIN 19
#define DHTTYPE DHT11

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* host = "43.200.24.221"; // 배포하면 ec2 퍼블릭 ipv4
const uint16_t port = 12345;

WiFiClient client;
Adafruit_MLX90640 mlx;
float frame[32*24]; // buffer for full frame of temperatures

DHT dht(DHTPIN, DHTTYPE);
// uncomment *one* of the below

void setup() {
// while (!Serial) delay(10);
  Serial.begin(115200);
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  delay(100);

  if (! mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("MLX90640 not found!");
    while (1) delay(10);
  }

  mlx.setMode(MLX90640_CHESS);
  mlx.setResolution(MLX90640_ADC_18BIT);
  mlx.setRefreshRate(MLX90640_2_HZ);

  dht.begin();
}

void loop() {
  if (mlx.getFrame(frame) != 0) {
    Serial.println("Failed");
    return;
  }
  if (client.connect(host, port)) {
    String sensorId = HOMECAM_SERIAL;  // 예: "24:6F:28:AA:BB:CC"
    client.print(sensorId);
    client.print("::");
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();
    if(isnan(humidity) || isnan(temperature)){
      return;
    }
    client.print(humidity,2);
    client.print(",");
    client.print(temperature,2);
    client.print("::");
    for (int i = 0; i < 32 * 24; i++) {
      if(frame[i]>=35.5 && frame[i]<=38){ // 사람 데이터만 추출
        client.print(frame[i], 2);
        if (i < 767) client.print(",");
      }
    }
    client.println(); // 개행으로 프레임 끝
    client.stop(); // 전송 끝내고 닫음
    Serial.println("Frame sent");
  } else {
    Serial.println("Connection to server failed");
  }
    delay(60000);
}