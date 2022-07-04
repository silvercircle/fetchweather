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

#ifndef __OPTIONS_H_
#define __OPTIONS_H_

#include "pch.h"

typedef struct _cfg {
    unsigned int apiProvider;
    int version;
    char temp_unit;
    std::string temp_unit_raw;
    std::string config_dir_path;
    std::string config_file_path;
    std::string apikeyFile;
    std::string data_dir_path;
    std::string apikey;
    std::string apiProviderString;
    std::string vis_unit;
    std::string speed_unit;
    std::string pressure_unit;
    std::string output_file;
    std::string location;
    std::string lat, lon;
    std::string timezone;
    bool offline;       // use cache, do not go online
    bool nocache;       // do not refresh cache
    bool skipcache;     // do not use cache
    bool silent;        // don't write to stdout
    bool debug;         // print debugging information, do not produce any output
    bool dumptofile;    // also write result to file, note that output_dir must be set and valid.
    int  forecastDays = 3;
} CFG;

class ProgramOptions {
  public:
    ProgramOptions(const ProgramOptions &) = delete;
    ProgramOptions &operator=(const ProgramOptions &) = delete;

    // Meyer's singleton pattern. This is thread-safe.
    static ProgramOptions &getInstance()
    {
        static ProgramOptions instance;
        volatile int x{};               // otherwise, -O3 might just optimize this away.
        return instance;
    }
    int parse(int argc, char **argv);
    void dumpOptions();
    void flush();
    void print_version();
    QString q;
    void setAppObject(QCoreApplication *the_app) { this->m_app = the_app; }

    const CFG &getConfig()
    { return this->m_config; }

    const QCommandLineParser &getParser()
    { return this->m_Parser; }

    const std::string &getLogFilePath()
    { return this->logfile_path; }

    /*
     * the file names for the two cache files, relativ to CFG.data_dir_path
     * (usually $HOME/.local/share/APPNAME on *iX).
     */
    static inline std::string const _version_number = "0.1.1";
    static inline std::string const _appname = "fetchweather";

    static constexpr std::array<const char*, 3> api_shortcodes = { "CC", "OWM", "VC" };
    static constexpr std::array<const char*, 3> api_readable_names = { "ClimaCell",
                                                                       "OpenWeatherMap",
                                                                       "Visual Crossing"};
    enum { API_CLIMACELL, API_OWM, API_VC, _API_END_ };

  private:
    ProgramOptions();
    ~ProgramOptions()   { };
    CLI::App            m_oCommand;
    std::string         m_name;
    void                _init();
    unsigned int        counter;
    CFG                 m_config;
    std::string         logfile_path, keyfile_path;
    bool                fUseKeyfile = false;
    QCommandLineParser  m_Parser;
    QCoreApplication*   m_app;
};
#endif //__OPTIONS_H_
