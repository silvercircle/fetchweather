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

#define __OPTIONS_INTERNAL

#include "utils.h"

ProgramOptions::ProgramOptions() : m_oCommand(),
                                   m_config{
                                     .apiProvider = 0, .temp_unit = 'C', .config_dir_path = "", .apikeyFile = "",
                                     .apikey = "", .apiProviderString = "",
                                     .vis_unit = "km", .speed_unit = "km/h", .pressure_unit = "hpa",
                                     .output_dir = "", .location="", .timezone="Europe/Vienna",
                                     .offline = false, .nocache = false, .skipcache = false,
                                     .silent = false, .debug = false
                                   }
{
    this->_init();
}

void ProgramOptions::_init()
{
    m_oCommand.add_flag("--version,-V", "Show program version and exit.");
    m_oCommand.add_flag("--offline",
                        this->m_config.offline, "No API request, used cached result only.");
    m_oCommand.add_flag("--nocache",
                        this->m_config.nocache, "Do not refresh the cache with the results.");
    m_oCommand.add_flag("--skipcache",
                        this->m_config.skipcache,
                        "Do not read from cached results, even when online request fails.\n"
                        "Note that --offline and --skipcache are mutually exclusive");
    m_oCommand.add_flag("--silent,-s",
                        this->m_config.silent, "Do not print anything to stdout. "
                                               "Makes only sense with --output.");
    m_oCommand.add_flag("--debug,-d",
                        this->m_config.debug, "Show various debugging output on the console "
                                              "and possibly in the\nlog files. Do not use in production!");
    m_oCommand.add_option("--apikey,-a", this->m_config.apikey, "Set the API key");
    m_oCommand.add_option("--provider,-p", this->m_config.apiProviderString,
                          "Set the API provider.\n"
                          "Allowed values: CC (ClimaCell), OWM (OpenWeatherMap) or VC (VisualCrossing)\n"
                          "Default is ClimaCell. This option is case-sensitive!");

    // TODO location
    /*
     * locationi can be either a registered location id or a lat,lon format
     * e.g. 48.1222795,16.3347827
     */
    m_oCommand.add_option("--loc,-l",
                          this->m_config.location,
                          "Set the location.\nThis is either a location id from your dashboard\n"
                          "or a LAT,LON location like 38.1222795,14.3347827.");
    m_oCommand.add_option("--lat",
                          this->m_config.lat,
                          "Set the latitude part of the location for API providers who need separate\n"
                          "latitude and longitude parameters. Format example: --lat=38.1222795");
    m_oCommand.add_option("--lon",
                          this->m_config.lon,
                          "Set the longitude part of the location for API providers who need separate\n"
                          "latitude and longitude parameters. Format example: --lon=16.1222795");
    m_oCommand.add_option("--tz", this->m_config.timezone, "Set the time zone, e.g. Europe/Berlin");
    m_oCommand.add_option("--ouput,-o", this->m_config.output_dir,
                          "Also write result to this file.");
}

/**
 * write options to configuration file
 * TODO // do we need a config file?
 */
void ProgramOptions::flush()
{
    std::string filename;
}
/**
 * parse command line and a config file (if it does exist)
 *
 * @param argc command line argument count (from main())
 * @param argv list of arguments
 *
 * @return  0 if --help was requested -> print help and exit
 *          1 if --version was requested -> print version and exit
 *          2 otherwise.
 */
int ProgramOptions::parse(int argc, char **argv)
{
    CLI11_PARSE(this->m_oCommand, argc, argv);

    const gchar *datadir = g_get_user_data_dir();
    const gchar *homedir = g_get_home_dir();
    const gchar *cfgdir = g_get_user_config_dir();

    if(this->m_config.apiProviderString.length() != 0) {
        bool found = false;
        unsigned pIndex = 0;
        for(const auto& it : ProgramOptions::api_shortcodes) {
            if(this->m_config.apiProviderString == it) {
                found = true;
                m_config.apiProvider = pIndex;
                break;
            }
            pIndex++;
        }
        if(!found) {
            this->m_config.apiProviderString.assign("CC");
            LOG_F(INFO, "ProgramOptions::parse(): found invalid API provider. Set to default (CC)");
            m_config.apiProvider = 0;
        }
    } else {
        m_config.apiProviderString.assign("CC");
        LOG_F(INFO, "ProgramOptions::parse(): API Provider set to default (CC)");
        m_config.apiProvider = 0;
    }

    LOG_F(INFO, "ProgramOptions::parse(): Final weather API provider set to: %s (%d)\n",
          m_config.apiProviderString.c_str(), m_config.apiProvider);

    std::string tmp(cfgdir);
    tmp.append("/config.toml");

    this->m_config.config_dir_path.assign(cfgdir);
    this->m_config.config_dir_path.append("/").append(ProgramOptions::_appname);
    this->m_config.config_file_path.assign(tmp);
    this->m_config.data_dir_path.assign(datadir);
    this->m_config.data_dir_path.append("/").append(ProgramOptions::_appname);

    this->logfile_path.assign(this->m_config.data_dir_path);
    this->logfile_path.append("/log.log");
    loguru::add_file(this->logfile_path.c_str(), loguru::Append, loguru::Verbosity_MAX);

    std::string p(m_config.data_dir_path);
    p.append("/cache");
    fs::path path(p);
    std::error_code ec;

    if (bool res = fs::create_directories(path, ec)) {
        LOG_F(INFO, "ProgramOptions::parse(): create_directories result: %d : %s", ec.value(), ec.message().c_str());
        if (0 == ec.value()) {
            fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace);
            fs::permissions(path.parent_path(), fs::perms::owner_all, fs::perm_options::replace);
        }
    } else {
        LOG_F(INFO, "ProgramOptions::parse(): Could not create the data directories. Maybe no permission or they exist?");
        LOG_F(INFO, "ProgramOptions::parse(): Attempted to create: %s", path.c_str());
        LOG_F(INFO, "ProgramOptions::parse(): Error code: %d : %s", ec.value(), ec.message().c_str());
    }

    if (bool res = fs::create_directories(fs::path(m_config.config_dir_path), ec)) {
        LOG_F(INFO, "ProgramOptions::parse(): Config directory %s created", m_config.config_dir_path.c_str());
    }
    /*
     * try to read API key from a file, when present.
     * usually $HOME/.config/fetchweather/XX.key where XX is the API provider
     * i.e. CC.key holds the key for ClimaCell.
     */
    std::string keyfile(m_config.config_dir_path);
    keyfile.append("/").append(m_config.apiProviderString).append(".key");
    LOG_F(INFO, "ProgramOptions::parse(): Attempting to read the API key from: %s", keyfile.c_str());

    std::ifstream f;

    f.open(keyfile);
    if(!f.fail()) {         // i dislike the overloaded ! for streams.
        std::string line;
        std::getline(f, line);
        /*
         * The first line is supposed to contain the api key, nothing else. Everyhing
         * else is ignored.
         */
        if(line.length() > 0) {
            utils::trim(line);
            LOG_F(INFO, "ProgramOptions::parse(): An API key for Provider %s has been found: %s",
                  m_config.apiProviderString.c_str(), line.c_str());
            if(this->m_oCommand.get_option("--apikey")->count() == 0) {
                LOG_F(INFO, "Setting this key as API key because none was supplied on the command line.");
                m_config.apikey.assign(line);
            } else {
                LOG_F(INFO, "Ignoring the key, because the command line option --apikey takes precedence");
            }
        }
        f.close();
    } else {
        LOG_F(INFO, "ProgramOptions::parse(): No keyfile found for API provider %s", m_config.apiProviderString.c_str());
    }
    return this->m_oCommand.get_option("--version")->count() ? 1 : 2;
}

void ProgramOptions::print_version()
{
    std::cout << "This is climacell_fetch version " << ProgramOptions::_version_number << std::endl;
    std::cout << "(C) 2021 by Alex Vie <silvercircle at gmail dot com>" << std::endl << std::endl;
    std::cout << "This software is free software governed by the MIT License." << std::endl;
    std::cout << "Please visit https://github.com/silvercircle/myconkysetup/tree/master/cpp" << std::endl;
    std::cout << "for more information about this software and copyright information." << std::endl;
}