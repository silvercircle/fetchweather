/*
 * MIT License
 *
 * Copyright (c) 2021 Alex Vie (silvercircle@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * The implementation for the OpenWeatherMap API.
 */

#include <time.h>
#include <utils.h>
#include "DataHandler_ImplOWM.h"

DataHandler_ImplOWM::DataHandler_ImplOWM() : DataHandler()
{
}

char DataHandler_ImplOWM::getCode(const int weatherCode, const bool daylight)
{
    size_t index = daylight ? 0 : 1;
    if(weatherCode >= 200 && weatherCode <= 299) {       // thunderstorm
        return daylight ? 'k' : 'K';
    }

    if(weatherCode >= 300 && weatherCode <= 399) {       // drizzle
        return 'x';
    }

    if(weatherCode == 800)
        return 'a';

    if(weatherCode >= 500 && weatherCode <= 599) {       // rain
        switch(weatherCode) {
            case 511:
                return 's';
            case 502:
            case 503:
            case 504:
                return 'j';
            default:
                return daylight ? 'g' : 'G';
        }
    }
    if(weatherCode >= 600 && weatherCode <= 699) {          //snow
        switch(weatherCode) {
            case 602:
            case 522:
                return 'w';
            case 504:
                return 'j';
            default:
                return daylight ? 'o' : 'O';
        }
    }
    if(weatherCode >= 800 && weatherCode <= 899) {       // other
        switch(weatherCode) {
            case 801:
                return daylight ? 'b' : 'B';
            case 802:
                return daylight ? 'c' : 'C';
            case 803:
                return daylight ? 'e' : 'f';
            case 804:
                return daylight ? 'd' : 'D';
            default:
                return daylight ? 'c' : 'C';
        }
    }
    if(weatherCode >= 700 && weatherCode <= 799) {
        switch(weatherCode) {
            case 711:
            case 741:
            case 701:
                return '0';
            default:
                return daylight ? 'c' : 'C';
        }
    }

    return 'a';
}

/**
 * implements the API request for OpenWeatherMap. They support single
 * call semantics for getting current + daily forecast.
 *
 * @return      - true if successful.
 */
bool DataHandler_ImplOWM::readFromApi()
{
    const CFG& cfg = m_options.getConfig();
    std::string baseurl("http://api.openweathermap.org/data/2.5/onecall?appid=");
    baseurl.append(cfg.apikey);
    baseurl.append("&lat=").append(cfg.lat).append("&lon=").append(cfg.lon);

    std::string current(baseurl);
    current.append("&exclude=minutely&units=metric");

    const char *url = current.c_str();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    if (cfg.debug) {
        printf("Debug Mode: Attempting to fetch weather from %s\n",
               ProgramOptions::api_readable_names[cfg.apiProvider]);
    }
    // read the current forecast first
    auto result = utils::curl_fetch(current.c_str(), this->result_current,
                                    this->m_currentCache, m_options.getConfig().skipcache);
    if(result) {
        if (!this->result_current["cod"].empty()) {         // field "cod" means error
            LOG_F(INFO,
                  "readFromApi(): Failure, error code = %d, error message = %s",
                  this->result_current["cod"].get<int>(),
                  this->result_current["message"].get<std::string>().c_str());
            return false;
        }
        this->populateSnapshot();
        /**
         * validation
         */
        return (this->result_current["current"].empty() || this->result_current["hourly"].empty()
          || this->result_current["daily"].empty()) ? false : true;
    }
    return false;
}

bool DataHandler_ImplOWM::readFromCache()
{
    LOG_F(INFO, "Attempting to read current from cache: %s", this->m_currentCache.c_str());
    std::ifstream current(this->m_currentCache);
    std::stringstream current_buffer, forecast_buffer;
    current_buffer << current.rdbuf();
    current.close();
    this->result_current = json::parse(current_buffer.str().c_str());

    if (result_current["current"].empty() || result_current["hourly"].empty()
      || result_current["daily"].empty()) {
        LOG_F(INFO, "Cache read from %s failed.", this->m_currentCache.c_str());
        return false;
    } else {
        LOG_F(INFO, "Cache read successful.");
        this->populateSnapshot();
        return true;
    }
}

void DataHandler_ImplOWM::populateSnapshot()
{
    nlohmann::json& d = this->result_current["current"];
    const CFG& cfg = this->m_options.getConfig();
    DataPoint& p = this->m_DataPoint;
    char tmp[128];

    p.weatherCode = d["weather"][0]["id"].is_number() ? d["weather"][0]["id"].get<int>() : 800;

    snprintf(p.timeZone, 127, "%s", this->result_current["timezone"].is_string() ?
             this->result_current["timezone"].get<std::string>().c_str() : "Unknown");

    p.timeRecorded = d["dt"].is_number() ? d["dt"].get<int>() : time(0);

    tm *now = localtime(&p.timeRecorded);
    snprintf(p.timeRecordedAsText, 19, "%02d:%02d", now->tm_hour, now->tm_min);

    p.dewPoint = d["dew_point"].is_number() ?
                 this->convertTemperature(d["dew_point"].get<double>(), cfg.temp_unit).first : 0.0f;

    p.humidity = d["humidity"].is_number() ? d["humidity"].get<double>() : 0.0f;

    p.precipitationProbability = this->result_current["hourly"][0]["pop"].is_number() ?
                                 this->result_current["hourly"][0]["pop"].get<double>() : 0.0f;

    if(!d["rain"].empty()) {
        p.precipitationIntensity = d["rain"]["1h"].is_number() ?
                                   d["rain"]["1h"].get<double>() : 0.0f;
    }

    if(!d["snow"].empty()) {
        p.precipitationIntensity = d["snow"]["1h"].is_number() ?
                                   d["snow"]["1h"].get<double>() : 0.0f;
    }

    if(p.precipitationIntensity > 0) {
        p.precipitationType = 1;
        snprintf(p.precipitationTypeAsString, 19, "%s", d["snow"].empty() ? "Rain" : "Snow");
    }

    p.temperature = d["temp"].is_number() ?
                    this->convertTemperature(d["temp"].get<double>(), cfg.temp_unit).first : 0.0f;

    p.temperatureApparent = d["feels_like"].is_number() ?
                            this->convertTemperature(d["feels_like"].get<double>(), cfg.temp_unit).first : 0;

    p.temperatureMin = this->result_current["daily"][0]["temp"]["min"].is_number() ?
                       this->convertTemperature(this->result_current["daily"][0]["temp"]["min"].get<double>(),
                                                cfg.temp_unit).first : 0;

    p.temperatureMax = this->result_current["daily"][0]["temp"]["max"].is_number() ?
                       this->convertTemperature(this->result_current["daily"][0]["temp"]["max"].get<double>(),
                                                cfg.temp_unit).first : 0;

    // OWM reports vis in meters not miles or km
    p.visibility = d["visibility"].is_number() ? this->convertVis(d["visibility"].get<double>() / 1000) : 0.0f;

    p.pressureSeaLevel = d["pressure"].is_number() ?
                         this->convertPressure(d["pressure"].get<double>()) : 0.0f;

    p.windSpeed = d["wind_speed"].is_number() ? this->convertWindspeed(d["wind_speed"].get<double>()) : 0.0f;
    p.windDirection = d["wind_deg"].is_number() ? d["wind_deg"].get<int>() : 0;
    p.windGust = d["wind_gust"].is_number() ? this->convertWindspeed(d["wind_gust"].get<double>()) : 0.0f;

    auto wind = this->degToBearing(p.windDirection);
    snprintf(p.windBearing, 9, "%s", wind.first.c_str());
    snprintf(p.windUnit, 9, "%s", wind.second.c_str());

    p.sunsetTime = d["sunset"].is_number() ? d["sunset"].get<int>() : 0;
    p.sunriseTime= d["sunrise"].is_number() ? d["sunrise"].get<int>() : 0;

    p.is_day = (p.sunriseTime < p.timeRecorded < p.sunsetTime);

    tm *sunset = localtime(&p.sunsetTime);
    snprintf(tmp, 100, "%02d:%02d", sunset->tm_hour, sunset->tm_min);
    snprintf(p.sunsetTimeAsString, 19, "%s", tmp);
    tm *sunrise = localtime(&p.sunriseTime);
    snprintf(tmp, 100, "%02d:%02d", sunrise->tm_hour, sunrise->tm_min);
    snprintf(p.sunriseTimeAsString, 19, "%s", tmp);

    snprintf(p.conditionAsString, 99, "%s", d["weather"][0]["main"].is_string() ?
      d["weather"][0]["main"].get<std::string>().c_str() : "Unknown");

    p.cloudCover = d["clouds"].is_number() ? d["clouds"].get<double>() : 0;
    // this is not provided by OWM
    p.cloudBase = 0; //d["cloudBase"].is_number() ? d["cloudBase"].get<double>() : 0;
    p.cloudCeiling = 0; //d["cloudCeiling"].is_number() ? d["cloudCeiling"].get<double>() : 0;

    p.uvIndex = d["uvi"].is_number() ? d["uvi"].get<double>() : 0;

    DailyForecast* daily = this->m_daily;
    nlohmann::json& jdaily = this->result_current["daily"];

    for(int i = 0; i < 3; i++) {
        daily[i].temperatureMin = jdaily[i + 1]["temp"]["min"].is_number() ?
          jdaily[i + 1]["temp"]["min"].get<double>() : 0;
        daily[i].temperatureMax = jdaily[i + 1]["temp"]["max"].is_number() ?
          jdaily[i + 1]["temp"]["max"].get<double>() : 0;
        daily[i].code = jdaily[i + 1]["weather"][0]["id"].is_number() ?
          this->getCode(jdaily[i + 1]["weather"][0]["id"].get<int>(), true) : 'a';

        time_t date = jdaily[i +1]["dt"].is_number() ? jdaily[i +1]["dt"].get<unsigned long>() : 0;

        tm *tmdate = localtime(&date);
        strftime(daily[i].weekDay, 9, "%a", tmdate);
    }
    p.weatherSymbol = this->getCode(p.weatherCode, p.is_day);
    p.valid = true;
}