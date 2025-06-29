#include "homepage.h"
#include "ui_homepage.h"
#include "sing.h"
#include "listen.h"
#include "write.h"
#include "log.h"
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QCoreApplication>
#include <QDebug>

Homepage::Homepage(QWidget *parent)
    : QWidget(parent), ui(new Ui::Homepage)
{
    ui->setupUi(this);
    this->sing = new Sing;
    this->listen = new Listen;
    this->write = new Write;
    this->log = new Log;
    this->aiAssistant = new AIAssistant;

    // 设置按钮背景色
    QString buttonStyle = "QPushButton {"
                          "    border: none;"
                          "    border-radius: 15px;"                  // 圆角
                          "    background-color: rgb(217, 217, 217);" // 原来的浅蓝色
                          "    padding: 10px;"                        // 内边距
                          "}"
                          "QPushButton:hover {"
                          "    background-color: rgb(197, 197, 197);" // 原来的悬浮色
                          "}"
                          "QPushButton:pressed {"
                          "    background-color: rgb(177, 177, 177);" // 原来的点击色
                          "    padding: 12px;"                        // 点击时内边距变大，产生按下效果
                          "}";

    // 为每个按钮设置样式
    ui->OpenSing->setStyleSheet(buttonStyle);
    ui->OpenListen->setStyleSheet(buttonStyle);
    ui->OpenWrite->setStyleSheet(buttonStyle);
    ui->OpenLog->setStyleSheet(buttonStyle);

    // 创建并设置图片标签
    QLabel *singLabel = new QLabel(ui->OpenSing);
    QLabel *listenLabel = new QLabel(ui->OpenListen);
    QLabel *writeLabel = new QLabel(ui->OpenWrite);
    QLabel *logLabel = new QLabel(ui->OpenLog);

    // 设置标签大小和位置
    singLabel->setFixedSize(70, 70);
    listenLabel->setFixedSize(70, 70);
    writeLabel->setFixedSize(70, 70);
    logLabel->setFixedSize(70, 70);

    // 设置图片
    singLabel->setPixmap(QPixmap(":/MyPicture/1.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    listenLabel->setPixmap(QPixmap(":/MyPicture/5.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    writeLabel->setPixmap(QPixmap(":/MyPicture/3.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logLabel->setPixmap(QPixmap(":/MyPicture/4.png").scaled(70, 70, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 设置标签居中
    singLabel->setAlignment(Qt::AlignCenter);
    listenLabel->setAlignment(Qt::AlignCenter);
    writeLabel->setAlignment(Qt::AlignCenter);
    logLabel->setAlignment(Qt::AlignCenter);

    // 设置标签位置
    singLabel->move((ui->OpenSing->width() - singLabel->width()) / 2, (ui->OpenSing->height() - singLabel->height()) / 2);
    listenLabel->move((ui->OpenListen->width() - listenLabel->width()) / 2, (ui->OpenListen->height() - listenLabel->height()) / 2);
    writeLabel->move((ui->OpenWrite->width() - writeLabel->width()) / 2, (ui->OpenWrite->height() - writeLabel->height()) / 2);
    logLabel->move((ui->OpenLog->width() - logLabel->width()) / 2, (ui->OpenLog->height() - logLabel->height()) / 2);

    connect(this->sing, &Sing::back, [=]()
            {
                this->sing->hide();
                this->show(); });
    connect(this->listen, &Listen::back, [=]()
            {
                this->listen->hide();
                this->show(); });
    connect(this->write, &Write::back, [=]()
            {
                this->write->hide();
                this->show(); });
    connect(this->log, &Log::back, [=]()
            {
                this->log->hide();
                this->show(); });
    connect(this->aiAssistant, &AIAssistant::back, [=]()
            {
                this->aiAssistant->hide();
                this->show(); });
    updateConsecutiveDays();
}

Homepage::~Homepage()
{
    delete ui;
}

void Homepage::on_OpenSing_clicked()
{
    this->hide();
    this->sing->show();
}

void Homepage::on_OpenListen_clicked()
{
    this->hide();
    this->listen->show();
}

void Homepage::on_OpenWrite_clicked()
{
    this->hide();
    this->write->show();
}

void Homepage::on_OpenLog_clicked()
{
    this->hide();
    this->log->show();
}

void Homepage::on_OpenAIAssistant_clicked()
{
    this->hide();
    this->aiAssistant->show();
}

void Homepage::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/hp1.png"));
}

void Homepage::updateConsecutiveDays()
{
    // 1. 获取文件路径：应用程序可执行文件所在的目录 + "date.txt"
    QString filePath = QCoreApplication::applicationDirPath() + "/log_file/date.txt";
    qDebug() << "Data file path:" << filePath; // 打印路径方便调试

    QFile file(filePath);

    // 2. 读取旧的记录
    QDate lastUsageDate;
    int consecutiveDays = 0;

    // 尝试打开文件进行读取
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString dateString = in.readLine();
        QString daysString = in.readLine();
        file.close();

        // 将读取的字符串转换为日期和整数
        lastUsageDate = QDate::fromString(dateString, Qt::ISODate); // "yyyy-MM-dd"
        consecutiveDays = daysString.toInt();
    }

    // 3. 核心逻辑判断
    QDate today = QDate::currentDate();
    int newConsecutiveDays;

    if (!lastUsageDate.isValid())
    {
        // 情况 A: 首次使用或文件不存在/损坏
        qDebug() << "First time usage or invalid data file.";
        newConsecutiveDays = 1;
    }
    else if (lastUsageDate == today)
    {
        // 情况 B: 当天重复使用，天数不变
        qDebug() << "Already used today. Streak remains:" << consecutiveDays;
        newConsecutiveDays = consecutiveDays;
    }
    else if (lastUsageDate.daysTo(today) == 1)
    {
        // 情况 C: 连续使用（昨天是上一次使用日期）
        qDebug() << "Consecutive day! Streak extended.";
        newConsecutiveDays = consecutiveDays + 1;
    }
    else
    {
        // 情况 D: 中断后使用，天数重置为1
        qDebug() << "Streak broken. Resetting to 1.";
        newConsecutiveDays = 1;
    }

    // 4. 将新记录覆盖写入文件
    // (优化：仅当数据有变化时才写入，避免不必要的磁盘操作)
    if (newConsecutiveDays != consecutiveDays || lastUsageDate != today)
    {
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            QTextStream out(&file);
            out << today.toString(Qt::ISODate) << "\n"; // 写入新日期
            out << newConsecutiveDays;                  // 写入新天数
            file.close();
            qDebug() << "Data file updated. New streak:" << newConsecutiveDays;
        }
        else
        {
            qWarning() << "Error: Could not write to file" << filePath;
        }
    }
}
