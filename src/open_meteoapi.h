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
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

//https://open-meteo.com/en/docs?daily=temperature_2m_min,temperature_2m_max,sunrise,sunset,rain_sum,snowfall_sum,wind_gusts_10m_max,wind_speed_10m_max,moon_phase,wind_direction_10m_dominant,moonrise,weather_code&latitude=50.741&longitude=4.0067&timezone=Europe%2FBerlin&forecast_days=1&hourly=temperature_2m,weather_code,rain,snowfall,relative_humidity_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m#data_sources
//https://api.open-meteo.com/v1/forecast?latitude=50.741&longitude=4.0067&daily=temperature_2m_min,temperature_2m_max,sunrise,sunset,rain_sum,snowfall_sum,wind_gusts_10m_max,wind_speed_10m_max,moon_phase,wind_direction_10m_dominant,moonrise,weather_code&hourly=temperature_2m,weather_code,rain,snowfall,relative_humidity_2m,wind_speed_10m,wind_direction_10m,wind_gusts_10m&timezone=Europe%2FBerlin&forecast_days=1
//{ "latitude":50.746, 
//"longitude" : 4.002, 
//"generationtime_ms" : 0.22935867309570312, 
//"utc_offset_seconds" : 7200, 
//"timezone" : "Europe/Berlin", 
//"timezone_abbreviation" : "GMT+2", 
//"elevation" : 31.0, "hourly_units" : {
//	"time":"iso8601", 
//		"temperature_2m" : "°C", 
//		"weather_code" : "wmo code", 
//		"rain" : "mm", 
//		"snowfall" : "cm", 
//		"relative_humidity_2m" : "%", 
//		"wind_speed_10m" : "km/h", 
//		"wind_direction_10m" : "°", 
//		"wind_gusts_10m" : "km/h"}, 
//	"hourly" : {
//	"time": ["2026-10-23T00:00", "2026-10-23T01:00", "2026-10-23T02:00", "2026-10-23T03:00", "2026-10-23T04:00", "2026-10-23T05:00", "2026-10-23T06:00", "2026-10-23T07:00", "2026-10-23T08:00", "2026-10-23T09:00", "2026-10-23T10:00", "2026-10-23T11:00", "2026-10-23T12:00", "2026-10-23T13:00", "2026-10-23T14:00", "2026-10-23T15:00", "2026-10-23T16:00", "2026-10-23T17:00", "2026-10-23T18:00", "2026-10-23T19:00", "2026-10-23T20:00", "2026-10-23T21:00", "2026-10-23T22:00", "2026-10-23T23:00"] , 
//		"temperature_2m" : [11.8, 11.9, 12.2, 12.3, 12.0, 11.3, 11.2, 10.7, 10.5, 10.6, 11.0, 11.5, 11.0, 10.8, 11.1, 11.1, 12.5, 12.2, 11.4, 10.2, 8.6, 7.3, 6.8, 7.0] , 
//		"weather_code" : [3, 51, 53, 53, 53, 53, 51, 55, 3, 3, 3, 3, 53, 61, 51, 61, 63, 53, 61, 53, 51, 53, 61, 63] , 
//		"rain" : [0.00, 0.30, 0.70, 0.90, 0.80, 0.90, 0.40, 1.00, 0.00, 0.00, 0.00, 0.00, 0.50, 2.00, 0.40, 1.80, 3.10, 0.90, 1.60, 0.80, 0.20, 0.90, 1.70, 2.60] , 
//		"snowfall" : [0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00, 0.00] , 
//		"relative_humidity_2m" : [95, 96, 95, 94, 92, 90, 90, 90, 91, 90, 86, 84, 89, 91, 87, 90, 91, 92, 92, 91, 89, 90, 91, 92] , 
//		"wind_speed_10m" : [17.6, 17.6, 20.5, 24.5, 25.9, 24.1, 27.0, 31.0, 27.4, 27.4, 33.1, 35.6, 33.1, 37.1, 46.4, 52.6, 41.4, 38.5, 37.4, 42.1, 41.0, 36.0, 36.0, 34.6] , 
//		"wind_direction_10m" : [169, 168, 169, 165, 170, 168, 165, 161, 166, 164, 165, 167, 170, 165, 181, 194, 238, 245, 236, 242, 250, 253, 251, 238] , 
//		"wind_gusts_10m" : [31.3, 31.3, 35.6, 42.1, 46.1, 43.9, 49.0, 54.7, 58.0, 48.2, 59.0, 63.7, 63.7, 66.2, 83.2, 94.7, 94.0, 73.8, 74.5, 74.2, 74.9, 73.8, 64.4, 68.0] }, 
//	"daily_units" : {
//	"time":"iso8601", 
//		"temperature_2m_min" : "°C", 
//		"temperature_2m_max" : "°C", 
//		"sunrise" : "iso8601", 
//		"sunset" : "iso8601", 
//		"rain_sum" : "mm", 
//		"snowfall_sum" : "cm", 
//		"wind_gusts_10m_max" : "km/h", 
//		"wind_speed_10m_max" : "km/h", 
//		"moon_phase" : "fraction", 
//		"wind_direction_10m_dominant" : "°", 
//		"moonrise" : "iso8601", 
//		"weather_code" : "wmo code"}, 
//	"daily" : {
//	"time": ["2026-10-23"] , 
//		"temperature_2m_min" : [6.8] , 
//		"temperature_2m_max" : [12.5] , 
//		"sunrise" : ["2026-10-23T08:20"] , 
//		"sunset" : ["2026-10-23T18:35"] , 
//		"rain_sum" : [23.10] , 
//		"snowfall_sum" : [0.00] , 
//		"wind_gusts_10m_max" : [94.7] , 
//		"wind_speed_10m_max" : [52.6] , 
//		"moon_phase" : [0.401] , 
//		"wind_direction_10m_dominant" : [198] , 
//		"moonrise" : ["2026-10-23T17:17"] , 
//		"weather_code" : [63] } 
//}

#include <JsonListener.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "Cert_open-meteo.h"


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


