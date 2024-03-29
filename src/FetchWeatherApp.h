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

#ifndef FETCHWEATHER_SRC_FETCHWEATHERAPP_H_
#define FETCHWEATHER_SRC_FETCHWEATHERAPP_H_

#include "pch.h"

Q_DECLARE_METATYPE(std::string)

class FetchWeatherApp : public QObject {
  Q_OBJECT

  public:
    FetchWeatherApp(QCoreApplication *parent = 0, int argc = 0, char **argv = nullptr)
      : QObject(parent)
    {
        qRegisterMetaType<std::string>();
        this->m_argc = argc;
        this->m_argv = argv;
        this->m_app = parent;
        QObject::connect(this, &FetchWeatherApp::testsignal, this, &FetchWeatherApp::testSlot, Qt::QueuedConnection);
        QObject::connect(this, &FetchWeatherApp::testsignal, this, &FetchWeatherApp::testSlot1);
    }

  public slots:
    void run();

  signals:
    void finished(int rc);
    void testsignal(QString* msg);

  private:
    int                 m_argc;
    char**              m_argv;
    QCoreApplication*   m_app;
    // normal methods can be slots in Qt 5
    void                testSlot(QString* msg);
    void                testSlot1(QString* msg);
};
#endif //FETCHWEATHER_SRC_FETCHWEATHERAPP_H_
