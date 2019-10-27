#include <HTTPClient.h>
#include <SimpleTimer.h>
#include <HTTP_Method.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define OTA_ALLOW_PIN 34
#define CONTACTRON_PIN 33
#define BATTERY_PIN 35

 

const char* ssid = "smarzony";
const char* password = "metalisallwhatineed";
const char* host = "192.168.1.226";
const int port = 80;
String idx_reed_switch = "30";
String idx_reed_switch_battery = "32";

WebServer server(80);
SimpleTimer HeartbeatTimer, DomoticzUpdateTimer, DoorRefreshTimer;
byte counter = 0;
bool contactron_state, contactron_state_last, ota_state;
int adc_value, voltage_value, battery_percentage;
uint32_t free_heap;

void setup() {
	Serial.begin(115200);
	Serial.println("Booting");

	pinMode(OTA_ALLOW_PIN, INPUT_PULLUP);
	pinMode(CONTACTRON_PIN, INPUT_PULLUP);

	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("Connection Failed! Rebooting...");
		delay(5000);
		ESP.restart();
	}

	// Port defaults to 3232
	// ArduinoOTA.setPort(3232);

	// Hostname defaults to esp3232-[MAC]
	ArduinoOTA.setHostname("Door_Checker_esp32");

	// No authentication by default
	// ArduinoOTA.setPassword("admin");

	// Password can be set with it's md5 value as well
	// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
	// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

	ArduinoOTA
		.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH)
			type = "sketch";
		else // U_SPIFFS
			type = "filesystem";

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		Serial.println("Start updating " + type);
			})
		.onEnd([]() {
				Serial.println("\nEnd");
			})
				.onProgress([](unsigned int progress, unsigned int total) {
				Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
					})
				.onError([](ota_error_t error) {
						Serial.printf("Error[%u]: ", error);
						if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
						else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
						else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
						else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
						else if (error == OTA_END_ERROR) Serial.println("End Failed");
					});

					ArduinoOTA.begin();

					Serial.println("Ready");
					Serial.print("IP address: ");
					Serial.println(WiFi.localIP());

					ota_state = digitalRead(OTA_ALLOW_PIN);
					contactron_state = digitalRead(CONTACTRON_PIN);

					if (ota_state == 1)
					{

						if (contactron_state == 1)
						{
							esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, LOW);
							Serial.println("Waiting on LOW interrupt");
						}

						if (contactron_state == 0)
						{
							esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
							Serial.println("Waiting on HIGH interrupt");
						}

						Serial.println("Going to sleep");
						delay(500);
						//Go to sleep now
						esp_deep_sleep_start();
					}
					else if (ota_state == 0)
					{
						Serial.println("---- Entering OTA mode ----");
						server.on("/", handleRootPath);		 // ustawienie przerwania
						server.begin();
						server.handleClient();
					}
					HeartbeatTimer.setInterval(1000, heartbeat);
					DomoticzUpdateTimer.setInterval(1000, readStateSendState);
					DoorRefreshTimer.setInterval(2000, refreshDoor);
					Serial.println("Starting main loop");
}


void loop() {
	
	ArduinoOTA.handle();
	server.handleClient();
	HeartbeatTimer.run();
	DoorRefreshTimer.run();

	ota_state = digitalRead(OTA_ALLOW_PIN);
	

	/*if (ota_state == 1)
	{

		if (contactron_state == 1)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, LOW);
			Serial.println("Waiting on LOW interrupt");
		}

		if (contactron_state == 0)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
			Serial.println("Waiting on HIGH interrupt");
		}

		Serial.println("Going to sleep");
		delay(500);
		//Go to sleep now
		esp_deep_sleep_start();
	}
	else
	{
		DomoticzUpdateTimer.run();
	}
	*/

}

void refreshDoor()
{
	contactron_state = digitalRead(CONTACTRON_PIN);
	if (contactron_state != contactron_state_last)
	{
		readStateSendState();
		delay(500);
	}
	free_heap = ESP.getFreeHeap();
	contactron_state_last = contactron_state;
}

void heartbeat()
{
	Serial.println(counter);
	counter++;
}

void readStateSendState()
{
	String chD7_state;
	//contactron_state = digitalRead(CONTACTRON_PIN);
	/*
	adc_value = analogRead(BATTERY_PIN);
	voltage_value = map(adc_value, 637, 665, 3780, 3960);
	battery_percentage = map(voltage_value, 2500, 4200, 0, 100);
	*/

	if (contactron_state)
		chD7_state = "Off";
	else
		chD7_state = "On";

	// 665 - 3.96
	// 637 - 3.78


	Serial.print("Sensor state: ");
	Serial.println(contactron_state);
	String url = "/json.htm?type=command&param=switchlight&idx=";
	url += idx_reed_switch;
	url += "&switchcmd=";
	url += chD7_state;

	sendToDomoticz(url);

	delay(10); 
	/*
	Serial.print("Sensor battery: ");
	Serial.println(battery_percentage);
	url = "/json.htm?type=command&param=udevice&idx=";
	url += idx_reed_switch_battery;
	url += "&nvalue=0&svalue=";
	url += (String)battery_percentage;

	sendToDomoticz(url);
	*/
}

void sendToDomoticz(String url)
{

	HTTPClient http;

	Serial.print("\nconnecting to ");
	Serial.println(host);
	Serial.print("Requesting URL: ");
	Serial.println(url);
	http.begin(host, port, url);
	int httpCode = http.GET();
	if (httpCode) {
		//if (httpCode == 200) {
		String payload = http.getString();
		Serial.println("Domoticz response ");
		Serial.println(payload);
		//}
	}
	Serial.println("closing connection");
	http.end();
	Serial.print("Free mem: ");
	Serial.print(ESP.getFreeHeap());
	Serial.println(" B");
}