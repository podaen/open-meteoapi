#pragma once
//#ifndef CONFIG_H
//#define CONFIG_H
#include <stdint.h>

namespace OpenMeteoConfig {
    // Location
    static const float LATITUDE = 50.741f;
    static const float LONGITUDE = 4.0067f;
    static const char* const ELEVATION = "auto";

    // Time and forecast range
    static const char* const TIMEZONE = "Europe/Berlin";
    static const char* const TIME_FORMAT = "iso8601";
    static const uint8_t FORECAST_DAYS = 7;
    static const uint8_t PAST_DAYS = 0;
    static const uint8_t HOURLY_COUNT = 5;   // max 48

    // Units and optional API selections
    static const char* const TEMPERATURE_UNIT = "celsius";
    static const char* const WIND_SPEED_UNIT = "kmh";
    static const char* const PRECIPITATION_UNIT = "mm";
    static const char* const CELL_SELECTION = "land";
    static const char* const MODEL = "";

    // Requested weather variables
    struct VariableOption {
        const char* name;
        bool enabled;
    };

    static const VariableOption CURRENT_FIELDS[] = {
        { "temperature_2m", true },
        { "relative_humidity_2m", true },
        { "apparent_temperature", false },
        { "is_day", false },
        { "precipitation", false },
        { "rain", true },
        { "showers", false },
        { "snowfall", true },
        { "snow_depth", false },
        { "weather_code", true },
        { "cloud_cover", false },
        { "pressure_msl", false },
        { "surface_pressure", false },
        { "wind_speed_10m", false },
        { "wind_direction_10m", false },
        { "wind_gusts_10m", false }
    };

    static const VariableOption HOURLY_FIELDS[] = {
        { "temperature_2m", true },
        { "relative_humidity_2m", true },
        { "dew_point_2m", false },
        { "apparent_temperature", false },
        { "precipitation_probability", false },
        { "precipitation", false },
        { "rain", true },
        { "showers", false },
        { "snowfall", true },
        { "snow_depth", false },
        { "weather_code", true },
        { "pressure_msl", false },
        { "surface_pressure", false },
        { "cloud_cover", false },
        { "cloud_cover_low", false },
        { "cloud_cover_mid", false },
        { "cloud_cover_high", false },
        { "visibility", false },
        { "evapotranspiration", false },
        { "et0_fao_evapotranspiration", false },
        { "vapour_pressure_deficit", false },
        { "wind_speed_10m", false },
        { "wind_speed_80m", false },
        { "wind_speed_120m", false },
        { "wind_speed_180m", false },
        { "wind_direction_10m", false },
        { "wind_direction_80m", false },
        { "wind_direction_120m", false },
        { "wind_direction_180m", false },
        { "wind_gusts_10m", false },
        { "is_day", false },
        { "shortwave_radiation", false },
        { "direct_radiation", false },
        { "diffuse_radiation", false },
        { "direct_normal_irradiance", false },
        { "global_tilted_irradiance", false },
        { "terrestrial_radiation", false },
        { "uv_index", false },
        { "uv_index_clear_sky", false },
        { "soil_temperature_0cm", false },
        { "soil_temperature_6cm", false },
        { "soil_temperature_18cm", false },
        { "soil_temperature_54cm", false },
        { "soil_moisture_0_to_1cm", false },
        { "soil_moisture_1_to_3cm", false },
        { "soil_moisture_3_to_9cm", false },
        { "soil_moisture_9_to_27cm", false },
        { "soil_moisture_27_to_81cm", false }
    };

    static const VariableOption DAILY_FIELDS[] = {
        { "weather_code", true },
        { "temperature_2m_max", true },
        { "temperature_2m_min", true },
        { "apparent_temperature_max", true },
        { "apparent_temperature_min", true },
        { "sunrise", true },
        { "sunset", true },
        { "daylight_duration", false },
        { "sunshine_duration", false },
        { "precipitation_sum", false },
        { "rain_sum", true },
        { "showers_sum", false },
        { "snowfall_sum", true },
        { "precipitation_hours", false },
        { "precipitation_probability_max", false },
        { "precipitation_probability_mean", false },
        { "shortwave_radiation_sum", false },
        { "et0_fao_evapotranspiration", false },
        { "uv_index_max", false },
        { "uv_index_clear_sky_max", false },
        { "wind_speed_10m_max", true },
        { "wind_gusts_10m_max", true },
        { "wind_direction_10m_dominant", true },
        { "moon_phase", true },
        { "moonrise", true },
        { "moonset", false }
    };
}
//#endif