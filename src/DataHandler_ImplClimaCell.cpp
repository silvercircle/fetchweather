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
 * This class handles API specific stuff for the ClimaCell Weather API.
 */

#include <time.h>
#include <utils.h>
#include "DataHandler_ImplClimaCell.h"

/**
 * c'tor for DataHandler. Sets up database path and dispatches
 * data reading, either from cache or from the network.
 */
DataHandler_ImplClimaCell::DataHandler_ImplClimaCell() : DataHandler(),
    /*
     * map weatherCodes to weather conditions.
     */
    m_conditions {
      {1000, "Clear"},              {1001, "Cloudy"},
      {1100, "Mostly Clear"},       {1101, "Partly Cloudy"},
      {1102, "Mostly Cloudy"},      {2000, "Fog"},
      {2100, "Light Fog"},          {3000, "Light Wind"},
      {3001, "Wind"},               {3002, "Strong Wind"},
      {4000, "Drizzle"},            {4001, "Rain"},
      {4200, "Light Rain"},         {4201, "Heavy Rain"},
      {5000, "Snow"},               {5001, "Flurries"},
      {5100, "Light Snow"},         {5101, "Heavy Snow"},
      {6000, "Freezing Drizzle"},   {6001, "Freezing Rain"},
      {6200, "Light Freezing Rain"},{6201, "Heavy Freezing Rain"},
      {7000, "Ice Pellets"},        {7001, "Heavy Ice Pellets"},
      {7102, "Light Ice Pellets"},  {8000, "Thunderstorm"} },
   /*
    * The following table maps weatherCodes from the API to characters used by
    * the conkyweather font to display weather icons. The string is always 2
    * characters, the first one for the day icon, the 2nd for the night icon.
    */
    m_icons {
      {1000, "aA"},                 {1001, "ef"},
      {1100, "bB"},                 {1101, "cC"},
      {1102, "dD"},                 {2000, "00"},
      {2100, "77"},                 {3000, "99"},
      {3001, "99"},                 {3002, "23"},
      {4000, "xx"},                 {4001, "gG"},
      {4200, "gg"},                 {4201, "jj"},
      {5000, "oO"},                 {5001, "xx"},
      {5100, "oO"},                 {5101, "ww"},
      {6000, "xx"},                 {6001, "yy"},
      {6200, "ss"},                 {6201, "yy"},
      {7000, "uu"},                 {7001, "uu"},
      {7102, "uu"},                 {8000, "kK"} }
{

}

char DataHandler_ImplClimaCell::getCode(const int weatherCode, const bool daylight)
{
    if (DataHandler_ImplClimaCell::m_icons.find(weatherCode)
      != DataHandler_ImplClimaCell::m_icons.end()) {
        return DataHandler_ImplClimaCell::m_icons[weatherCode][daylight ? 0 : 1];
    }
    return 'a';
}

/**
 * attempt to read current and forecast data from cached JSON
 *
 * @return  true if successful, false otherwise.
 */
bool DataHandler_ImplClimaCell::readFromCache()
{
    LOG_F(INFO, "Attempting to read current from cache: %s", this->m_currentCache.c_str());
    std::ifstream current(this->m_currentCache);
    std::stringstream current_buffer, forecast_buffer;
    current_buffer << current.rdbuf();
    current.close();
    this->result_current = json::parse(current_buffer.str().c_str());
    LOG_F(INFO, "Attempting to read forecast from cache: %s", this->m_currentCache.c_str());
    std::ifstream forecast(this->m_currentCache);
    forecast_buffer << forecast.rdbuf();
    //forecast_buffer.seekg(0, std::ios::end);
    forecast.close();
    this->result_forecast = json::parse(forecast_buffer.str());

    if(!result_current["data"].empty() && !result_forecast["data"].empty()) {
        LOG_F(INFO, "Cache read successful.");
        printf("Trying to populate snapshot\n");
        this->populateSnapshot();
        printf("Snapshot populated\n");
        return true;
    }
    return false;
}

/**
 * fetch data from network API and populate the json object
 * unlike darksky, which allowed for a single-call request with all data,
 * ClimaCell does not. We need to perform two requests. One detail request for the
 * current weather, and one forecast request to get data for the next 3-5 days.
 *
 * @return      - true if successful.
 */
bool DataHandler_ImplClimaCell::readFromApi()
{
    const CFG& cfg = m_options.getConfig();
    bool fSuccess_current = true;
    bool fSuccess_forecast = true;
    std::string baseurl("https://data.climacell.co/v4/timelines?&apikey=");
    baseurl.append(cfg.apikey);
    baseurl.append("&location=");
    baseurl.append(cfg.location);
    if(cfg.timezone.length() > 0) {
        baseurl.append("&timezone=");
        baseurl.append(cfg.timezone);
    }
    std::string current(baseurl);
    current.append("&fields=weatherCode,temperature,temperatureApparent,visibility,windSpeed,windDirection,");
    current.append("precipitationType,precipitationProbability,pressureSeaLevel,windGust,cloudCover,cloudBase,");
    current.append("cloudCeiling,humidity,precipitationIntensity,dewPoint&timesteps=current&units=metric");
    //std::cout << current << std::endl;

    std::string daily(baseurl);
    daily.append("&fields=weatherCode,temperatureMax,temperatureMin,sunriseTime,sunsetTime,moonPhase,");
    daily.append("precipitationType,precipitationProbability&timesteps=1d&startTime=");

    if(cfg.debug) {
        printf("Debug Mode: Attempting to fetch weather from %s\n", ProgramOptions::api_readable_names[cfg.apiProvider]);
    }

    /*
     * figure out the startTime parameter for the forcast. It needs to be UTC and tomorrow
     */

    GDateTime *g = g_date_time_new_now_local();                  // now

    char cl[128];
    snprintf(cl, 40, "%04d-%02d-%02dT23:00:00Z",
             g_date_time_get_year(g),
             g_date_time_get_month(g), g_date_time_get_day_of_month(g));

    std::string _tmp(cl);
    daily.append(_tmp);

    /*
     * calculate the end date, we need 5 days at max for our forecast
     */
    GDateTime *g_end = g_date_time_add_days(g, 5);     // 5 days forecast
    snprintf(cl, 40, "%04d-%02d-%02dT06:00:00Z",
             g_date_time_get_year(g_end),
             g_date_time_get_month(g_end), g_date_time_get_day_of_month(g_end));

    g_date_time_unref(g_end);
    g_date_time_unref(g);

    daily.append("&endTime=");
    _tmp.assign(cl);
    daily.append(_tmp);
    const char *url = current.c_str();

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
        /**
         * validation
         */
        fSuccess_current = !this->result_current["data"].empty();
    } else {
        fSuccess_current = false;
    }
    // now the daily forecast

    result = utils::curl_fetch(daily.c_str(), this->result_forecast,
                                    this->m_ForecastCache, m_options.getConfig().skipcache);
    if(result) {
        if (!this->result_current["cod"].empty()) {         // field "cod" means error
            LOG_F(INFO,
                  "readFromApi(): Failure, error code = %d, error message = %s",
                  this->result_current["cod"].get<int>(),
                  this->result_current["message"].get<std::string>().c_str());
            return false;
        }
        /**
         * validation
         */
        fSuccess_forecast = !this->result_current["data"].empty();
    } else {
        fSuccess_forecast = false;
    }
    if (fSuccess_forecast && fSuccess_current) {
        LOG_F(INFO, "CC:readFromApi(): read successful, populating snapshot");
        this->populateSnapshot();
        return true;
    }
    return false;
}

/**
 * this records the JSON data in a struct and does some sanity checks.
 * It also does unit conversions
 *
 * m_DataPoint is then used for output, database recording and dumping the
 * data to a file (optional)
 */
void DataHandler_ImplClimaCell::populateSnapshot()
{
    // shortcuts
    nlohmann::json& d =     this->result_current["data"]["timelines"][0]["intervals"][0]["values"];
    nlohmann::json& df =    this->result_forecast["data"]["timelines"][0]["intervals"][0]["values"];
    DataPoint& p = this->m_DataPoint;
    const CFG& cfg = this->m_options.getConfig();
    char tmp[128];

    if(d["weatherCode"].empty())
        return; // datapoint likely not valid

    p.weatherCode = d["weatherCode"].is_number() ? d["weatherCode"].get<int>() : 0;
    snprintf(p.timeZone, SIZEOF(p.timeZone), "%s", m_options.getConfig().timezone.c_str());

    p.timeRecorded = time(0);
    tm *now = localtime(&p.timeRecorded);
    snprintf(p.timeRecordedAsText, 19, "%02d:%02d", now->tm_hour, now->tm_min);

    p.dewPoint = d["dewPoint"].is_number() ?
      this->convertTemperature(d["dewPoint"].get<double>(), cfg.temp_unit).first : 0.0f;

    p.humidity = d["humidity"].is_number() ?
      d["humidity"].get<double>() : 0.0f;

    p.precipitationProbability = d["precipitationProbability"].is_number() ?
      d["precipitationProbability"].get<double>() : 0.0f;

    p.precipitationIntensity = d["precipitationIntensity"].is_number() ?
      d["precipitationIntensity"].get<double>() : 0.0f;

    p.temperature = d["temperature"].is_number() ?
      this->convertTemperature(d["temperature"].get<double>(), cfg.temp_unit).first : 0.0f;

    p.temperatureApparent = d["temperatureApparent"].is_number() ?
      this->convertTemperature(d["temperatureApparent"].get<double>(), cfg.temp_unit).first : 0;

    p.temperatureMin = df["temperatureMin"].is_number() ?
      this->convertTemperature(df["temperatureMin"].get<double>(),
        cfg.temp_unit).first : 0;

    p.temperatureMax = df["temperatureMax"].is_number() ?
      this->convertTemperature(df["temperatureMax"].get<double>(),
        cfg.temp_unit).first : 0;

    p.visibility = d["visibility"].is_number() ? this->convertVis(d["visibility"].get<double>()) : 0.0f;
    p.pressureSeaLevel = d["pressureSeaLevel"].is_number() ?
      this->convertPressure(d["pressureSeaLevel"].get<double>()) : 0.0f;

    p.windSpeed = d["windSpeed"].is_number() ? this->convertWindspeed(d["windSpeed"].get<double>()) : 0.0f;
    p.windDirection = d["windDirection"].is_number() ? d["windDirection"].get<int>() : 0;
    p.windGust = d["windGust"].is_number() ? this->convertWindspeed(d["windGust"].get<double>()) : 0.0f;

    auto wind = this->degToBearing(p.windDirection);
    snprintf(p.windBearing, 9, "%s", wind.first.c_str());
    snprintf(p.windUnit, 9, "%s", wind.second.c_str());

    p.sunsetTime = df["sunsetTime"].is_string() ?
      utils::ISOToUnixtime(df["sunsetTime"].get<std::string>(), 0) : 0;
    p.sunriseTime= df["sunriseTime"].is_string() ?
      utils::ISOToUnixtime(df["sunriseTime"].get<std::string>(), 0) : 0;

    p.is_day = (p.sunriseTime < p.timeRecorded < p.sunsetTime);

    tm *sunset = localtime(&p.sunsetTime);
    snprintf(tmp, 100, "%02d:%02d", sunset->tm_hour, sunset->tm_min);
    snprintf(p.sunsetTimeAsString, 19, "%s", tmp);
    tm *sunrise = localtime(&p.sunriseTime);
    snprintf(tmp, 100, "%02d:%02d", sunrise->tm_hour, sunrise->tm_min);
    snprintf(p.sunriseTimeAsString, 19, "%s", tmp);

    p.precipitationType = d["precipitationType"].is_number() ? d["precipitationType"].get<int>() : 0;
    snprintf(p.precipitationTypeAsString, 19, "%s", this->getPrecipType(p.precipitationType));
    snprintf(p.conditionAsString, 99, "%s", this->getCondition(p.weatherCode));

    p.weatherSymbol = this->getCode(p.weatherCode, p.is_day);

    p.cloudCover = d["cloudCover"].is_number() ? d["cloudCover"].get<double>() : 0;
    p.cloudBase = d["cloudBase"].is_number() ? d["cloudBase"].get<double>() : 0;
    p.cloudCeiling = d["cloudCeiling"].is_number() ? d["cloudCeiling"].get<double>() : 0;

    p.moonPhase = this->result_forecast["data"]["timelines"][0]["intervals"][0]["values"]["moonPhase"].is_number() ?
      this->result_forecast["data"]["timelines"][0]["intervals"][0]["values"]["moonPhase"].get<int>() : 0;

    p.valid = true;
    LOG_F(INFO, "DataHandler::populateSnapshot(): snapshot populated successfully.");
#if __clang_major__ >= 8
    //__builtin_dump_struct(&m_DataPoint, &printf);
#endif

    // populate the 3 day forecast
    p.haveUVI = false;
    DailyForecast* daily = this->m_daily;

    for(int i = 0; i < 3; i++) {
        printf("forecasting day: %d\n", i+1);
        int weatherCode = this->result_forecast["data"]["timelines"][0]["intervals"][i
          + 1]["values"]["weatherCode"].is_number() ?
                          this->result_forecast["data"]["timelines"][0]["intervals"][i
                            + 1]["values"]["weatherCode"].get<int>() : 1000;

        if (DataHandler_ImplClimaCell::m_icons.find(weatherCode)
          != DataHandler_ImplClimaCell::m_icons.end()) {
            daily[i].code = DataHandler_ImplClimaCell::m_icons[weatherCode][0];
        } else {
            daily[i].code = 'a';
        }
        daily[i].temperatureMax = this->result_forecast["data"]["timelines"][0]["intervals"][i
          + 1]["values"]["temperatureMax"].is_number() ?
                                  this->result_forecast["data"]["timelines"][0]["intervals"][i
                                    + 1]["values"]["temperatureMax"].get<double>() : 0.0f;

        daily[i].temperatureMin = this->result_forecast["data"]["timelines"][0]["intervals"][i
          + 1]["values"]["temperatureMin"].is_number() ?
                                  this->result_forecast["data"]["timelines"][0]["intervals"][i
                                    + 1]["values"]["temperatureMin"].get<double>() : 0.0f;

        printf("forecasting day: %d, sunrise time\n", i+1);

        GDateTime *g =
          g_date_time_new_from_iso8601(result_forecast["data"]["timelines"][0]["intervals"][i
            + 1]["values"]["sunriseTime"].get<std::string>().c_str(), 0);
        gint weekday = g_date_time_get_day_of_week(g);
        g_date_time_unref(g);

        if (weekday >= 1 && weekday <= 7) {
            snprintf(daily[i].weekDay, 5, "%s", DataHandler::weekDays[weekday - 1]);
        } else {
            snprintf(daily[i].weekDay, 5, "%s", DataHandler::weekDays[7]);       // print "invalid"
        }
    }
    if(this->m_options.getConfig().debug) {
        this->dumpSnapshot();
    }
}

const char *DataHandler_ImplClimaCell::getCondition(int weatherCode)
{
    if(DataHandler_ImplClimaCell::m_conditions.find(weatherCode) != DataHandler_ImplClimaCell::m_icons.end()) {
        return DataHandler_ImplClimaCell::m_conditions[weatherCode];
    }
    return "UNDEFINED";
}

const char *DataHandler_ImplClimaCell::getPrecipType(int code) const
{
    code = (code > 4 || code < 0) ? 0 : code;
    return DataHandler_ImplClimaCell::precipType[code];
}

/**
 * this verify the json data received for plausability
 * TODO implement this
 * @return  true, if checks passed
 */
bool DataHandler_ImplClimaCell::verifyData()
{
    return true;
}
