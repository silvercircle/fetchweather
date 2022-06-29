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

/**
 * some utility functions for time conversion and CURL handling
 */

#include "utils.h"
#include <vector>

namespace utils {

  time_t ISOToUnixtime(const char *iso_string, GTimeZone *tz)
  {
      GDateTime *g = g_date_time_new_from_iso8601(iso_string, tz);

      time_t _unix = g_date_time_to_unix(g);
      g_date_time_unref(g);
      return _unix;
  }


  time_t ISOToUnixtime(const std::string& iso_string, GTimeZone *tz)
  {
      GDateTime *g = g_date_time_new_from_iso8601(iso_string.c_str(), tz);

      time_t _unix = g_date_time_to_unix(g);
      g_date_time_unref(g);
      return _unix;
  }

  /**
   * This callback is used by curl to store the data.
   *
   * @param contents  - the data chunk read
   * @param size      - the length of the chunk in elements
   * @param nmemb     - the size of one element
   * @param s         - user-supplied data (std::string *)
   * @return          - amount of data read and processed.
   */
  size_t curl_callback(void *contents, size_t size, size_t nmemb, std::string *s)
  {
      s->append((char *)contents);
      return size * nmemb;
  }
  

  int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName)
  {
      int i;
      for(i = 0; i<argc; i++) {
          printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      }
      printf("\n");
      return 0;
  }

  /**
   * fetch a document from the given url.
   *
   * @param url             - the document URI
   * @param parse_result    - json object to be parsed into
   * @param cache           - write the response to this cache file
   * @param skipcache       - skip the caching step (option --skipcache)
   * @return                - 0 for failure, 1 otherwise
   */
  unsigned int curl_fetch(const char *url, nlohmann::json& parse_result, const std::string& cache,
                          bool skipcache)
  {
      unsigned int result = 1;
      std::string response;

      curl_global_init(CURL_GLOBAL_DEFAULT);
      CURL *curl = curl_easy_init();
      if(curl) {
          curl_easy_setopt(curl, CURLOPT_URL, url);
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils::curl_callback);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
          curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
          curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60);

          auto rc = curl_easy_perform(curl);
          if(rc != CURLE_OK) {
              LOG_F(INFO, "curl_easy_perform() failed, return = %s", curl_easy_strerror(rc));
              result = 0;
          } else {
              parse_result = json::parse(response.c_str());
              if(parse_result.empty()) {
                  LOG_F(INFO, "Current forecast: Request failed, no valid data received");
                  result = 0;
              } else {
                  if(skipcache) {
                      LOG_F(INFO, "Current forecast: Skipping cache refresh (--nocache option present)");
                  } else {
                      std::ofstream f(cache);
                      f.write(response.c_str(), response.length());
                      f.flush();
                      f.close();
                  }
              }
          }
      }
      curl_easy_cleanup(curl);
      curl_global_cleanup();

      return result;
  }
} // namespace utils
