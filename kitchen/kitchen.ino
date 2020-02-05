#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Wifi login details
const char* WIFI_SSID = "0118 999 881 999 119 725";
const char* WIFI_PASSWORD = "... 3";

// Server info
String SERVER = "192.168.1.154";
int SERVER_PORT = 1025;
String URL_TEMPERATURE_PATH = "/updateTemperature";
String URL_SOUND_DETECTION_PATH = "/ring";

// ID for server
String MODULE_NAME = "NodemcuV3-1";

// GPIO where servo is connected to
const int SERVO_PIN = 4; //this is D2

// GPIO where temperature sensor is connected to
const int TEMP_SENSOR_PIN = 5; //this is D1

// Delay between readings in seconds
const int DELAY = 20;

// When no temperature sensor is detected it'll return -127.00, we can use this to do stuff
const float ERROR_READING_TEMPERATURE_SENSOR = -127.00;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(TEMP_SENSOR_PIN);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Create webserver on port 80
AsyncWebServer server(80);

// Create object to control servo
Servo servo;

void setup(){

  // Serial connection 
  Serial.begin(115200);

  // Start the DS18B20 sensor
  sensors.begin();

  // Attach servo
  servo.attach(SERVO_PIN);

  connectWifi();



  // I should probably make this POST and add some kind of authentication, or I could blindly trust any request. 
  // Yes, I'll do that, I'll blindly trust all requests.
  
  // Route for servo
  server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request){
    
    // Check for parameter
    if(request->hasParam("rotate")){
      
      // Get param
      AsyncWebParameter* p = request->getParam("rotate");
      
      // Believe that is an int because why would they lie
      int value = p->value().toInt();
      
      // Move servo to that position
      moveServo(value);
      
      // Everying went well!
      request->send(200);
    }else{
      // Well, we kinda needed that parameter
      request->send(400);
    } 

  });

  // Start server to control servo
  server.begin();
}

void loop(){

  // Reconnect to wifi if needed
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Wifi is ded\n Trying to reconnect");
    connectWifi();
  }else{
    //Send temperature information
    sendTemperatureInfo(getTemperatureSensorInformation());    
  }

  Serial.println("Seeping for " + String(DELAY) + " minutes.");
  delay(DELAY * 60000);
}

void moveServo(int value){
  // moveServo looks so much better than servo.write :P
  servo.write(value);
}

void sendTemperatureInfo(float temperature){ 
  HTTPClient http;
  
  // Create URL
  String URL = "http://" + SERVER + ":" + SERVER_PORT + URL_TEMPERATURE_PATH;

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

float getTemperatureSensorInformation(){

  // Get temperature from sensor
  sensors.requestTemperatures();

  // In celsius plox
  float temp = sensors.getTempCByIndex(0);

  // If no sensor detected print a message
  if(temp == ERROR_READING_TEMPERATURE_SENSOR) {
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

  Serial.print("\nConnected to SSID: ");
  Serial.print(WIFI_SSID);
  Serial.print(" with IP: ");
  Serial.println(WiFi.localIP().toString());

}
