#include <SimpleTimer.h>
#include <HTTP_Method.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define OTA_ALLOW_PIN 34
#define CONTACTRON_PIN 33

const char* ssid = "smarzony";
const char* password = "metalisallwhatineed";

WebServer server(80);
SimpleTimer HeartbeatTimer;
byte counter = 0;

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
	// ArduinoOTA.setHostname("myesp32");

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

	
	if (digitalRead(OTA_ALLOW_PIN) == 1)
	{

		if (digitalRead(CONTACTRON_PIN) == 1)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, LOW);
			Serial.println("Waiting on LOW interrupt");
		}

		if (digitalRead(CONTACTRON_PIN) == 0)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
			Serial.println("Waiting on HIGH interrupt");
		}

		Serial.println("Going to sleep");
		delay(500);
		//Go to sleep now
		esp_deep_sleep_start();
	}
	else if (digitalRead(OTA_ALLOW_PIN) == 0)
	{
		Serial.println("---- Entering OTA mode ----");
		server.on("/", handleRootPath);		 // ustawienie przerwania
		server.begin();
		server.handleClient();
	}
	HeartbeatTimer.setInterval(1000, heartbeat);
	Serial.println("Starting main loop");
}

void loop() {
	ArduinoOTA.handle();
	server.handleClient();
	HeartbeatTimer.run();

	if (digitalRead(OTA_ALLOW_PIN) == 1)
	{

		if (digitalRead(CONTACTRON_PIN) == 1)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, LOW);
			Serial.println("Waiting on LOW interrupt");
		}

		if (digitalRead(CONTACTRON_PIN) == 0)
		{
			esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, HIGH);
			Serial.println("Waiting on HIGH interrupt");
		}

		Serial.println("Going to sleep");
		delay(500);
		//Go to sleep now
		esp_deep_sleep_start();
	}
}

void heartbeat()
{
	Serial.println(counter);
	counter++;
}