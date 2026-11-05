/*
 Name:		basic.ino
 Created:	10/20/2026 7:39:39 AM
 Author:	Davy P.
*/

#include <esp_sntp.h>
#include <time.h>

#include <JsonListener.h>
#include <open_meteoapi.h>
#include <NetworkClientSecure.h>
#include <ssl_client.h>
#include <WiFiClientSecure.h>
#include "Cert_open-meteo.h"

#include "nvs_flash.h"
#include <wifi_connectapp.h>
#include <WiFiProv.h>
#include <WiFi.h>

wifi_connectapp Prov;
WiFiClientSecure secureClient;
open_meteoapi weather;

OpenMeteoDailyData dailyWeather{};
constexpr uint8_t HOURLY_WEATHER_COUNT = 5;
OpenMeteoHourlyData hourlyWeather[HOURLY_WEATHER_COUNT]{};

bool daily = true;
bool hourly = true;

/*weather timers*/
constexpr uint32_t WEATHER_REQUEST_INTERVAL = 15UL * 60UL * 1000UL;//15 minutes interval for weather data caching

const char* StrDayName = "";
uint8_t Icon;

struct tm now = {};
static const char daysOfTheWeek[7][12] = {
    "Zo", "Ma", "Di", "Wo", "Do", "Vr", "Za"
};


void startTime() {
    // Central European Time with automatic daylight-saving time.
    // CET: UTC+1
    // CEST: UTC+2
    //CET-1: standard time is UTC+1.
    //CEST: daylight-saving time name.
    //M3.5.0/2: daylight-saving time starts on the last Sunday of March at 02:00 local time.
    //M10.5.0/3: daylight-saving time ends on the last Sunday of October at 03:00 local time.
    configTzTime(
        "CET-1CEST,M3.5.0/2,M10.5.0/3",
        "europe.pool.ntp.org",
        "pool.ntp.org");
    sntp_set_sync_interval(24UL * 60UL * 60UL * 1000UL);//once a day synchronization clock
    Serial.println("Wi-Fi and time synchronization started");
}
const char* getDayName() {

    if (now.tm_wday < 0 || now.tm_wday >= 7) {
        return "";
    }

    return daysOfTheWeek[now.tm_wday];
}
/*print time*/
void printDayname(const struct tm& timeInfo) {
    if (timeInfo.tm_wday < 0 || timeInfo.tm_wday >= 7) {
        Serial.println("Day: unknown");
        return;
    }

    Serial.printf("Day: %s\n", daysOfTheWeek[timeInfo.tm_wday]);
}
void printTime(const struct tm& timeInfo) {
    Serial.printf(
        "Time: %04d-%02d-%02d %02d:%02d:%02d\n",
        timeInfo.tm_year + 1900,
        timeInfo.tm_mon + 1,
        timeInfo.tm_mday,
        timeInfo.tm_hour,
        timeInfo.tm_min,
        timeInfo.tm_sec);
}

void StartWeather() {
    weather.begin(secureClient, 50.741f, 4.0067f, "Europe/Berlin");

    weather.setCACert(rootCA);
    weather.setTemperatureUnit("celsius");
    weather.setWindSpeedUnit("kmh");
    weather.setPrecipitationUnit("mm");
    weather.setTimeout(30000);

    weather.interval = WEATHER_REQUEST_INTERVAL;

    Serial.println("Weather service initialized");
}

bool GetDailyWeather() {
    Serial.println("Starting daily request...");
    Serial.printf("Wi-Fi status: %d, IP: %s\n",
        WiFi.status(),
        WiFi.localIP().toString().c_str());

    const unsigned long started = millis();
    const int result = weather.getDaily(&dailyWeather, 1);

    Serial.printf(
        "Daily request returned after %lu ms\n",
        millis() - started);

    if (result != 0) {
        Serial.printf(
            "Daily request failed (%d): %s\n",
            result,
            weather.lastErrorMessage());

        return false;
    }

    if (dailyWeather.date[0] == '\0') {
        Serial.println("Daily request returned no data");
        return false;
    }

    Icon = static_cast<uint8_t>(dailyWeather.weatherCode);

    printDaily();
    return true;
}
bool GetHourlyWeather() {
    constexpr uint8_t HOURLY_ITEMS = 5;

    Serial.println("Starting hourly request...");
    Serial.printf(
        "Wi-Fi status: %d, IP: %s\n",
        WiFi.status(),
        WiFi.localIP().toString().c_str());

    const uint32_t started = millis();
    const int result = weather.getHourly(hourlyWeather, HOURLY_ITEMS);

    Serial.printf(
        "Hourly request returned after %lu ms\n",
        millis() - started);

    if (result != 0) {
        Serial.printf(
            "Hourly request failed (%d): %s\n",
            result,
            weather.lastErrorMessage());

        return false;
    }

    if (hourlyWeather[0].time[0] == '\0') {
        Serial.println("Hourly request returned no data");
        return false;
    }

    printHourly();
    return true;
}

void printDaily() {
    //"daily" : {"time": ["2026-11-03"] , "temperature_2m_min" : [1.9] , "temperature_2m_max" : [9.5] , "sunrise" : ["2026-11-03T07:38"] , "sunset" : ["2026-11-03T17:15"] , "rain_sum" : [0.10] , "snowfall_sum" : [0.00] , "wind_gusts_10m_max" : [31.3] , "wind_speed_10m_max" : [17.6] , "moon_phase" : [0.807] , "wind_direction_10m_dominant" : [47] , "moonrise" : ["2026-11-03T00:37"] , "weather_code" : [51] } }
    Serial.printf("\nDaily %s\n", dailyWeather.date);
    Serial.printf("  Temperature min/max: %.1f / %.1f C\n",
        dailyWeather.temperatureMin,
        dailyWeather.temperatureMax);
    Serial.printf("  Apparent min/max: %.1f / %.1f C\n",
        dailyWeather.apparentTemperatureMin,
        dailyWeather.apparentTemperatureMax);
    Serial.printf("  Sunrise/sunset: %s / %s\n",
        dailyWeather.sunrise,
        dailyWeather.sunset);
    Serial.printf("  Rain/snow: %.1f / %.1f mm\n",
        dailyWeather.rainSum,
        dailyWeather.snowfallSum);
    Serial.printf("  Max wind speed/gust: %.1f / %.1f km/h\n",
        dailyWeather.windSpeedMax,
        dailyWeather.windGustsMax);
    Serial.printf("  Dominant wind direction: %.0f degrees\n",
        dailyWeather.windDirectionDominant);
    Serial.printf("  Moon phase/rise: %.2f / %s\n",
        dailyWeather.moonPhase,
        dailyWeather.moonrise);
    Serial.printf("  Weather code: %d\n",
        dailyWeather.weatherCode);
}
void printHourly() {
    for (uint8_t index = 0; index < HOURLY_WEATHER_COUNT; ++index) {
        Serial.printf("\nHourly %s\n", hourlyWeather[index].time);
        Serial.printf(
            "  Temperature: %.1f C\n",
            hourlyWeather[index].temperature);
        Serial.printf(
            "  Rain/snow: %.1f / %.1f mm\n",
            hourlyWeather[index].rain,
            hourlyWeather[index].snowfall);
        Serial.printf(
            "  Relative humidity: %.0f%%\n",
            hourlyWeather[index].relativeHumidity);
        Serial.printf(
            "  Weather code: %d\n",
            hourlyWeather[index].weatherCode);
    }
}

void waitForSerial(unsigned long timeoutMillis) {
    const unsigned long started = millis();
    while (!Serial && millis() - started < timeoutMillis) {
        delay(10);
    }
}
bool waitForClock(unsigned long timeoutMillis) {
    const unsigned long started = millis();
    time_t epoch = time(nullptr);

    Serial.print("Waiting for clock");

    while (epoch < 1700000000 && millis() - started < timeoutMillis) {
        delay(500);
        Serial.print('.');
        epoch = time(nullptr);
    }

    Serial.println();

    if (epoch < 1700000000) {
        Serial.println("Clock synchronization timed out");
        return false;
    }

    // Convert Unix time to local time using the timezone configured in startTime().
    localtime_r(&epoch, &now);

    Serial.printf("Unix time: %ld\n", static_cast<long>(epoch));
    printTime(now);
    printDayname(now);

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

    startTime();

    if (!waitForClock(15000)) {
        Serial.println("Continuing without valid time");
        return;
    }

    StartWeather();

    if (daily) {
        if (GetDailyWeather()) { printDaily(); }
    }

    if (hourly) {
        if (GetHourlyWeather()); { printHourly(); }
    }
    Serial.println("All requests completed");
}
void loop() {
    delay(1000);
}