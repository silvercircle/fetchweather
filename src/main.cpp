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

int main(int argc, char **argv)
{
    bool extended_checks_failed = false;
    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::init(argc, argv);
    ProgramOptions &opt = ProgramOptions::getInstance();
    char    msg[256];

    auto result = opt.parse(argc, argv);
    LOG_F(INFO, "main(): The result from ProgramOptions::parse() was: %d", result);
    // catch the help
    if (0 == result)
        exit(result);

    if (1 == result) {
        // --version or -V parameter was given. Print version information and exit.
        opt.print_version();
        exit(0);
    }

    /* more sanity checks */

    const CFG& cfg = opt.getConfig();
    if(cfg.offline && cfg.skipcache) {
        /* ignoring both online mode and the cache does not make sense */
        printf("The options --offline and --skipcache are mutually exclusive\n"
               "and cannot be used together.");
        LOG_F(INFO, "main(): The options --offline and --skipcache cannot be used together");
        exit(-1);
    }
    if(cfg.silent && cfg.output_dir.length() == 0) {
        /* --silent without a filename for dumping the output does not make sense
         * either
         */
        printf("The option --silent requires a filename specified with --output.");
        LOG_F(INFO, "main(): --silent option was specified without using --output");
        exit(-1);
    }

    if(cfg.apikey.length() == 0) {
        LOG_F(INFO, "main(): Api KEY missing. Aborting.");
        extended_checks_failed = true;
        printf("\nThe API Key is missing. You must specify it with --apikey=your_key.\n");
    }

    if(cfg.location.length() == 0 && cfg.lat.length() == 0 && cfg.lon.length() == 0) {
        LOG_F(INFO, "main(): Location is missing. Aborting.");
        extended_checks_failed = true;
        printf("No location given. Option --loc=LOCATION is mandatory, where LOCATION\n"
               "is either in LAT,LON form or a location ID created on your ClimaCell dashboard.\n");

    }

    if(extended_checks_failed)
        exit(-1);

    if(cfg.debug) {
        opt.dumpOptions();
    }

    //ProgramOptions::api_shortcodes
    switch(cfg.apiProvider) {
        case ProgramOptions::API_CLIMACELL: {
            DataHandler_ImplClimaCell dh;
            dh.run();
            break;
        }
        case ProgramOptions::API_OMW: {
            DataHandler_ImplOWM dh;
            dh.run();
            break;
        }
        default:
            LOG_F(INFO, "No valid Provider selected. exiting.");
            break;
    }
}
