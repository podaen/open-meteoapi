/*
 Name:		basic.ino
 Created:	10/20/2026 7:39:39 AM
 Author:	Davy P.
*/

#include <time.h>
#include <JsonStreamingParser.h>
#include <JsonListener.h>
#include <open_meteoapi.h>
#include <NetworkClientSecure.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include "nvs_flash.h"
#include <wifi_connectapp.h>
#include <WiFiProv.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include "Cert_open-meteo.h"

wifi_connectapp Prov;
WiFiClientSecure secureClient;
open_meteoapi weather;

void printDaily(const OpenMeteoDailyData& d) {
    Serial.printf("\nDaily %s\n", d.date);
    Serial.printf("  Temperature min/max: %.1f / %.1f C\n", d.temperatureMin, d.temperatureMax);
    Serial.printf("  Apparent min/max: %.1f / %.1f C\n",
        d.apparentTemperatureMin, d.apparentTemperatureMax);
    Serial.printf("  Sunrise/sunset: %s / %s\n", d.sunrise, d.sunset);
    Serial.printf("  Rain/snow: %.1f / %.1f mm\n", d.rainSum, d.snowfallSum);
    Serial.printf("  Max wind speed/gust: %.1f / %.1f km/h\n",
        d.windSpeedMax, d.windGustsMax);
    Serial.printf("  Dominant wind direction: %.0f degrees\n", d.windDirectionDominant);
    Serial.printf("  Moon phase/rise: %.2f / %s\n", d.moonPhase, d.moonrise);
    Serial.printf("  Weather code: %d\n", d.weatherCode);
}

void printHourly(const OpenMeteoHourlyData& h) {
    Serial.printf("\nHourly %s\n", h.time);
    Serial.printf("  Temperature: %.1f C\n", h.temperature);
    Serial.printf("  Rain/snow: %.1f / %.1f mm\n", h.rain, h.snowfall);
    Serial.printf("  Relative humidity: %.0f%%\n", h.relativeHumidity);
    Serial.printf("  Weather code: %d\n", h.weatherCode);
}

void printCurrent(const OpenMeteoCurrentData& c) {
    Serial.printf("\nCurrent conditions at %s\n", c.time);
    Serial.printf("  Temperature: %.1f C\n", c.temperature);
    Serial.printf("  Rain/snow: %.1f / %.1f mm\n", c.rain, c.snowfall);
    Serial.printf("  Relative humidity: %.0f%%\n", c.relativeHumidity);
    Serial.printf("  Weather code: %d\n", c.weatherCode);
}

void waitForSerial(unsigned long timeoutMillis) {
    const unsigned long started = millis();
    while (!Serial && millis() - started < timeoutMillis) {
        delay(10);
    }
}

bool waitForClock(unsigned long timeoutMillis) {
    const unsigned long started = millis();
    time_t now = time(nullptr);

    Serial.print("Waiting for clock");
    while (now < 1700000000 && millis() - started < timeoutMillis) {
        delay(500);
        Serial.print('.');
        now = time(nullptr);
    }
    Serial.println();

    if (now < 1700000000) {
        Serial.println("Clock synchronization timed out");
        return false;
    }

    Serial.printf("Unix time: %ld\n", (long)now);
    return true;
}

void setup() {
    Serial.begin(115200);
    waitForSerial(5000);

    Prov.startWifi();
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print('.');
    }
    Serial.println("\nWi-Fi connected");

    // TLS certificate verification requires the ESP32 clock to be correct.
    configTzTime("UTC0", "pool.ntp.org", "time.nist.gov");
    if (!waitForClock(30000)) {
        return;
    }

    weather.begin(secureClient, 50.741f, 4.0067f, "Europe/Berlin");
    weather.setCACert(rootCA);
    weather.setTimeout(30000);

    // One struct for each request; no caller-side data arrays are needed.
    OpenMeteoDailyData daily;
    OpenMeteoHourlyData hourly;
    OpenMeteoCurrentData current;

    Serial.println("Starting daily request...");
    unsigned long started = millis();
    int result = weather.getDaily(&daily, 1);
    Serial.printf("Daily request returned after %lu ms\n", millis() - started);
    if (result == 0) {
        printDaily(daily);
    }
    else {
        Serial.printf("Daily request failed (%d): %s\n",
            result, weather.lastErrorMessage());
    }

    Serial.println("Starting hourly request...");
    started = millis();
    result = weather.getHourly(&hourly, 1);
    Serial.printf("Hourly request returned after %lu ms\n", millis() - started);
    if (result == 0) {
        printHourly(hourly);
    }
    else {
        Serial.printf("Hourly request failed (%d): %s\n",
            result, weather.lastErrorMessage());
    }

    Serial.println("Starting current request...");
    started = millis();
    result = weather.getCurrent(&current);
    Serial.printf("Current request returned after %lu ms\n", millis() - started);
    if (result == 0) {
        printCurrent(current);
    }
    else {
        Serial.printf("Current request failed (%d): %s\n",
            result, weather.lastErrorMessage());
    }
    Serial.println("All requests completed");
}

void loop() {
    delay(1000);
}
