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
 */

#ifndef __DATAHANDLER_H_
#define __DATAHANDLER_H_

#include <nlohmann/json.hpp>

/*
 * The data point collects and normalizes data from the API provider
 * it is expected that:
 * a) it is completely populated.
 * b) all values are in the metric system
 */
struct DataPoint {
    bool            valid = false;
    bool            is_day;
    time_t          timeRecorded, sunsetTime, sunriseTime;
    char            timeRecordedAsText[30];
    char            timeZone[128];
    int             weatherCode;
    char            weatherSymbol;
    double          temperature, temperatureApparent, temperatureMin, temperatureMax;
    double          visibility;         // this must be in km (some providers use meters)
    double          windSpeed, windGust;
    double          cloudCover;     // in percent
    double          cloudBase, cloudCeiling;
    int             moonPhase;
    char            moonPhaseAsString[50];
    unsigned int    windDirection;
    int             precipitationType;
    char            precipitationTypeAsString[20];
    double          precipitationProbability, precipitationIntensity;
    double          pressureSeaLevel, humidity, dewPoint;
    char            sunsetTimeAsString[20], sunriseTimeAsString[20], windBearing[10], windUnit[10];
    char            conditionAsString[100];
    double          uvIndex;
    bool            haveUVI;
};

struct DailyForecast {
    char            code;
    double          temperatureMin, temperatureMax;
    char            weekDay[10];
    double          pop;
};

class DataHandler {

  public:
    DataHandler();
    ~DataHandler() { writeToDB(); }

    void doOutput();
    void dumpSnapshot();
    int  run();
    std::pair<std::string, std::string> degToBearing        (unsigned int wind_direction) const;
    std::pair<double, char>             convertTemperature  (double temp, char unit) const;
    double                              convertWindspeed    (double speed) const;
    double                              convertVis          (const double vis) const;
    double                              convertPressure     (double hPa) const;
    const DataPoint&                    getDataPoint        () const { return m_DataPoint; }

    void outputTemperature  (double val, const bool addUnit = false,
                             const char *format = "%.1f%s\n");
    void outputDailyForecast(const bool is_day = true);

    static constexpr const char *wind_directions[] =
      {"N", "NNE", "NE",
       "ENE", "E", "ESE",
       "SE", "SSE", "S",
       "SSW", "SW", "WSW",
       "W", "WNW", "NW",
       "NNW"};

    static constexpr const char *speed_units[] = {"m/s", "kts", "km/h"};
    static constexpr const char *weekDays[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat",
                                               "Sun", "_invalid"};
  protected:
    virtual         bool            readFromCache() = 0;
    virtual         bool            readFromApi() = 0;
    virtual         bool            verifyData() = 0;
    ProgramOptions&                 m_options;
    DataPoint                       m_DataPoint;
    DailyForecast                   m_daily[3];         // 3 days, might be desireable to have this customizable
    nlohmann::json                  result_current, result_forecast;

    std::string                     m_currentCache, m_ForecastCache;
    void writeToDB();

  private:
    std::string                     db_path;
};

#endif //__DATAHANDLER_H_
