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
    Serial.printf("  Apparent min/max: %.1f / %.1f C\n", d.apparentTemperatureMin, d.apparentTemperatureMax);
    Serial.printf("  Sunrise/sunset: %s / %s\n", d.sunrise, d.sunset);
    Serial.printf("  Rain/snow: %.1f / %.1f mm\n", d.rainSum, d.snowfallSum);
    Serial.printf("  Max wind speed/gust: %.1f / %.1f km/h\n", d.windSpeedMax, d.windGustsMax);
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

void waitForSerial(unsigned long timeout_millis) {
    unsigned long start = millis();
    while (!Serial) {
        if (millis() - start > timeout_millis)
            break;
    }
}

void setup() {
    Serial.begin(115200);
    waitForSerial(5000);
    Prov.startWifi();

    // Wait until the provisioning flow has connected the ESP32 to Wi-Fi.
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print('.');
    }
    Serial.println("\nWi-Fi connected");

    // After Prov.startWifi() and after Wi-Fi connects:
    configTzTime("UTC0", "pool.ntp.org", "time.nist.gov");

    Serial.print("Waiting for clock");
    time_t now = time(nullptr);
    while (now < 1700000000) {  // Reject an unset/old clock
        delay(500);
        Serial.print('.');
        now = time(nullptr);
    }
    Serial.printf("\nUnix time: %ld\n", (long)now);

    weather.setCACert(rootCA);
    weather.begin(secureClient, 50.741f, 4.0067f, "Europe/Berlin");

    //    // Temporary demo TLS setting. For production, use weather.setCACert(ROOT_CA)
    //// with a persistent PEM root CA string instead.
    //    secureClient.setInsecure();
    IPAddress resolvedIp;
    if (WiFi.hostByName("api.open-meteo.com", resolvedIp) == 1) {
        Serial.printf("Open-Meteo DNS resolved to: %s\n", resolvedIp.toString().c_str());
    }
    else {
        Serial.println("Open-Meteo DNS lookup failed");
    }

    // Same array-and-struct pattern to send over serial, or other transport protocols.
    OpenMeteoDailyData dataD[2];
    OpenMeteoHourlyData dataH[5];
    OpenMeteoCurrentData dataC;

    Serial.println("Starting daily request...");
    unsigned long requestStarted = millis();

    // Request 2 daily records (today and tomorrow) for the given coordinates.
    int result = weather.getDaily(dataD, 2);

    Serial.printf("Daily request returned after %lu ms\n",
        millis() - requestStarted);

    if (result == 0) {
        Serial.printf("Daily records: 2\n");
        for (uint8_t i = 0; i < 2; ++i) {
            printDaily(dataD[i]);
        }
    }
    else {
        Serial.printf("Daily request failed (%d): %s\n",
            result, weather.lastErrorMessage());
    }

    Serial.println("Starting hourly request...");
    unsigned long hourlyStarted = millis();

    // Request 5 hourly records (the next 5 hours) for the given coordinates.
    result = weather.getHourly(dataH, 5);

    Serial.printf("Hourly request returned after %lu ms\n",
        millis() - hourlyStarted);

    if (result == 0) {
        Serial.println("Hourly records: 5");
        for (uint8_t i = 0; i < 5; ++i) {
            printHourly(dataH[i]);
        }
    }
    else {
        Serial.printf("Hourly request failed (%d): %s\n",
            result, weather.lastErrorMessage());
    }

    Serial.println("Starting current request...");
    unsigned long currentStarted = millis();

    // Request the current weather conditions for the given coordinates.
    result = weather.getCurrent(&dataC);

    Serial.printf("Current request returned after %lu ms\n",
        millis() - currentStarted);

    if (result == 0) {
        printCurrent(dataC);
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
