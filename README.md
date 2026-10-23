# Open-Meteo ESP32 library

## How to install

1. Install the `JsonStreamingParser` dependency from https://github.com/squix78/json-streaming-parser, or download and install it as a ZIP library.
2. Install my library to connect to the wifi network https://github.com/podaen/wifi-connectapp 
3. Install this library by selecting **Sketch → Include Library → Add .ZIP Library…** and choosing the Open-Meteo library ZIP.
4. Include `<open_meteoapi.h>` in your sketch.
5. For HTTPS certificate verification, create the certificate header described below, call `weather.setCACert(rootCA)`, and synchronize the ESP32 clock after Wi-Fi connects.

## Prepare the PEM root certificate

You do not create or sign a certificate yourself. Download the public root CA certificate that the server's certificate chain connects to. For the chain checked in this example, use the official **ISRG Root X1 self-signed PEM** from [Let's Encrypt's certificate page](https://letsencrypt.org/ca/certificates/).

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

The examples below assume Wi-Fi is already connected and the weather client has been configured with `begin()` and `setCACert()`. They show just the daily request and output.

## Two ways to receive daily data

The library lets your sketch receive results in either an array of data structs or one data struct. Both examples below request two forecast days. The array example prints only the first day; use that form if you want to transmit multiple records with ESP-NOW. The single-struct example requests and prints just one day.

### Array of structs

This can collect more than one daily result. Here it requests two days and prints only `dataD[0]`.

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

Use one struct when the sketch needs only one daily result. The count argument is `1`.

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

    Port closed
    Uploading to I/O board
    Opening port
    Port open
    VMDPR_login to WiFi
    Begin Provisioning using Soft AP
    ..............
    Connected IP address : 192.168.0.200
    .
    Wi-Fi connected
    Waiting for clock.
    Unix time: 1792758192
    Starting daily request...
    Daily request returned after 3335 ms

    Daily 2026-10-23
      Temperature min/max: 7.4 / 12.3 C
      Apparent min/max: 1.8 / 9.7 C
      Sunrise/sunset: 2026-10-23T08:20 / 2026-10-23T18:35
      Rain/snow: 19.0 / 0.0 mm
      Max wind speed/gust: 50.8 / 98.3 km/h
      Dominant wind direction: 196 degrees
      Moon phase/rise: 0.40 / 2026-10-23T17:17
      Weather code: 63
    Starting hourly request...
    Hourly request returned after 3325 ms

    Hourly 2026-10-23T14:00
      Temperature: 11.1 C
      Rain/snow: 0.9 / 0.0 mm
      Relative humidity: 89%
      Weather code: 53
    Starting current request...
    Current request returned after 3324 ms

Current conditions at 2026-10-23T14:15
  Temperature: 11.2 C
  Rain/snow: 0.1 / 0.0 mm
  Relative humidity: 89%
  Weather code: 51
All requests completed


