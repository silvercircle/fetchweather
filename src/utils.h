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
 * DataHandler does the majority of the work. It reads data, builds the json and
 * generates the formatted output.
 */

#ifndef __UTILS_H_
#define __UTILS_H_

#include <time.h>
#include <glib-2.0/glib.h>
#include <ctime>

namespace utils {
  time_t ISOToUnixtime(const char *iso_string, GTimeZone *tz = 0);
  time_t ISOToUnixtime(const std::string& s, GTimeZone *tz = 0);
  size_t curl_callback(void *contents, size_t size, size_t nmemb, std::string *s);
  int sqlite_callback(void *NotUsed, int argc, char **argv, char **azColName);
  unsigned int curl_fetch(const char *url, nlohmann::json& parse_result, const std::string& cache,
                          bool skipcache = false);

  /**
   * a couple of funtions to trim strings left, right and on both sides
   *
   * there are two versions of each. Inline and copying.
   */
  inline void ltrim(std::string &s)
  {
      s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
      }));
  }

  inline void rtrim(std::string &s)
  {
      s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
      }).base(), s.end());
  }

  inline void trim(std::string &s)
  {
      ltrim(s);
      rtrim(s);
  }

  inline std::string ltrim_copy(std::string s)
  {
      ltrim(s);
      return s;
  }

  inline std::string rtrim_copy(std::string s)
  {
      rtrim(s);
      return s;
  }

  inline std::string trim_copy(std::string s)
  {
      trim(s);
      return s;
  }
}
#endif //__UTILS_H_
