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

#ifndef OBJCTEST_OPTIONS_H
#define OBJCTEST_OPTIONS_H

#ifdef __OPTIONS_INTERNAL
#endif

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
    std::string output_dir;
    std::string location;
    std::string lat, lon;
    std::string timezone;
    bool offline, nocache, skipcache, silent, debug;
} CFG;

class ProgramOptions {
  public:
    ProgramOptions();
    ProgramOptions(const ProgramOptions &) = delete;
    ProgramOptions &operator=(const ProgramOptions &) = delete;

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

    const CFG &getConfig()
    { return m_config; }

    const std::string &getLogFilePath()
    { return this->logfile_path; }

    /*
     * the file names for the two cache files, relativ to CFG.data_dir_path
     * (usually $HOME/.local/share/APPNAME on *iX).
     */
    static inline std::string const _version_number = "0.1.1";
    static inline std::string const _appname = "fetchweather";

    static constexpr std::array<const char*, 2> api_shortcodes = { "CC", "OWM" };
    static constexpr std::array<const char*, 2> api_readable_names = { "ClimaCell", "OpenWeatherMap" };
    enum { API_CLIMACELL, API_OMW, _API_END_ };

  private:
    CLI::App m_oCommand;
    std::string m_name;
    void _init();
    unsigned int counter;
    CFG m_config;
    std::string logfile_path;
};
#endif //OBJCTEST_OPTIONS_H
