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

void FetchWeatherApp::run()
{
    bool    extended_checks_failed = false;
    char    msg[256];
    int     runresult = 0;

    loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
    loguru::init(this->m_argc, this->m_argv);
    ProgramOptions &opt = ProgramOptions::getInstance();

    const CFG& cfg = opt.getConfig();

    auto result = opt.parse(this->m_argc, this->m_argv);
    LOG_F(INFO, "main(): The result from ProgramOptions::parse() was: %d", result);
    // catch the help
    if (0 == result) {
        this->m_app->exit(result);
    }

    if (1 == result) {
        // --version or -V parameter was given. Print version information and exit.
        opt.print_version();
        this->m_app->exit(0);
    }

    if(cfg.debug) {
        opt.dumpOptions();
    }

    /* more sanity checks */

    if(cfg.offline && cfg.skipcache) {
        /* ignoring both online mode and the cache does not make sense */
        printf("The options --offline and --skipcache are mutually exclusive\n"
               "and cannot be used together.");
        LOG_F(INFO, "main(): The options --offline and --skipcache cannot be used together");
        this->m_app->exit(-1);
    }
    if(cfg.silent && cfg.output_file.length() == 0) {
        /* --silent without a filename for dumping the output does not make sense
         * either
         */
        printf("The option --silent requires a filename specified with --output.");
        LOG_F(INFO, "main(): --silent option was specified without using --output");
        this->m_app->exit(-1);
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

    if(extended_checks_failed) {
        this->m_app->exit(-1);
    }

    switch(cfg.apiProvider) {
        case ProgramOptions::API_CLIMACELL: {
            DataHandler_ImplClimaCell dh;
            runresult = dh.run();
            break;
        }
        case ProgramOptions::API_OWM: {
            DataHandler_ImplOWM dh;
            runresult = dh.run();
            break;
        }
        default:
            LOG_F(INFO, "No valid Provider selected. exiting.");
            runresult = -1;
            break;
    }
    //QString foo("Affen");
    //emit testsignal(&foo);

    this->m_app->exit(cfg.debug ? -1 : runresult);
}

void FetchWeatherApp::testSlot(QString* msg)
{
    qDebug() << "The message in TestSlot is: " << *msg;
}

void FetchWeatherApp::testSlot1(QString* msg)
{
    qDebug() << "The message in TestSlot1 is: " << *msg;
}
