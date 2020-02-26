/******************************************************************************
    Simple Player:  this file is part of QtAV examples
    Copyright (C) 2012-2015 Wang Bin <wbsecg1@gmail.com>

*   This file is part of QtAV

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/
#include "customwall.h"
#include "form.h"
#include "playerwindow.h"
#include <QApplication>
#include <QDir>
#include <QMutex>
#include <QtAV>
#include <QtAVWidgets>
#include <QtCore/QFile>

Q_GLOBAL_STATIC(QFile, fileLogger)

QMutex loggerMutex;

void Logger(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    // QFile is not thread-safe
    QMutexLocker locker(&loggerMutex);

    QString text;
    switch (type) {
        case QtDebugMsg:
            text = QString("Debug:");
            break;
        case QtWarningMsg:
            text = QString("Warning:");
            break;
        case QtCriticalMsg:
            text = QString("Critical:");
            break;
        case QtFatalMsg:
            text = QString("Fatal:");
    }

    QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);

    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
    QString message = QString("%1 %2 %3 ").arg(text).arg(current_date_time).arg(QString(msg.toUtf8()));

    fileLogger()->open(QIODevice::WriteOnly | QIODevice::Append);

    QTextStream text_stream(fileLogger);
    text_stream << message << "\r\n";
    fileLogger()->flush();
    fileLogger()->close();
}

int main(int argc, char *argv[]) {

    QtAV::Widgets::registerRenderers();
    QtAV::setFFmpegLogLevel("quiet");
    QtAV::setLogLevel(QtAV::LogOff);

    QApplication::setAttribute(Qt::AA_UseOpenGLES);
    QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication a(argc, argv);

    QString logfile(QString::fromLatin1("%1/log.txt").arg(qApp->applicationDirPath()));
    if (!logfile.isEmpty()) {
        fileLogger()->setFileName(logfile);
        if (fileLogger()->open(QIODevice::WriteOnly | QIODevice::Append)) {
            qInstallMessageHandler(Logger);
            fileLogger()->close();
        } else {
            qWarning() << "Failed to open log file '" << fileLogger()->fileName() << "': " << fileLogger()->errorString();
        }
    }
    Form custom;
    custom.show();
    return a.exec();
}
