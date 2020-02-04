#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wifi auth data
String WIFI_SSID = "MY_SSID";
String WIFI_PASSWORD = "MY_SUPER_SECRET_PASSWORD";

// Server info
String SERVER = "192.168.1.154";
int SERVER_PORT = 1025;
String URL_PATH = "/updateTemperature";

// ID for server
String MODULE_NAME = "ESP01-1";

// Delay between updates in minutes
int DELAY = 3;

// GPIO where the DS18B20 is connected to
const int TEMP_SENSOR_PIN = 2;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_SENSOR_PIN);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// When no module is detected it'll return -127.00, we can use this to do stuff
const float ERROR_READING_SENSOR = -127.00;


void setup() {

  // Serial connection 
  Serial.begin(115200);
  
  // Start the DS18B20 sensor
  sensors.begin();
  
  // Connect to wifi
  connectWifi();

}

void loop() {

  // Reconnect to wifi if needed
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Wifi is ded\n Trying to reconnect");
    connectWifi();
  }else{
    //Send sensor information
    sendInfo(getSensorInformation());    
  }

  Serial.println("Seeping for " + String(DELAY) + " minutes.");
  delay(DELAY * 60000);

}

void sendInfo(float temperature){ 
  HTTPClient http;
  
  // Create URL
  String URL = "http://" + SERVER + ":" + SERVER_PORT + URL_PATH;

  Serial.println("Sending information to " + URL);

  // Specify request destination
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 

  // Create POST
  String postData = "temperature=" + String(temperature);
  postData += "&module=" + MODULE_NAME; 

  // Send the request and get its http code
  int httpCode = http.POST(postData);
  
  //Get the response payload
  String payload = http.getString();    

  // Print stuff
  Serial.println(httpCode);
  Serial.println(payload);

  //Close connection
  http.end();

}

float getSensorInformation(){

  // Get temperature from sensor
  sensors.requestTemperatures();

  // In celsius plox
  float temp = sensors.getTempCByIndex(0);

  // If no sensor detected print a message
  if(temp == ERROR_READING_SENSOR) {
    Serial.println("Failed to read from DS18B20 sensor :(");
  }
  
  return temp;
}

void connectWifi(){
  
  //Wifi connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); 

  Serial.print("\nConnecting wifi"); 
  
  // Wait for the WifI connection completion 
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");  
    delay(500);
  }

  Serial.println("\nConnected to SSID: " + WIFI_SSID + " with IP: " + WiFi.localIP().toString());

}
