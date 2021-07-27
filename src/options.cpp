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

#include "utils.h"

ProgramOptions::ProgramOptions() :
    m_oCommand(),
    m_config{
     .apiProvider = 0, .temp_unit = 'C', .temp_unit_raw = "C",
     .config_dir_path = "", .apikeyFile = "", .apikey = "", .apiProviderString = "",
     .vis_unit = "km", .speed_unit = "km/h", .pressure_unit = "hPa",
     .output_file = "", .location="", .timezone="Europe/Vienna",
     .offline = false, .nocache = false, .skipcache = false,
     .silent = false, .debug = false, .dumptofile = false,
     .forecastDays = 3
    },
    m_Parser{}
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
    m_oCommand.add_flag("--debug",
                        this->m_config.debug, "Dump config and perform a dry run showing what would be done.\n"
                                              "don't use in production as no real work will be done!");
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
    m_oCommand.add_option("--output,-o", this->m_config.output_file,
                          "Also write result to this file. Does not imply --silent.");

    m_oCommand.add_option("--tempUnit", this->m_config.temp_unit_raw,
                          "Unit to output the temperature: C or F, default is C.");
    m_oCommand.add_option("--speedUnit", this->m_config.speed_unit,
                          "Unit to output wind speed. Allowed are: m/s (default), kts, km/h or mph");
    m_oCommand.add_option("--visUnit", this->m_config.vis_unit,
                          "Unit to output visibility. Allowed are: km (default) or mi (Miles)");
    m_oCommand.add_option("--pressureUnit", this->m_config.pressure_unit,
                          "Unit to output pressure. Allowed are: hPa (default) or inhg");
    m_oCommand.add_option("--forecastDays,-d", this->m_config.forecastDays,
                          "Number of days to record daily forecasts. Defaults to 3\n"
                          "Maximum depends on the Weather API provider.");
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
    char msg[256];

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
        LOG_F(INFO, "ProgramOptions::parse(): create_directories result: %d : %s",
              ec.value(), ec.message().c_str());
        if (0 == ec.value()) {
            fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace);
            fs::permissions(path.parent_path(), fs::perms::owner_all, fs::perm_options::replace);
        }
    } else {
        LOG_F(INFO, "ProgramOptions::parse(): Could not create the data directories. "
                    "Maybe no permission or they exist?");
        LOG_F(INFO, "ProgramOptions::parse(): Attempted to create: %s", path.c_str());
        LOG_F(INFO, "ProgramOptions::parse(): Error code: %d : %s", ec.value(), ec.message().c_str());
    }

    if (bool res = fs::create_directories(fs::path(m_config.config_dir_path), ec)) {
        LOG_F(INFO, "ProgramOptions::parse(): Config directory %s created",
              m_config.config_dir_path.c_str());
    }
    /*
     * try to read API key from a file, when present.
     * usually $HOME/.config/fetchweather/XX.key where XX is the API provider
     * i.e. CC.key holds the key for ClimaCell.
     */
    this->keyfile_path.assign(m_config.config_dir_path);
    this->keyfile_path.append("/").append(m_config.apiProviderString).append(".key");
    LOG_F(INFO, "ProgramOptions::parse(): Attempting to read the API key from: %s",
          this->keyfile_path.c_str());

    std::ifstream f;

    f.open(this->keyfile_path);
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
                this->fUseKeyfile = true;
            } else {
                LOG_F(INFO, "Ignoring the key, because the command line option --apikey takes precedence");
            }
        }
        f.close();
    } else {
        LOG_F(INFO, "ProgramOptions::parse(): No keyfile found for API provider %s",
              m_config.apiProviderString.c_str());
    }

    /**
     * validators for units
     * we do not exit for wrong values, just set the default and
     * log the problem (or print to console in debug mode)
     */

    if(!m_config.temp_unit_raw.empty()) {
        m_config.temp_unit = static_cast<char>(toupper(m_config.temp_unit_raw[0]));
    }
    if(m_config.temp_unit != 'F' && m_config.temp_unit != 'C') {
        snprintf(msg, 255, "Unrecognized temperature Unit %c (allowed are C or F). Reverting default",
                 m_config.temp_unit);
        m_config.temp_unit = 'C';
        if(m_config.debug) {
            printf("%s\n", msg);
        } else {
            LOG_F(INFO, "%s", msg);
        }
    }

    if(m_config.speed_unit != "m/s" && m_config.speed_unit != "km/h" && m_config.speed_unit != "kts" && m_config.speed_unit != "mph") {
        snprintf(msg, 255, "Unrecognized speed unit %s (m/s, km/h, mph or knots). Reverting default.",
                 m_config.speed_unit.c_str());
        m_config.speed_unit.assign("km/h");
        if(m_config.debug) {
            printf("%s\n", msg);
        } else {
            LOG_F(INFO, "%s", msg);
        }
    }

    if(m_config.vis_unit != "km" && m_config.vis_unit != "mi") {
        snprintf(msg, 255, "Unrecognized visbility Unit %s (allowed are km or mi). Reverting default",
                 m_config.vis_unit.c_str());
        m_config.vis_unit.assign("km");
        if(m_config.debug) {
            printf("%s\n", msg);
        } else {
            LOG_F(INFO, "%s", msg);
        }
    }

    if(m_config.pressure_unit != "hPa" && m_config.pressure_unit != "inhg") {
        snprintf(msg, 255, "Unrecognized pressure Unit %s (allowed are hPa or inhg). Reverting default",
                 m_config.pressure_unit.c_str());
        m_config.pressure_unit.assign("hPa");
        if(m_config.debug) {
            printf("%s\n", msg);
        } else {
            LOG_F(INFO, "%s", msg);
        }
    }
    return this->m_oCommand.get_option("--version")->count() ? 1 : 2;
}

void ProgramOptions::print_version()
{
    std::cout << "This is fetchweather version " << ProgramOptions::_version_number << std::endl;
    std::cout << "(C) 2021 by Alex Vie <silvercircle at gmail dot com>" << std::endl << std::endl;
    std::cout << "This software is free software governed by the MIT License." << std::endl;
    std::cout << "Please visit https://github.com/silvercircle/fetchweather" << std::endl;
    std::cout << "for more information about this software and copyright information." << std::endl;
}

/**
 * this is for --debug. Dumps all options, performs a dry run.
 * does not fetch or modify any data.
 */
void ProgramOptions::dumpOptions()
{
    printf("*** DEBUG / DRY RUN output ***\n");
    printf("This is fetchweather version %s\n\n", ProgramOptions::_version_number.c_str());
    printf("* Configuration: *\n");
    printf("Configuration directory: %s\n", m_config.config_dir_path.c_str());
    printf("Data directory:          %s\n", m_config.data_dir_path.c_str());
    printf("Log file:                %s\n", this->logfile_path.c_str());
    printf("Selected API to use:     %d (%s)\n", m_config.apiProvider,
           ProgramOptions::api_readable_names[m_config.apiProvider]);
    printf("API Key:                 %s %s %s\n", m_config.apikey.c_str(),
           this->fUseKeyfile ? "From keyfile:" : "(from command line option)",
           this->fUseKeyfile ? this->keyfile_path.c_str() : "");
    if (m_config.location.empty()) {
        printf("Location:                %s, %s\n", m_config.lat.c_str(), m_config.lon.c_str());
    } else {
        printf("Location:                %s\n", m_config.location.c_str());
    }
    printf("Timezone:                %s\n", m_config.timezone.c_str());
    printf("Units (Temp, Windspeed, Vis, Pressure): %c, %s, %s, %s\n", m_config.temp_unit,
           m_config.speed_unit.c_str(), m_config.vis_unit.c_str(), m_config.pressure_unit.c_str());
}
