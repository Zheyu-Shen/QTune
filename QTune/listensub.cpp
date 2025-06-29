#include "listensub.h"
#include "ui_listensub.h"
#include "listensubsub.h"
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QPushButton>
#include <QDebug>
#include <QMediaPlayer>
#include <QUrl>
#include <QProcess>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QIODevice>
#include <QFile>

ListenSub::ListenSub(QWidget *parent)
    : QWidget(parent), ui(new Ui::ListenSub)
{
    ui->setupUi(this);

    // 隐藏scrollArea的水平滚动条
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->listensubsub = new ListenSubSub;
    connect(this->listensubsub, &ListenSubSub::back, [=]()
            {
                this->listensubsub->hide();
                this->show(); });

    // 初始化播放器
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    // 连接播放器状态变化信号
    connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state)
            {
        if (state == QMediaPlayer::StoppedState) {
            ui->play->setChecked(false);
        } });

    // 设置返回按钮样式
    ui->BackListen->setStyleSheet("QPushButton {"
                                  "    border: none;"
                                  "    background-color: transparent;"         // 透明背景
                                  "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                  "}"
                                  "QPushButton:hover {" // 悬浮时保持透明
                                  "    background-color: transparent;"
                                  "}");

    showYuepu();
}

ListenSub::~ListenSub()
{
    player->stop();            // 停止播放
    player->setSource(QUrl()); // 移除媒体源
    qDebug() << "Player is empty." << player->source().isEmpty();
    delete ui;
}

void ListenSub::showYuepu()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepu(dir + "/training");
    switch (mode)
    {
    case 1:
        yuepu += "/one/tmp.png";
        break;
    case 2:
        yuepu += "/two/tmp.png";
        break;
    case 3:
        yuepu += "/three/tmp.png";
        break;
    case 4:
        yuepu += "/four/tmp.png";
        break;
    }
    qDebug() << "readdir:" << yuepu;

    // 加载图片
    QPixmap pixmap(yuepu);
    if (pixmap.isNull())
    {
        ui->yuepuLabel->setText("无法加载乐谱图片");
        return;
    }

    // 获取标签宽度
    int labelWidth = ui->yuepuLabel->width();

    // 计算等比例缩放后的高度
    int scaledHeight = (pixmap.height() * labelWidth) / pixmap.width();

    // 缩放图片到标签宽度，保持宽高比
    pixmap = pixmap.scaled(labelWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 设置图片
    ui->yuepuLabel->setPixmap(pixmap);
    ui->yuepuLabel->setScaledContents(false);
    ui->yuepuLabel->setFixedWidth(labelWidth);
    ui->yuepuLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 设置图片顶部左对齐
}

void ListenSub::setMode(int modenum)
{
    mode = modenum;
    // 更新图片数量
    /*QString dir = QCoreApplication::applicationDirPath();
    QString folderName = QString("yinzu%1").arg(modenum);
    QDir yuepuFolder(dir + "/" + folderName);
    QStringList filters;
    filters << "*.png";
    imageCount = yuepuFolder.entryInfoList(filters, QDir::Files).size();
    qDebug() << "乐谱图片总数:" << imageCount;
    qDebug() << "当前文件夹:" << folderName;*/

    showYuepu(); // 显示第一张乐谱
}

void ListenSub::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/ls2.png"));
}

void ListenSub::on_BackListen_clicked()
{
    player->stop();            // 停止播放
    player->setSource(QUrl()); // 移除媒体源
    emit this->back();
    currentYuepuIndex = 0;
}

void ListenSub::on_play_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString filePath = dir + "/training";
    switch (mode)
    {
    case 1:
        filePath += "/one/tmp.wav";
        break;
    case 2:
        filePath += "/two/tmp.wav";
        break;
    case 3:
        filePath += "/three/tmp.wav";
        break;
    case 4:
        filePath += "/four/tmp.wav";
        break;
    }
    qDebug() << "mode: " << mode;
    qDebug() << "open wav: " << filePath;

    // 如果当前没有设置源，设置源
    if (player->source().isEmpty())
    {
        QUrl url = QUrl::fromLocalFile(filePath);
        player->setSource(url);
        player->setAudioOutput(audioOutput);
    }

    // 根据当前播放状态切换播放/暂停
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->pause();
        ui->play->setChecked(false);
    }
    else
    {
        player->play();
        ui->play->setChecked(true);
    }
}

void ListenSub::on_download_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString filePath = dir + "/training";
    switch (mode)
    {
    case 1:
        filePath += "/one/tmp.wav";
        break;
    case 2:
        filePath += "/two/tmp.wav";
        break;
    case 3:
        filePath += "/three/tmp.wav";
        break;
    case 4:
        filePath += "/four/tmp.wav";
        break;
    }

    // 打开文件保存对话框
    QString defaultName = QString("乐谱%1.wav").arg(mode);
    QString savePath = QFileDialog::getSaveFileName(this, "保存音频文件", defaultName, "视频文件 (*.mp4)");

    if (savePath.isEmpty())
    {
        return; // 用户取消了保存
    }

    // 保存filepath的file到savepath
    QFile sourceFile(filePath);
    if (sourceFile.exists() && sourceFile.open(QIODevice::ReadOnly))
    {
        QFile destFile(savePath);
        if (destFile.open(QIODevice::WriteOnly))
        {
            destFile.write(sourceFile.readAll());
            destFile.close();
            sourceFile.close();
            QMessageBox::information(this, "成功", "音频已保存: " + savePath);
        }
        else
        {
            QMessageBox::warning(this, "失败", "无法写入目标文件");
        }
    }
    else
    {
        QMessageBox::warning(this, "失败", "源音频文件不存在或无法打开");
    }

    // // 检查FFmpeg是否可用
    // QProcess checkProcess;
    // checkProcess.start("ffmpeg", QStringList() << "-version");
    // if (!checkProcess.waitForFinished(3000))
    // {
    //     QMessageBox::critical(this, "错误", "FFmpeg未安装或未添加到环境变量");
    //     return;
    // }

    // // 构建FFmpeg命令
    // QStringList arguments;
    // arguments << "-y" // 覆盖已存在的文件
    //           << "-loop" << "1"
    //           << "-i" << pngPath
    //           << "-i" << mp3Path
    //           << "-c:v" << "libx264"
    //           << "-preset" << "medium"                                                                           // 使用中等编码速度
    //           << "-crf" << "23"                                                                                  // 控制视频质量
    //           << "-vf" << "scale=1280:720:force_original_aspect_ratio=decrease,pad=1280:720:(ow-iw)/2:(oh-ih)/2" // 调整视频尺寸
    //           << "-c:a" << "aac"
    //           << "-b:a" << "192k"
    //           << "-pix_fmt" << "yuv420p"
    //           << "-shortest"
    //           << mp4Path;

    // QProcess process;
    // process.start("ffmpeg", arguments);

    // if (!process.waitForFinished(-1))
    // {
    //     QMessageBox::critical(this, "错误", "视频生成超时");
    //     return;
    // }

    // if (process.exitCode() != 0)
    // {
    //     QString errorOutput = QString::fromUtf8(process.readAllStandardError());
    //     QMessageBox::critical(this, "错误", "视频生成失败:\n" + errorOutput);
    //     return;
    // }
}

void ListenSub::on_refresh_clicked()
{
    // 停止当前播放
    if (player->playbackState() == QMediaPlayer::PlayingState) // QMediaPlayer::PlayingState​​​​表示播放器当前正在播放音频/视频
    {
        player->stop();
    }

    showYuepu();
}

void ListenSub::on_test_clicked()
{
    // 停止当前播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
    this->listensubsub->setsubMode(mode);
    this->hide();
    this->listensubsub->show();
}
