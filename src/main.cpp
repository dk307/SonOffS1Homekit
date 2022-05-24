#include <ESP8266WiFi.h>

#include "webServer.h"
#include "configManager.h"
#include "WiFiManager.h"
#include "operations.h"
#include "hardware.h"
#include "homeKit2.h"
#include "logging.h"

void setup(void)
{
	// Serial.begin(115200);

	operations::instance.begin();
	config::instance.begin();
	hardware::instance.begin();
	WifiManager::instance.begin(); // 3
	WebServer::instance.begin(); // 4
	homeKit2::instance.begin(); // 5

	hardware::instance.setLedDefaultState();
	LOG_INFO(F("Finish setup. Free heap: ") << ESP.getFreeHeap() / 1024 << F(" KB"));
}

void loop(void)
{
	config::instance.loop();
	WifiManager::instance.loop();
	homeKit2::instance.loop();
	hardware::instance.loop();
	operations::instance.loop(); // this can restart etc so last
}
