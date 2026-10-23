/*
 Name:		open_meteoapi.cpp
 Created:	10/22/2026 9:29:21 PM
 Author:	Daen
 Editor:	http://www.visualmicro.com
*/

#include "open_meteoapi.h"
#include <JsonListener.h>
#include <JsonStreamingParser.h>

namespace {
    const char* DAILY_FIELDS =
        "temperature_2m_min,temperature_2m_max,sunrise,sunset,rain_sum,"
        "snowfall_sum,wind_gusts_10m_max,wind_speed_10m_max,moon_phase,"
        "wind_direction_10m_dominant,apparent_temperature_max,"
        "apparent_temperature_min,moonrise,weather_code";
    const char* HOURLY_FIELDS =
        "temperature_2m,weather_code,rain,snowfall,relative_humidity_2m";
    const char* CURRENT_FIELDS =
        "temperature_2m,weather_code,rain,snowfall,relative_humidity_2m";

    enum ErrorCode {
        ERR_NOT_INITIALIZED = -1001,
        ERR_BAD_ARGUMENT = -1002,
        ERR_TRANSPORT = -1003,
        ERR_JSON = -1004,
        ERR_MISSING_DATA = -1005
    };

    enum DataKind { KIND_DAILY, KIND_HOURLY, KIND_CURRENT };
    enum Section { SECTION_NONE, SECTION_DAILY, SECTION_HOURLY, SECTION_CURRENT };

    float parseFloatValue(const String& value) {
        return value == "null" || value.length() == 0 ? NAN : value.toFloat();
    }
    int parseIntValue(const String& value) {
        return value == "null" || value.length() == 0 ? -1 : value.toInt();
    }
    void copyString(char* destination, size_t capacity, const String& source) {
        if (!capacity) return;
        source.toCharArray(destination, capacity);
        destination[capacity - 1] = '\0';
    }
}

// Converts streaming parser callbacks directly into the caller's result structs.
class OpenMeteoJsonListener : public JsonListener {
public:
    OpenMeteoJsonListener(DataKind kind, void* output, uint8_t count)
        : _kind(kind), _output(output), _count(count) {
    }

    void whitespace(char) override {}
    void startDocument() override { _depth = 0; }
    void key(String key) override {
        _key = key;
        if (_depth == 1) {
            if (_kind == KIND_DAILY && key == "daily") _pending = SECTION_DAILY;
            else if (_kind == KIND_HOURLY && key == "hourly") _pending = SECTION_HOURLY;
            else if (_kind == KIND_CURRENT && key == "current") _pending = SECTION_CURRENT;
        }
    }
    void value(String value) override {
        if (_section == SECTION_NONE) return;
        if (_kind == KIND_CURRENT) {
            if (_depth == 2) assignCurrent(_key, value);
            return;
        }
        if (_depth == 2 && _inArray) {
            if (_arrayIndex < _count) assignArray(_key, _arrayIndex, value);
            ++_arrayIndex;
        }
    }
    void endArray() override {
        if (_inArray && _key == "time") _timeCount = _arrayIndex;
        _inArray = false;
    }
    void endObject() override {
        if (_depth == 2 && _section != SECTION_NONE) _section = SECTION_NONE;
        if (_depth > 0) --_depth;
    }
    void endDocument() override { _complete = true; }
    void startArray() override {
        if (_section != SECTION_NONE && _depth == 2) {
            _inArray = true;
            _arrayIndex = 0;
        }
    }
    void startObject() override {
        ++_depth;
        if (_depth == 2 && _pending != SECTION_NONE) {
            _section = _pending;
            _pending = SECTION_NONE;
            _sectionSeen = true;
        }
    }

    bool complete() const { return _complete; }
    bool hasExpectedData() const {
        if (!_complete || !_sectionSeen) return false;
        if (_kind == KIND_CURRENT) {
            const OpenMeteoCurrentData* data = static_cast<OpenMeteoCurrentData*>(_output);
            return data && data->time[0] != '\0';
        }
        return _timeCount >= _count;
    }

private:
    DataKind _kind;
    void* _output;
    uint8_t _count;
    int _depth = 0;
    Section _pending = SECTION_NONE;
    Section _section = SECTION_NONE;
    String _key;
    bool _inArray = false;
    bool _complete = false;
    bool _sectionSeen = false;
    uint8_t _arrayIndex = 0;
    uint8_t _timeCount = 0;

    void assignCurrent(const String& key, const String& value) {
        OpenMeteoCurrentData* d = static_cast<OpenMeteoCurrentData*>(_output);
        if (key == "time") copyString(d->time, sizeof(d->time), value);
        else if (key == "temperature_2m") d->temperature = parseFloatValue(value);
        else if (key == "weather_code") d->weatherCode = parseIntValue(value);
        else if (key == "rain") d->rain = parseFloatValue(value);
        else if (key == "snowfall") d->snowfall = parseFloatValue(value);
        else if (key == "relative_humidity_2m") d->relativeHumidity = parseFloatValue(value);
    }

    void assignArray(const String& key, uint8_t i, const String& value) {
        if (_kind == KIND_DAILY) {
            OpenMeteoDailyData* d = static_cast<OpenMeteoDailyData*>(_output);
            if (key == "time") copyString(d[i].date, sizeof(d[i].date), value);
            else if (key == "temperature_2m_min") d[i].temperatureMin = parseFloatValue(value);
            else if (key == "temperature_2m_max") d[i].temperatureMax = parseFloatValue(value);
            else if (key == "sunrise") copyString(d[i].sunrise, sizeof(d[i].sunrise), value);
            else if (key == "sunset") copyString(d[i].sunset, sizeof(d[i].sunset), value);
            else if (key == "rain_sum") d[i].rainSum = parseFloatValue(value);
            else if (key == "snowfall_sum") d[i].snowfallSum = parseFloatValue(value);
            else if (key == "wind_gusts_10m_max") d[i].windGustsMax = parseFloatValue(value);
            else if (key == "wind_speed_10m_max") d[i].windSpeedMax = parseFloatValue(value);
            else if (key == "moon_phase") d[i].moonPhase = parseFloatValue(value);
            else if (key == "wind_direction_10m_dominant") d[i].windDirectionDominant = parseFloatValue(value);
            else if (key == "apparent_temperature_max") d[i].apparentTemperatureMax = parseFloatValue(value);
            else if (key == "apparent_temperature_min") d[i].apparentTemperatureMin = parseFloatValue(value);
            else if (key == "moonrise") copyString(d[i].moonrise, sizeof(d[i].moonrise), value);
            else if (key == "weather_code") d[i].weatherCode = parseIntValue(value);
        }
        else {
            OpenMeteoHourlyData* d = static_cast<OpenMeteoHourlyData*>(_output);
            if (key == "time") copyString(d[i].time, sizeof(d[i].time), value);
            else if (key == "temperature_2m") d[i].temperature = parseFloatValue(value);
            else if (key == "weather_code") d[i].weatherCode = parseIntValue(value);
            else if (key == "rain") d[i].rain = parseFloatValue(value);
            else if (key == "snowfall") d[i].snowfall = parseFloatValue(value);
            else if (key == "relative_humidity_2m") d[i].relativeHumidity = parseFloatValue(value);
        }
    }
};

open_meteoapi::open_meteoapi()
    : _client(nullptr),
    _caCert(nullptr),
    _latitude(0),
    _longitude(0),
    _forecastDays(1),
    _timeout(15000),
    _lastError(0) {
    _transportError[0] = '\0';
    strlcpy(_timezone, "auto", sizeof(_timezone));
    strlcpy(_temperatureUnit, "celsius", sizeof(_temperatureUnit));
    strlcpy(_windSpeedUnit, "kmh", sizeof(_windSpeedUnit));
    strlcpy(_precipitationUnit, "mm", sizeof(_precipitationUnit));
}

void open_meteoapi::begin(WiFiClientSecure& client, float latitude, float longitude,
    const char* timezone) {
    _client = &client;
    _latitude = latitude;
    _longitude = longitude;
    strlcpy(_timezone, timezone ? timezone : "auto", sizeof(_timezone));
    if (_caCert) _client->setCACert(_caCert);
}

void open_meteoapi::setCACert(const char* rootCA) {
    _caCert = rootCA;
    if (_client && _caCert) _client->setCACert(_caCert);
}

void open_meteoapi::setTemperatureUnit(const char* unit) {
    strlcpy(_temperatureUnit, unit ? unit : "celsius", sizeof(_temperatureUnit));
}
void open_meteoapi::setWindSpeedUnit(const char* unit) {
    strlcpy(_windSpeedUnit, unit ? unit : "kmh", sizeof(_windSpeedUnit));
}
void open_meteoapi::setPrecipitationUnit(const char* unit) {
    strlcpy(_precipitationUnit, unit ? unit : "mm", sizeof(_precipitationUnit));
}
void open_meteoapi::setForecastDays(uint8_t days) {
    _forecastDays = constrain(days, 1, OPEN_METEO_MAX_DAILY);
}
void open_meteoapi::setTimeout(uint16_t milliseconds) { _timeout = milliseconds; }

const char* open_meteoapi::lastErrorMessage() const {
    if (_lastError == ERR_TRANSPORT && _transportError[0]) return _transportError;
    switch (_lastError) {
    case 0: return "Success";
    case ERR_NOT_INITIALIZED: return "Call begin() before requesting data";
    case ERR_BAD_ARGUMENT: return "Invalid output buffer or item count";
    case ERR_TRANSPORT: return "HTTP transport or connection error";
    case ERR_JSON: return "Could not parse Open-Meteo JSON";
    case ERR_MISSING_DATA: return "Response is missing expected data";
    default: return _lastError < 0 ? "Library error" : "HTTP server error";
    }
}

String open_meteoapi::encode(const char* value) {
    String result;
    if (!value) return result;
    while (*value) {
        const char c = *value++;
        if (c == ' ') result += "%20";
        else if (c == '/') result += "%2F";
        else if (c == '+') result += "%2B";
        else result += c;
    }
    return result;
}

String open_meteoapi::makeUrl(const char* section, const char* variables,
    uint8_t count) {
    String url = "https://api.open-meteo.com/v1/forecast?latitude=";
    url += String(_latitude, 4);
    url += "&longitude=";
    url += String(_longitude, 4);
    url += '&';
    url += section;
    url += '=';
    url += variables;
    url += "&timezone=";
    url += encode(_timezone);
    url += "&forecast_days=";
    url += String(_forecastDays);
    url += "&temperature_unit=";
    url += _temperatureUnit;
    url += "&wind_speed_unit=";
    url += _windSpeedUnit;
    url += "&precipitation_unit=";
    url += _precipitationUnit;
    url += "&timeformat=iso8601";
    if (strcmp(section, "hourly") == 0) {
        url += "&forecast_hours=";
        url += String(count);
    }
    return url;
}

int open_meteoapi::request(const char* section, const char* variables,
    uint8_t count, OpenMeteoJsonListener& listener) {
    if (!_client) return _lastError = ERR_NOT_INITIALIZED;
    _transportError[0] = '\0';
    HTTPClient http;
    http.setTimeout(_timeout);
    const String url = makeUrl(section, variables, count);
    if (!http.begin(*_client, url)) {
        strlcpy(_transportError, "HTTPClient rejected the URL/client", sizeof(_transportError));
        return _lastError = ERR_TRANSPORT;
    }
    const int status = http.GET();
    if (status <= 0) {
        String detail = HTTPClient::errorToString(status);
        strlcpy(_transportError, detail.c_str(), sizeof(_transportError));
        http.end();
        return _lastError = ERR_TRANSPORT;
    }
    if (status != HTTP_CODE_OK) {
        http.end();
        return _lastError = status;
    }

    JsonStreamingParser parser;
    parser.setListener(&listener);
    Stream& stream = http.getStream();
    const uint32_t started = millis();
    while (!listener.complete() && millis() - started < _timeout) {
        while (stream.available() && !listener.complete()) {
            parser.parse(static_cast<char>(stream.read()));
        }
        if (!listener.complete() && _client->connected()) delay(1);
        else if (!stream.available()) break;
    }
    http.end();
    if (!listener.complete()) return _lastError = ERR_JSON;
    if (!listener.hasExpectedData()) return _lastError = ERR_MISSING_DATA;
    _lastError = 0;
    return 0;
}

int open_meteoapi::getDaily(OpenMeteoDailyData* out, uint8_t count) {
    if (!out || count == 0 || count > OPEN_METEO_MAX_DAILY)
        return _lastError = ERR_BAD_ARGUMENT;
    setForecastDays(count);
    OpenMeteoJsonListener listener(KIND_DAILY, out, count);
    return request("daily", DAILY_FIELDS, count, listener);
}

int open_meteoapi::getHourly(OpenMeteoHourlyData* out, uint8_t count) {
    if (!out || count == 0 || count > OPEN_METEO_MAX_HOURLY)
        return _lastError = ERR_BAD_ARGUMENT;
    OpenMeteoJsonListener listener(KIND_HOURLY, out, count);
    return request("hourly", HOURLY_FIELDS, count, listener);
}

int open_meteoapi::getCurrent(OpenMeteoCurrentData* out) {
    if (!out) return _lastError = ERR_BAD_ARGUMENT;
    OpenMeteoJsonListener listener(KIND_CURRENT, out, 1);
    return request("current", CURRENT_FIELDS, 1, listener);
}


