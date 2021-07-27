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

FileDumper::FileDumper(DataHandler* h) :
    m_dataPoint(h->getDataPoint()),
    m_Handler(h),
    m_Options(ProgramOptions::getInstance())
{ }

void FileDumper::dump()
{
    const CFG& cfg = m_Options.getConfig();
    bool  fPathValid = true;
    fs::path filename;
    fs::path outfile(cfg.output_file);
    if(outfile.is_absolute()) {
        LOG_F(INFO, "DataHandler::run(): The output file path is an absolute path."
                    " This is not allowed.");
        fPathValid = false;
    } else {
        filename.assign(cfg.data_dir_path);
        filename.append(cfg.output_file);
    }
    if(fs::is_directory(filename)) {
        LOG_F(INFO, "DataHandler::run(): The output file path is an existing directory."
                    " This is not allowed.");
        fPathValid = false;
    } else if (fPathValid){
        FILE *f = fopen(filename.c_str(), "w");
        if(NULL != f) {
            m_Handler->doOutput(f);
            fclose(f);
            LOG_F(INFO, "DataHandler::run(): Dumping to: %s,", filename.c_str());
        } else {
            LOG_F(INFO, "DataHandler::run(): Unable to open the specified dump file (%s)."
                        " No data was written.",
                  filename.c_str());
        }
    } else {
        LOG_F(INFO, "DataHandler::run(): The output file was not properly specified."
                    " No data dump was written.");
    }
}