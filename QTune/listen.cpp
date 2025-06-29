#include "listen.h"
#include "ui_listen.h"
#include "listensub.h"
#include "generatescore.h"
#include "drawscore.h"
#include "playscore.h"

Listen::Listen(QWidget *parent)
    : QWidget(parent), ui(new Ui::Listen)
{
    ui->setupUi(this);
    this->listensub = new ListenSub;

    connect(this->listensub, &ListenSub::back, [=]()
            {
                this->listensub->hide();
                this->show(); });

    // 设置返回按钮样式
    ui->BackHome->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: transparent;"         // 透明背景
                                "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                "}"
                                "QPushButton:hover {" // 悬浮时保持透明
                                "    background-color: transparent;"
                                "}");
}

Listen::~Listen()
{
    delete ui;
}

void Listen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/l1.png"));
}

void Listen::on_BackHome_clicked()
{
    emit this->back();
}

void Listen::on_danyin_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/training/one");
    QString pngfile = (yuepuFolder + "/tmp.png");
    if(!QFile::exists(pngfile)){
        Score score; ChordGenerator generator;
        score = generator.generateTraining(ONE_NOTE);
        drawScore(score, yuepuFolder);
        scoreToWAV(score, yuepuFolder);
    }
    this->listensub->setMode(1);
    this->hide();
    this->listensub->show();
}

void Listen::on_shuangyin_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/training/two");
    QString pngfile = (yuepuFolder + "/tmp.png");
    if(!QFile::exists(pngfile)){
        Score score; ChordGenerator generator;
        score = generator.generateTraining(TWO_NOTES);
        drawScore(score, yuepuFolder);
        scoreToWAV(score, yuepuFolder);
    }
    this->listensub->setMode(2);
    this->hide();
    this->listensub->show();
}

void Listen::on_sanyin_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/training/three");
    QString pngfile = (yuepuFolder + "/tmp.png");
    if(!QFile::exists(pngfile)){
        Score score; ChordGenerator generator;
        score = generator.generateTraining(THREE_NOTES);
        drawScore(score, yuepuFolder);
        scoreToWAV(score, yuepuFolder);
    }
    this->listensub->setMode(3);
    this->hide();
    this->listensub->show();
}

void Listen::on_qiyin_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/training/four");
    QString pngfile = (yuepuFolder + "/tmp.png");
    if(!QFile::exists(pngfile)){
        Score score; ChordGenerator generator;
        score = generator.generateTraining(FOUR_NOTES);
        drawScore(score, yuepuFolder);
        scoreToWAV(score, yuepuFolder);
    }
    this->listensub->setMode(4);
    this->hide();
    this->listensub->show();
}
