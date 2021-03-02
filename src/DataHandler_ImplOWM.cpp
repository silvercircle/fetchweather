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

/**
 * implements the API request for OpenWeatherMap. They support single
 * call semantics for getting current + daily forecast.
 *
 * @return      - true if successful.
 */
bool DataHandler_ImplOWM::readFromApi()
{
    const CFG& cfg = m_options.getConfig();
    bool fSuccess = true;
    std::string baseurl("https://api.openweathermap.org/data/2.5/onecall?appid=");
    baseurl.append(cfg.apikey);
    baseurl.append("&lat=").append(cfg.lat).append("&lon=").append(cfg.lon);

    std::string current(baseurl);
    current.append("&exclude=hourly,minutely&units=metric");

    const char *url = current.c_str();
    curl_global_init(CURL_GLOBAL_DEFAULT);

    // read the current forecast first
    CURL *curl = curl_easy_init();
    if(curl) {
        std::string response;
        const char *url = current.c_str();
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::curl_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        auto rc = curl_easy_perform(curl);
        if(rc != CURLE_OK) {
            LOG_F(INFO, "curl_easy_perform() failed, return = %s", curl_easy_strerror(rc));
            fSuccess = false;
        } else {
            std::cout << response << std::endl;
            this->result_current = json::parse(response.c_str());
            if(result_current["data"].empty()) {
                LOG_F(INFO, "Current forecast: Request failed, no valid data received");
            } else if(m_options.getConfig().nocache) {
                LOG_F(INFO, "Current forecast: Skipping cache refresh (--nocache option present)");
            } else {
                std::ofstream f(this->m_currentCache);
                f.write(response.c_str(), response.length());
                f.flush();
                f.close();
            }
        }
    }
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    this->populateSnapshot();
    return (fSuccess && !this->result_current["data"].empty() && !this->result_forecast["data"].empty());
}

bool DataHandler_ImplOWM::readFromCache()
{
    return true;
}

void DataHandler_ImplOWM::populateSnapshot()
{

}