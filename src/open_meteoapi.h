/*
 Name:		open_meteoapi.h
 Created:	10/22/2026 9:29:21 PM
 Author:	Daen
 Editor:	http://www.visualmicro.com
*/
//
#pragma once
#ifndef _open_meteoapi_h
#define _open_meteoapi_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

#include <JsonListener.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>


class OpenMeteoJsonListener;

static const uint8_t OPEN_METEO_MAX_DAILY = 16;
static const uint8_t OPEN_METEO_MAX_HOURLY = 48;

struct OpenMeteoDailyData {
	char date[11] = {};
	float temperatureMin = NAN;
	float temperatureMax = NAN;
	char sunrise[20] = {};
	char sunset[20] = {};
	float rainSum = NAN;
	float snowfallSum = NAN;
	float windGustsMax = NAN;
	float windSpeedMax = NAN;
	float moonPhase = NAN;
	float windDirectionDominant = NAN;
	float apparentTemperatureMax = NAN;
	float apparentTemperatureMin = NAN;
	char moonrise[20] = {};
	int weatherCode = -1;
};

struct OpenMeteoHourlyData {
	char time[20] = {};
	float temperature = NAN;
	int weatherCode = -1;
	float rain = NAN;
	float snowfall = NAN;
	float relativeHumidity = NAN;
};

struct OpenMeteoCurrentData {
	char time[20] = {};
	float temperature = NAN;
	int weatherCode = -1;
	float rain = NAN;
	float snowfall = NAN;
	float relativeHumidity = NAN;
};

class open_meteoapi {
public:
    open_meteoapi();

    uint32_t interval = 0;

    void begin(
        WiFiClientSecure& client,
        float latitude,
        float longitude,
        const char* timezone = "auto");

    void setCACert(const char* rootCA);
    void setTemperatureUnit(const char* unit);
    void setWindSpeedUnit(const char* unit);
    void setPrecipitationUnit(const char* unit);
    void setForecastDays(uint8_t days);

    int getDaily(OpenMeteoDailyData* out, uint8_t count);
    int getHourly(OpenMeteoHourlyData* out, uint8_t count);
    int getCurrent(OpenMeteoCurrentData* out);

    void setTimeout(uint16_t milliseconds);
    int lastError() const { return _lastError; }
    const char* lastErrorMessage() const;

private:
    WiFiClientSecure* _client;
    const char* _caCert;
    float _latitude;
    float _longitude;
    char _timezone[48];
    char _temperatureUnit[16];
    char _windSpeedUnit[8];
    char _precipitationUnit[16];
    uint8_t _forecastDays;
    uint16_t _timeout;
    uint32_t _lastRequest;
    char _lastRequestSection[12];
    int _lastError;
    char _transportError[96];

    int request(
        const char* section,
        const char* variables,
        uint8_t count,
        OpenMeteoJsonListener& listener);

    String makeUrl(
        const char* section,
        const char* variables,
        uint8_t count);

    static String encode(const char* value);
};

#endif

