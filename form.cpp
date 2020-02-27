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
#include <QSettings>

#include <math.h>

Form::Form(QWidget *parent) : QWidget(parent), ui(new Ui::Form), currentIndex(0), limitNumber(9), m_interval(30) {
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);

    this->showMaximized();
    connect(&manager, SIGNAL(finished(QNetworkReply *)), SLOT(downloadFinished(QNetworkReply *)));

    changedTimer = new QTimer(this);
    connect(changedTimer, SIGNAL(timeout()), this, SLOT(slotUpdate()));

    loadInI();

    for (int i = 0; i < limitNumber * 2; ++i) {
        auto play = new PlayerWindow(this);
        play->hide();
        playList.append(play);
    }
    initPlayWidget();
    connect(ui->widgetTitle, &TitleWidget::signalMin, this, &Form::showMinimized);
    connect(ui->widgetTitle, &TitleWidget::signalClose, this, &Form::close);

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
    qDebug() << " getlistNumber  begin" << devSNList.count() << QTime::currentTime().toString("hh:mm:ss.zzz");

    // ui->widgetTitle->setTitleInfo("SimplePlayer");
    slotUpdate();
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

    qDebug() << "Form::slotChanged stackedWidget  end ********" << ui->stackedWidget->currentIndex()
             << QTime::currentTime().toString("hh:mm:ss.zzz");
    if (!changedTimer->isActive()) {
        changedTimer->start(1000 * m_interval); // 30S
    }

    int index = ui->stackedWidget->currentIndex();
    ui->stackedWidget->setCurrentIndex(index > 0 ? 0 : 1);
}

void Form::loadInI() {
    QSettings settings("Setting.ini", QSettings::IniFormat);
    QString title = settings.value("Title").toString();
    m_interval = settings.value("Interval").toInt();
    limitNumber = settings.value("Number").toInt();
    ui->widgetTitle->setTitleInfo(title);
    setWindowTitle(title);

    qDebug() << "Form::loadInI " << title << m_interval << limitNumber;
}

void Form::saveInI() const {

    int number = 9;
    QSettings settings("Setting.ini", QSettings::IniFormat);
    settings.setValue("Number", number);
}

void Form::initPlayWidget() {
    int count = 0;
    int row = 0;                  // 行列
    int column = 0;               // 列
    int index = 0;                // 起始位置
    int iRow = sqrt(limitNumber); // 每行最大窗口数量

    for (int i = 0; i < limitNumber; ++i) {
        if (i >= index) {
            ui->gridLayoutOne->addWidget(playList.at(i), row, column);
            ui->gridLayoutTwo->addWidget(playList.at(i + limitNumber), row, column);
            playList.at(i)->setVisible(true);
            playList.at(i + limitNumber)->setVisible(true);
            count++;
            column++;
            //  换行
            if (column == iRow) {
                row++;
                column = 0;
            }
        }

        if (count == (iRow * iRow)) {
            break;
        }
    }
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
}

void Form::mousePressEvent(QMouseEvent *event) {
    //只能是鼠标左键移动和改变大小
    if (event->button() == Qt::LeftButton) {
        mouse_press = true;
    }

    //窗口移动距离
    move_point = event->globalPos() - pos();
}

void Form::mouseReleaseEvent(QMouseEvent *event) {
    mouse_press = false;
}

void Form::mouseMoveEvent(QMouseEvent *event) {
    //移动窗口
    if (mouse_press) {
        QPoint move_pos = event->globalPos();
        move(move_pos - move_point);
    }
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
                // QCoreApplication::processEvents();
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

            // QCoreApplication::processEvents();
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
    auto list = playList.mid((index > 0 ? 0 : limitNumber), limitNumber);
    if (devUrlMessageHash.count() <= list.count()) {
        int i = 0;
        foreach (auto &url, devUrlMessageHash.values()) {
            PlayerWindow *play = qobject_cast<PlayerWindow *>(list.at(i));
            if (play != nullptr) {
                QString mac = devUrlMessageHash.keys().at(i);
                play->showDevSn(mac);
                play->playUrl(url.rtmpUrl);
                // qDebug() << " Form::slotPlay mac  " << mac << "url.rtmpUrl" << url.rtmpUrl << QTime::currentTime().toString("hh:mm:ss.zzz");
            }
            ++i;
        }
    }

    QTimer::singleShot(8000, this, SLOT(slotChanged()));
    qDebug() << " Form::slotPlay" << devUrlMessageHash.count() << QTime::currentTime().toString("hh:mm:ss.zzz");
    // ui->widget->playUrl(subObj.value("rtmpUrl").toString());
}
