#include "listensubsub.h"
#include "ui_listensubsub.h"
#include <QCoreApplication>
#include <QDebug>
#include <QMessageBox>
#include "generatescore.h"
#include "playscore.h"

ListenSubSub::ListenSubSub(QWidget *parent)
    : QWidget(parent), ui(new Ui::ListenSubSub)
{
    ui->setupUi(this);

    // 初始化播放器
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    currentMp3Index = -1;
    mp3Count = 0;

    // 安装事件过滤器
    ui->you->installEventFilter(this);

    // 连接错误信号
    connect(player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString)
            {
        qDebug() << "播放错误:" << error << errorString;
        QMessageBox::critical(this, "播放错误", "无法播放MP3文件: " + errorString); });

    // 连接播放器状态变化信号
    connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state)
            {
        if (state == QMediaPlayer::StoppedState) {
            ui->play->setChecked(false);
            ui->play->setIcon(QIcon(":/MyPicture/p.png"));
        } });

    // 设置返回按钮样式
    ui->BackListenSub->setStyleSheet("QPushButton {"
                                     "    border: none;"
                                     "    background-color: transparent;"         // 透明背景
                                     "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                     "}"
                                     "QPushButton:hover {" // 悬浮时保持透明
                                     "    background-color: transparent;"
                                     "}");
}

ListenSubSub::~ListenSubSub()
{
    delete ui;
}

bool ListenSubSub::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->you && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            checkAnswer();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ListenSubSub::checkAnswer()
{
    QString userInput = ui->you->text();

    if (userInput.isEmpty())
    {
        QMessageBox::warning(this, "提示", "请输入答案");
        return;
    }
    if (answer == "")
    {
        QMessageBox::warning(this, "提示", "请先聆听");
        return;
    }

    bool isCorrect = compareAnswer(userInput, answer);
    qDebug() << userInput << " " << answer;
    ui->result->setText(isCorrect ? "正确" : "错误");

    logListenEvent(isCorrect);

    // 隐藏覆盖标签
    ui->answerCover->hide();

    // 禁用编辑框
    ui->you->setEnabled(false);
}

void ListenSubSub::setsubMode(int modenum)
{
    mode = modenum;
    answer = "";

    // 根据模式设置提示文本
    switch (mode)
    {
    case 1:
        ui->remindlabel->setText("第一个音是标准音A，请输入你听到的第二个音，如：C#");
        break;
    case 2:
        ui->remindlabel->setText("第一个音是标准音A，请输入你在其后听到的两个音，如：G#C，请把较低的音写在前面，无需空格");
        break;
    case 3:
        ui->remindlabel->setText("第一个音是标准音A，请输入你在其后听到的和弦，如：C，C#m，Faug，Gdim");
        break;
    case 4:
        ui->remindlabel->setText("第一个音是标准音A，请输入你在其后听到的和弦，如：Fmaj7，G#m7，B7");
        break;
    default:
        ui->remindlabel->setText("TextLabel");
        break;
    }
}

void ListenSubSub::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/lss2.png"));
}

bool ListenSubSub::compareAnswer(const QString &userInput, const QString &correctAnswer)
{
    return userInput.trimmed().toLower() == correctAnswer.trimmed().toLower();
}

void ListenSubSub::on_play_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString filePath = dir + "/test";

    // 如果当前没有设置源，设置源
    if (player->source().isEmpty())
    {
        ChordGenerator generator;
        std::pair<Score, QString> pair = generator.generateExam(mode);
        Score score = pair.first;
        answer = pair.second;
        ui->answer->setText(answer);
        scoreToWAV(score, filePath);
        filePath += "/tmp.wav";

        QUrl url = QUrl::fromLocalFile(filePath);
        player->setSource(url);
        player->setAudioOutput(audioOutput);
    }

    // 根据当前播放状态切换播放/暂停
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
        ui->play->setChecked(false);
        ui->play->setIcon(QIcon(":/MyPicture/p.png"));
    }
    else
    {
        player->play();
        ui->play->setChecked(true);
        ui->play->setIcon(QIcon(":/MyPicture/stp.png"));
    }
}

void ListenSubSub::on_refresh_clicked()
{
    qDebug() << "点击刷新按钮";

    // 清除结果和输入
    ui->result->clear();
    ui->you->clear();

    // 启用编辑框
    ui->you->setEnabled(true);

    // 停止当前播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
    player->setSource(QUrl());
    qDebug() << "清空：" << player->source().isEmpty();

    answer = "";
    ui->answer->setText("");

    // 显示覆盖标签
    ui->answerCover->show();
}

void ListenSubSub::on_BackListenSub_clicked()
{
    // 停止播放并释放资源
    player->stop();
    player->setSource(QUrl()); // 清除当前源
    ui->answerCover->show();

    // 清除结果和输入
    ui->result->clear();
    ui->you->clear();

    // 启用编辑框
    ui->you->setEnabled(true);

    // 停止当前播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    // 发出返回信号
    emit this->back();
}

void ListenSubSub::logListenEvent(bool isCorrect)
{
    // 1. 定义日志文件路径（放在 test 文件夹下，保持一致）
    QString logFilePath = QCoreApplication::applicationDirPath() + "/log_file/listen_log.txt";
    QFile logFile(logFilePath);

    // 2. 以追加模式打开文件
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&logFile);

        // 3. 准备要写入的日志条目
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString resultString = isCorrect ? "1" : "0"; // 根据布尔值生成文本

        // 格式: 时间, 难度模式, 结果
        QString logEntry = QString("%1, Mode: %2, Result: %3")
                               .arg(currentTime)
                               .arg(mode) // mode 是您已有的成员变量
                               .arg(resultString);

        // 4. 写入文件并换行
        out << logEntry << "\n";
        logFile.close();

        qDebug() << "Listening event logged:" << logEntry;
    }
    else
    {
        qWarning() << "Failed to open listening log file for writing:" << logFilePath;
    }
}
