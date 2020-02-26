#include "form.h"
#include "playerwindow.h"
#include "ui_form.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDesktopWidget>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>

Form::Form(QWidget *parent) : QWidget(parent), ui(new Ui::Form), currentIndex(0), limitNumber(9) {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    this->showMaximized();
    connect(&manager, SIGNAL(finished(QNetworkReply *)), SLOT(downloadFinished(QNetworkReply *)));

    changedTimer = new QTimer(this);
    connect(changedTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

    widegetList = ui->stackedWidget->widget(0)->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    widegetTwoList = ui->stackedWidget->widget(1)->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);

    connect(ui->widgetTitle, &TitleWidget::signalMin, this, &Form::showMinimized);
    connect(ui->widgetTitle, &TitleWidget::signalClose, this, &Form::close);
    setWindowTitle("SimplePlayer");
    QTimer::singleShot(300, this, SLOT(getDevSNlist()));
    // 1.
    // getDevSNlist();
}

Form::~Form() {
    delete ui;
}

void Form::getDevSNlist() {

    QFile file(QString::fromLatin1("%1/DevSNList.txt").arg(qApp->applicationDirPath()));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Form::getDevSNlist getFile fail " << QTime::currentTime().toString("hh:mm:ss.zzz");
        return;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if (!line.isEmpty()) {
            devSNList.append(line.trimmed());
        }
    }

    file.close();
    qDebug() << " getlistNumber" << devSNList.count();

    ui->widgetTitle->setTitleInfo("SimplePlayer");
    slotUpdate();
    changedTimer->start(1000 * 30); // 30S
}

void Form::getDevInfo(int currentIndex, int limitNumber) {
    QString Invoker = "DaPing";
    QString InvokerPublicKey = "#mclzDP20200213#";
    qint64 UTCTimestamp = QDateTime::currentDateTimeUtc().currentMSecsSinceEpoch();
    // QString  test   =  QString("Invoker=value&UTCTimestamp=value &InvokerPublicKey=*******");
    QString macs = devSNList.mid(currentIndex, limitNumber).join(","); // devSNList.join(",");
    QString tmpParms =
    tr("deviceMacs=%1&Invoker=%2&UTCTimestamp=%3&InvokerPublicKey=%4").arg(macs).arg(Invoker).arg(UTCTimestamp).arg(InvokerPublicKey);
    QString AuthToken = QCryptographicHash::hash(tmpParms.toLocal8Bit(), QCryptographicHash::Md5).toHex();
    QString tmpurl = tr("http://ipc-balance.stavix.cn:8088/epgserver/rtsp/getDevicesStatusBySNs?"
                        "deviceMacs=%1"
                        "&Invoker=%2"
                        "&UTCTimestamp=%3"
                        "&AuthToken=%4")
                     .arg(macs)
                     .arg(Invoker)
                     .arg(UTCTimestamp)
                     .arg(AuthToken);

    QNetworkRequest request;
    request.setUrl(QUrl(tmpurl));
    manager.get(request);
    qDebug() << " Form::getDevInfo tmpParms" << tmpParms << "AuthToken " << AuthToken << "url" << tmpurl
             << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void Form::getRtmpUrl() {
    QString Invoker = "DaPing";
    QString InvokerPublicKey = "#mclzDP20200213#";
    qDebug() << " Form::getRtmpUrl " << devStatusHash.count() << QTime::currentTime().toString("hh:mm:ss.zzz");
    foreach (auto var, devStatusHash.keys()) {
        qint64 UTCTimestamp = QDateTime::currentDateTimeUtc().currentMSecsSinceEpoch();
        QString macs = var; // devSNList.join(",");
        QString tmpParms =
        tr("sn=%1&Invoker=%2&UTCTimestamp=%3&InvokerPublicKey=%4").arg(macs).arg(Invoker).arg(UTCTimestamp).arg(InvokerPublicKey);
        QString AuthToken = QCryptographicHash::hash(tmpParms.toLocal8Bit(), QCryptographicHash::Md5).toHex();
        QString tmpurl = tr("http://ipc-balance.stavix.cn:8088/epgserver/rtsp/getRTMP?"
                            "sn=%1"
                            "&Invoker=%2"
                            "&UTCTimestamp=%3"
                            "&AuthToken=%4")
                         .arg(macs)
                         .arg(Invoker)
                         .arg(UTCTimestamp)
                         .arg(AuthToken);
        // qDebug() << " Form::getRtmpUrl tmpParms" << tmpParms << "AuthToken " << AuthToken << "url" << tmpurl
        //         << QTime::currentTime().toString("hh:mm:ss.zzz");

        QNetworkRequest request;
        request.setUrl(QUrl(tmpurl));
        manager.get(request);
        QCoreApplication::processEvents();
    }
    // currentIndex += devStatusHash.count();
    qDebug() << " Form::getRtmpUrl " << QTime::currentTime().toString("hh:mm:ss.zzz") << "currentIndex " << currentIndex
             << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void Form::slotGetDevRet() {
    if (devStatusHash.count() != limitNumber) {
        qDebug() << "Form::slotGetDevRet  timeout" << QTime::currentTime().toString("hh:mm:ss.zzz") << "currentIndex " << currentIndex;
    }
}

void Form::slotGetRtmpUrRetl() {
    if (devUrlMessageHash.count() != limitNumber) {
        qDebug() << "Form::slotGetRtmpUrRetl  timeout" << QTime::currentTime().toString("hh:mm:ss.zzz") << "currentIndex " << currentIndex;
    }
}

void Form::slotChanged() {
    int index = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(index > 0 ? 0 : 1);
    qDebug() << "Form::slotChanged stackedWidget  end ********" << ui->stackedWidget->currentIndex()
             << QTime::currentTime().toString("hh:mm:ss.zzz");
}

void Form::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(58, 62, 67));
    painter.drawRect(QRect(0, 0, this->width(), this->height()));
}

void Form::resizeEvent(QResizeEvent *e) {
    Q_UNUSED(e);

    QWidget::resizeEvent(e);
    /*
    for(int i=0;  i<widegetList.size(); ++i )
    {
        VlcPlayerWidget  *play = qobject_cast< VlcPlayerWidget *>(widegetList.at(i));
        if(play !=nullptr)
        {
            play->updateRatio();
        }
    }*/
}

void Form::downloadFinished(QNetworkReply *reply) {
    // 获取响应状态码，200表示正常
    QVariant nCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    // qDebug() << " Form::downloadFinished  nCode " << nCode.toString() << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray bytes = reply->readAll();
        // qDebug()<<" Form::downloadFinished ok"<< bytes;
        QJsonParseError json_error;
        QJsonDocument jsonDoc(QJsonDocument::fromJson(bytes, &json_error));
        if (json_error.error != QJsonParseError::NoError) {
            qDebug() << "json error!" << QTime::currentTime().toString("hh:mm:ss.zzz");
            return;
        }
        QJsonObject rootObj = jsonDoc.object();
        //  QStringList keys = rootObj.keys();

        if (rootObj.contains("result")) {
            const int result = 1; //
            int ret = rootObj.value("result").toInt();
            // qDebug() << "ret  is:" << ret ;
            if (ret != result) {
                qDebug() << "ret error!" << QTime::currentTime().toString("hh:mm:ss.zzz");
                return;
            }
        }
        if (rootObj.contains("data")) {
            QJsonArray subArray = rootObj.value("data").toArray();
            const int tmpsize = subArray.size();
            currentIndex += tmpsize;
            for (int i = 0; i < tmpsize; i++) {
                auto subObj = subArray.at(i).toObject();
                devstatus tmpStatus;
                tmpStatus.deviceMac = subObj.value("deviceMac").toString();
                tmpStatus.connectionStatus = subObj.value("connectionStatus").toInt() > 0;
                tmpStatus.streamStatus = subObj.value("streamStatus").toInt() > 0;
                if (tmpStatus.connectionStatus && tmpStatus.streamStatus) {
                    devStatusHash.insert(tmpStatus.deviceMac, tmpStatus);
                }
                // qDebug() << " deviceMac" << tmpStatus.deviceMac << " connectionStatus" << tmpStatus.connectionStatus
                //          << " streamStatus" << tmpStatus.streamStatus;
                QCoreApplication::processEvents();
            }
            // qDebug() << "Form::downloadFinished  getdata" << QTime::currentTime().toString("hh:mm:ss.zzz");

            // step 2

            if (devStatusHash.count() < limitNumber) {
                getDevInfo(currentIndex, limitNumber - devStatusHash.count());
            } else {
                getRtmpUrl();
            }
        }

        if (rootObj.contains("urlMessage")) {
            auto subObj = rootObj.value("urlMessage").toObject();
            urlMessage url;
            url.result = subObj.value("result").toInt() > 0;
            url.m3u8Url = subObj.value("m3u8Url").toString();
            url.rtmpUrl = subObj.value("rtmpUrl").toString();
            QString mac = url.rtmpUrl.split("/").last();
            if (url.result) {
                devUrlMessageHash.insert(mac, url);
            }

            QCoreApplication::processEvents();
            // qDebug() << "mac   is:" << mac << "rtmpUrl" << url.rtmpUrl;
            // qDebug() << "Form::downloadFinished  geturlMessage" << QTime::currentTime().toString("hh:mm:ss.zzz");
            slotPlay();
        }
    } else {
        qDebug() << " Form::downloadFinished fail " << reply->error() << QTime::currentTime().toString("hh:mm:ss.zzz");
    }
}

void Form::slotUpdate() {
    qDebug() << "Form::slotUpdate  beging ********" << QTime::currentTime().toString("hh:mm:ss.zzz");
    // 先清除 已有的信息
    if (devSNList.count() <= currentIndex) {
        currentIndex = 0;
    }
    devStatusHash.clear();
    devUrlMessageHash.clear();
    getDevInfo(currentIndex);
    QTimer::singleShot(2000, this, SLOT(slotGetDevRet()));
    QTimer::singleShot(5000, this, SLOT(slotGetRtmpUrRetl()));
}

void Form::slotPlay() {

    if (devUrlMessageHash.count() < limitNumber) {
        return;
    }

    int index = ui->stackedWidget->currentIndex();

    auto &list = index > 0 ? widegetList : widegetTwoList;
    if (devUrlMessageHash.count() <= widegetList.count()) {
        int i = 0;
        foreach (auto &url, devUrlMessageHash.values()) {

            // FormPlay  *play = qobject_cast< FormPlay *>(widegetList.at(i));
            // VlcPlayerWidget *play = qobject_cast<VlcPlayerWidget *>(list.at(i));
            PlayerWindow *play = qobject_cast<PlayerWindow *>(list.at(i));
            if (play != nullptr) {
                QString mac = devUrlMessageHash.keys().at(i);
                // play->showTitle(mac);
                // play->updateRatio();
                // play->playUrlFile(url.rtmpUrl);
                play->showDevSn(mac);
                play->playUrl(url.rtmpUrl);
                // qDebug() << " Form::slotPlay mac  " << mac << "url.rtmpUrl" << url.rtmpUrl << QTime::currentTime().toString("hh:mm:ss.zzz");
            }
            ++i;
        }
    }

    QTimer::singleShot(5000, this, SLOT(slotChanged()));
    qDebug() << " Form::slotPlay" << devUrlMessageHash.count() << QTime::currentTime().toString("hh:mm:ss.zzz");
    // ui->widget->playUrl(subObj.value("rtmpUrl").toString());
}
