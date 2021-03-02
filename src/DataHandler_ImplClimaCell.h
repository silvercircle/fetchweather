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

#ifndef OBJCTEST_SRC_DATAHANDLER_H_
#define OBJCTEST_SRC_DATAHANDLER_H_

class DataHandler_ImplClimaCell : public DataHandler {
  public:
    DataHandler_ImplClimaCell();
    ~DataHandler_ImplClimaCell() { writeToDB(); }

    bool readFromCache();
    bool readFromAPI();
    int  run();

    const char*                         getCondition        (int weatherCode);
    const char*                         getPrecipType       (int code) const;

    char getCode            (const int weatherCode, const bool daylight = true);
    void populateSnapshot   ();

    static constexpr const char *precipType[] = { "", "(Rain)", "(Snow)", "(Freezing Rain)", "(Ice Pellets)" };
  private:
    std::map<int, const char *>     m_conditions;
    std::map<int, const char *>     m_icons;
};

#endif //OBJCTEST_SRC_DATAHANDLER_H_
