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

#include "utils.h"

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
   * @param size      - the length of the chunk
   * @param nmemb
   * @param s         - user-supplied data (std::string *)
   * @return          - the total length of data read
   */
  size_t curl_callback(void *contents, size_t size, size_t nmemb, std::string *s)
  {
      s->append((char *)contents);
      return s->length();
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
}
