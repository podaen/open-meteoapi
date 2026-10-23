# Open-Meteo ESP32 library

## How to install

1. Install the `JsonStreamingParser` dependency using the Arduino IDE Library Manager, or download and install it as a ZIP library.
2. Install this library by selecting **Sketch → Include Library → Add .ZIP Library…** and choosing the Open-Meteo library ZIP.
3. Include `<open_meteoapi.h>` in your sketch.
4. For HTTPS certificate verification, create the certificate header described below, call `weather.setCACert(rootCA)`, and synchronize the ESP32 clock after Wi-Fi connects.

## Prepare the PEM root certificate

You do not create or sign a certificate yourself. Download the public root CA certificate that the server's certificate chain connects to. For the chain used by this example, use the official **ISRG Root X1 self-signed PEM** from [Let's Encrypt's certificate page](https://letsencrypt.org/ca/certificates/).

1. Open the Let's Encrypt certificate page and find **ISRG Root X1**. Use the **PEM** download for the **self-signed** certificate, not a cross-signed version.
2. Open the downloaded PEM file in a text editor. It should contain a certificate block beginning with `-----BEGIN CERTIFICATE-----` and ending with `-----END CERTIFICATE-----`.
3. In the folder containing your Arduino `.ino` sketch, create a file named `Cert_open-meteo.h`.
4. Put the following declaration in that header and replace the instruction line with every base64 text line from the downloaded file:

    #pragma once

    static const char rootCA[] = R"PEM(
    -----BEGIN CERTIFICATE-----
    paste every certificate body line here
    -----END CERTIFICATE-----
    )PEM";

5. Keep the PEM boundary lines exactly as shown: five hyphens on each side of BEGIN and END, with no added spaces or punctuation. Copy the entire body without adding quotes or `\n` to the ends of lines. Do not paste OpenSSL diagnostic text such as certificate-chain headings, fingerprints, or verification messages.
6. Include the header in the sketch with `#include "Cert_open-meteo.h"`. After Wi-Fi connects and the ESP32 clock has synchronized, configure the client:

    weather.begin(secureClient, 50.741f, 4.0067f, "Europe/Berlin");
    weather.setCACert(rootCA);

Do not call `secureClient.setInsecure()`: it turns off certificate verification. If TLS reports that the certificate format is invalid, check for incorrect PEM boundary hyphens, missing certificate body lines, or extra text copied into the string.

## Set up the forecast in Open-Meteo Docs

Open the [Open-Meteo Weather Forecast API docs](https://open-meteo.com/en/docs). The page is an interactive request builder: select the location and variables, then review the generated API URL and response preview.

1. **Set the location.** Enter decimal latitude and longitude. The example uses latitude `50.741` and longitude `4.0067`. For locations west of Greenwich, longitude is negative.
2. **Set the timezone.** Choose the timezone for the location, such as `Europe/Berlin`. The API requires a timezone when daily values are requested; it also controls the local timestamps and day boundaries in the response.
3. **Select the variables.** The current library requests a fixed set of variables. To see those fields in the docs response, select:
   - **Daily:** minimum/maximum temperature, apparent minimum/maximum temperature, sunrise, sunset, rain sum, snowfall sum, maximum wind gusts, maximum wind speed, moon phase, dominant wind direction, moonrise, and weather code.
   - **Hourly:** temperature at 2 m, weather code, rain, snowfall, and relative humidity at 2 m.
   - **Current:** temperature at 2 m, weather code, rain, snowfall, and relative humidity at 2 m.
4. **Choose the forecast length.** Daily results are controlled by the count passed to `getDaily()`. For example, `getDaily(daily, 1)` requests one day. Hourly results are controlled by the count passed to `getHourly()`; `getHourly(hourly, 5)` requests five hourly records. The docs have forecast-days and forecast-hours controls for trying these settings in the browser.
5. **Match units.** The library defaults to Celsius, km/h, and mm. The docs use those same defaults. To choose other supported units, use `setTemperatureUnit("fahrenheit")`, `setWindSpeedUnit("mph")`, or `setPrecipitationUnit("inch")` before requesting data.
6. **Check the preview.** Compare the generated API URL and JSON response with your sketch's location, timezone, selected fields, count, and units. The library only parses the fixed variables listed above; selecting additional variables in the docs does not add them to the library structs.

The API docs list the available hourly and daily variables. To find meanings for `weather_code`, scroll down to **Weather variable documentation → WMO Weather interpretation codes (WW)**. The API returns a number; the table maps it to a description such as `0 = Clear sky`, `3 = Overcast`, `61 = Slight rain`, or `95 = Thunderstorm`. For the complete and current table, use the [WMO weather-code table in the API docs](https://open-meteo.com/en/docs).

## Two ways to receive daily data

The library lets your sketch receive results in either an array of data structs or one data struct. The array example requests two days and prints only the first; the single-struct example requests one day. Use the array form when you want to collect multiple records, for example before sending them with ESP-NOW.

### Array of structs

    OpenMeteoDailyData dataD[2];

    int result = weather.getDaily(dataD, 2);
    if (result == 0) {
      printDaily(dataD[0]);  // Print only the first day
    } else {
      Serial.printf("Daily request failed (%d): %s\n",
                    result, weather.lastErrorMessage());
    }

To send records using ESP-NOW, pass the selected struct (for example, `dataD[0]`) to your ESP-NOW send routine. Check the struct size against the ESP-NOW payload limit and use matching data definitions on both devices.

### One struct

    OpenMeteoDailyData daily;

    int result = weather.getDaily(&daily, 1);
    if (result == 0) {
      printDaily(daily);
    } else {
      Serial.printf("Daily request failed (%d): %s\n",
                    result, weather.lastErrorMessage());
    }

Both snippets assume this printer function is already defined:

    void printDaily(const OpenMeteoDailyData& d) {
      Serial.printf("\nDaily %s\n", d.date);
      Serial.printf("  Temperature min/max: %.1f / %.1f C\n",
                    d.temperatureMin, d.temperatureMax);
      Serial.printf("  Rain/snow: %.1f / %.1f mm\n", d.rainSum, d.snowfallSum);
      Serial.printf("  Weather code: %d\n", d.weatherCode);
    }



The output from the example sketch looks like this for the data structs (advanced example):

    Opening port
    Port open
    ...........
    Connected IP address : 192.168.0.200
    .
    Wi-Fi connected
    Waiting for clock........
    Unix time: 1792765296
    Starting daily request...
    Daily request returned after 3321 ms

    Daily 2026-10-23
      Temperature min/max: 7.2 / 12.4 C
      Apparent min/max: 1.3 / 9.7 C
      Sunrise/sunset: 2026-10-23T08:20 / 2026-10-23T18:35
      Rain/snow: 22.5 / 0.0 mm
      Max wind speed/gust: 50.4 / 93.2 km/h
      Dominant wind direction: 196 degrees
      Moon phase/rise: 0.40 / 2026-10-23T17:17
      Weather code: 63
    Starting hourly request...
    Hourly request returned after 3288 ms

    Hourly 2026-10-23T16:00
      Temperature: 11.4 C
      Rain/snow: 3.9 / 0.0 mm
      Relative humidity: 93%
      Weather code: 63

    Hourly 2026-10-23T17:00
      Temperature: 12.4 C
      Rain/snow: 2.4 / 0.0 mm
      Relative humidity: 92%
      Weather code: 61

    Hourly 2026-10-23T18:00
      Temperature: 11.9 C
      Rain/snow: 2.2 / 0.0 mm
      Relative humidity: 90%
      Weather code: 61

    Hourly 2026-10-23T19:00
      Temperature: 10.7 C
      Rain/snow: 1.3 / 0.0 mm
      Relative humidity: 91%
      Weather code: 61

    Hourly 2026-10-23T20:00
      Temperature: 8.8 C
      Rain/snow: 0.3 / 0.0 mm
      Relative humidity: 90%
      Weather code: 51
    Starting current request...
    Current request returned after 3286 ms

    Current conditions at 2026-10-23T16:15
      Temperature: 11.6 C
      Rain/snow: 0.6 / 0.0 mm
      Relative humidity: 93%
      Weather code: 61
    All requests completed

    Current conditions at 2026-10-23T14:15
      Temperature: 11.2 C
      Rain/snow: 0.1 / 0.0 mm
      Relative humidity: 89%
      Weather code: 51
    All requests completed


