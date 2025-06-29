#include "sing.h"
#include "ui_sing.h"
#include "generatescore.h"
#include "drawscore.h"
#include "drawscore.cpp"
#include "playscore.h"

Sing::Sing(QWidget *parent)
    : QWidget(parent), ui(new Ui::Sing)
{
    ui->setupUi(this);
    this->singsub = new SingSub;

    connect(this->singsub, &SingSub::back, [=]()
            {
                this->singsub->hide();
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

Sing::~Sing()
{
    delete ui;
}

void Sing::on_BackHome_clicked()
{
    emit this->back();
}

void Sing::on_easybtn_clicked()
{
    Score score;
    ChordGenerator generator;
    score = generator.generateSightSinging1();
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/sing_file");
    drawScore(score, yuepuFolder);
    scoreToWAV(score, yuepuFolder);
    this->singsub->setDifficulty(1, score); // 设置简单难度
    this->hide();
    this->singsub->show();
    // qDebug() << "dir:" << dir;
}

void Sing::on_mediumbtn_clicked()
{
    Score score;
    ChordGenerator generator;
    score = generator.generateSightSinging2();
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/sing_file");
    drawScore(score, yuepuFolder);
    scoreToWAV(score, yuepuFolder);
    this->singsub->setDifficulty(2, score); // 设置中等难度
    this->hide();
    this->singsub->show();
}

void Sing::on_hardbtn_clicked()
{
    Score score;
    ChordGenerator generator;
    score = generator.generateSightSinging3();
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/sing_file");
    drawScore(score, yuepuFolder);
    scoreToWAV(score, yuepuFolder);
    this->singsub->setDifficulty(3, score); // 设置困难难度
    this->hide();
    this->singsub->show();
}

void Sing::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/s1.png"));
}
