#include "singsub.h"
#include "ui_singsub.h"
#include "generatescore.h"
#include "drawscore.h"
#include "playscore.h"
#include "note.h"
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QPushButton>
#include <QDebug>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QAudioSource>
#include <QBuffer>
#include <QDataStream>
#include <QAudioOutput>
#include <QRandomGenerator>
#include "Gist/Gist.h"

SingSub::SingSub(QWidget *parent)
    : QWidget(parent), ui(new Ui::SingSub), difficultyLevel(0), isRecording(false), currentYuepuIndex(0), imageCount(0), currentScore(0)
{
    ui->setupUi(this);

    // 初始化播放器
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    // 连接播放器状态变化信号
    connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state)
            {
        if (state == QMediaPlayer::StoppedState) {
            ui->recording->setText("播放录音");
        } });

    // showYuepu(); //显示第一张乐谱

    // 设置返回按钮样式
    ui->BackSing->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: transparent;"         // 透明背景
                                "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                "}"
                                "QPushButton:hover {" // 悬浮时保持透明
                                "    background-color: transparent;"
                                "}");
}

SingSub::~SingSub()
{
    delete ui;
}

void SingSub::on_BackSing_clicked()
{
    // 停止播放
    saveScoreToFile();
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
    isRecording = false;
    currentScore = 0;
    ui->scoreLabel->setText("");
    ui->recording->setText("开始录音");
    emit this->back();
}

void SingSub::setDifficulty(int level, Score x)
{
    difficultyLevel = level;
    testScore = x;
    showYuepu();
}

void SingSub::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/ss1.png"));
}

void SingSub::showYuepu()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepu(dir + "/sing_file/tmp.png");

    // 加载图片
    QPixmap pixmap(yuepu);
    if (pixmap.isNull())
    {
        ui->yuepuLabel->setText("无法加载乐谱图片");
        return;
    }

    // 计算缩放后的尺寸，保持原始比例
    QSize labelSize = ui->yuepuLabel->size();
    QSize scaledSize = pixmap.size();
    scaledSize.scale(labelSize, Qt::KeepAspectRatio);

    // 缩放图片
    pixmap = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 设置图片并居中显示
    ui->yuepuLabel->setPixmap(pixmap);
    ui->yuepuLabel->setAlignment(Qt::AlignCenter);
}

void SingSub::on_refresh_clicked()
{
    saveScoreToFile();
    qDebug() << "refesh clicked";
    // 停止播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    // 清除录音文件
    if (!currentRecordingPath.isEmpty())
    {
        QFile::remove(currentRecordingPath);
        currentRecordingPath.clear();
    }

    // 重置状态
    isRecording = false;
    currentScore = 0;
    ui->scoreLabel->setText("");
    ui->recording->setText("开始录音");
    ui->answerplay->setText("");

    // 切换到下一张图片
    Score score;
    ChordGenerator generator;
    if (difficultyLevel == 1)
    {
        score = generator.generateSightSinging1();
    }
    else if (difficultyLevel == 2)
        score = generator.generateSightSinging2();
    else if (difficultyLevel == 3)
        score = generator.generateSightSinging3();
    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder(dir + "/sing_file");
    drawScore(score, yuepuFolder);
    scoreToWAV(score, yuepuFolder);
    testScore = score;
    showYuepu();
}

void SingSub::on_recording_clicked()
{
    if (isRecording)
    {
        // 如果正在录音，则停止录音
        stopRecording();
    }
    else if (!currentRecordingPath.isEmpty() && QFile::exists(currentRecordingPath))
    {
        // 如果有录音文件，则播放/暂停
        if (player->playbackState() == QMediaPlayer::PlayingState)
        {
            player->pause();
            ui->recording->setText("继续播放");
        }
        else
        {
            // 如果当前没有设置源，设置源
            if (player->source().isEmpty() || player->source().toLocalFile() != currentRecordingPath)
            {
                QUrl url = QUrl::fromLocalFile(currentRecordingPath);
                player->setSource(url);
                player->setAudioOutput(audioOutput);
            }
            player->play();
            ui->recording->setText("暂停播放");
        }
    }
    else
    {
        // 开始新的录音
        startRecording();
    }
}

void SingSub::clearRecording()
{
    currentRecordingPath.clear();
    isRecording = false;
    currentScore = 0;
    ui->scoreLabel->setText("");
    ui->recording->setText("开始录音");
}

void SingSub::updateScore()
{
    ui->scoreLabel->setText(QString("%1").arg(currentScore));
}

void SingSub::updateRecordingState()
{
    // 录音状态更新处理
    if (isRecording)
    {
        ui->recording->setText("停止录音");
    }
    else if (!currentRecordingPath.isEmpty() && QFile::exists(currentRecordingPath))
    {
        if (player->playbackState() == QMediaPlayer::PlayingState)
        {
            ui->recording->setText("暂停播放");
        }
        else
        {
            ui->recording->setText("播放录音");
        }
    }
    else
    {
        ui->recording->setText("开始录音");
    }
}

void SingSub::writeWavHeader(QFile *file, const QAudioFormat &format)
{
    if (!file || !file->isOpen())
        return;

    file->seek(0); // 确保从文件开始写入

    QDataStream out(file);
    out.setByteOrder(QDataStream::LittleEndian);

    // 确定位深度
    int bitsPerSample = 0;
    switch (format.sampleFormat())
    {
    case QAudioFormat::UInt8:
        bitsPerSample = 8;
        break;
    case QAudioFormat::Int16:
        bitsPerSample = 16;
        break;
    case QAudioFormat::Int32:
        bitsPerSample = 32;
        break;
    case QAudioFormat::Float:
        bitsPerSample = 32;
        break;
    default:
        qWarning() << "不支持的采样格式:" << format.sampleFormat();
        bitsPerSample = 16;
    }

    quint16 audioFormat = 1; // PCM格式
    // 如果是浮点格式，使用IEEE浮点标识
    if (format.sampleFormat() == QAudioFormat::Float)
    {
        audioFormat = 3; // IEEE浮点
    }

    quint16 numChannels = format.channelCount();
    quint32 sampleRate = format.sampleRate();
    quint16 blockAlign = numChannels * bitsPerSample / 8;
    quint32 byteRate = sampleRate * blockAlign;

    // RIFF头
    out.writeRawData("RIFF", 4);
    out << quint32(0); // 文件大小占位，后续更新
    out.writeRawData("WAVE", 4);

    // fmt块
    out.writeRawData("fmt ", 4);
    out << quint32(16);            // fmt块大小（标准PCM为16字节）
    out << quint16(audioFormat);   // 音频格式
    out << quint16(numChannels);   // 声道数
    out << quint32(sampleRate);    // 采样率
    out << quint32(byteRate);      // 数据传输速率
    out << quint16(blockAlign);    // 数据块对齐
    out << quint16(bitsPerSample); // 采样位深

    // data块
    out.writeRawData("data", 4);
    out << quint32(0); // 数据大小占位，后续更新
}

void SingSub::updateWavHeader(QFile *file)
{
    if (!file || !file->isOpen())
        return;

    // 获取文件大小
    qint64 fileSize = file->size();
    if (fileSize <= 44) // WAV头部占用44字节
        return;

    // 更新RIFF块大小
    file->seek(4);
    QDataStream riffOut(file);
    riffOut.setByteOrder(QDataStream::LittleEndian);
    riffOut << quint32(fileSize - 8);

    // 更新data块大小
    file->seek(40);
    QDataStream dataOut(file);
    dataOut.setByteOrder(QDataStream::LittleEndian);
    dataOut << quint32(fileSize - 44);

    // 确保所有数据都写入到文件
    file->flush();
}

void SingSub::startRecording()
{
    // 检查默认音频输入设备
    if (QMediaDevices::defaultAudioInput().isNull())
    {
        qDebug() << "无可用麦克风设备";
        QMessageBox::critical(this, "错误", "无可用麦克风设备");
        return;
    }

    // 设置录音格式
    QAudioFormat fmt;
    fmt.setSampleRate(44100);
    fmt.setChannelCount(2);
    fmt.setSampleFormat(QAudioFormat::Int16);

    // 检查格式是否支持
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (!inputDevice.isFormatSupported(fmt))
    {
        qDebug() << "默认音频格式不支持，尝试使用最近的格式";
        fmt = inputDevice.preferredFormat();
    }

    // 创建录音文件夹（与乐谱图片在同一目录）
    QString dir = QCoreApplication::applicationDirPath();
    QDir yuepuDir(dir + "/sing_file");

    // 设置录音文件路径
    currentRecordingPath = QString("%1/sing_file/record.wav")
                               .arg(dir);
    //.arg(difficultyLevel)
    //.arg(currentYuepuIndex + 1);

    // 创建音频输入设备
    audioInput = new QAudioSource(fmt, this);
    audioFile = new QFile(currentRecordingPath, this);
    audioFile->open(QIODevice::WriteOnly | QIODevice::Truncate);

    // 写入WAV文件头
    writeWavHeader(audioFile, fmt);

    // 开始录音
    audioInput->start(audioFile);
    isRecording = true;

    // 更新UI
    updateRecordingState();

    qDebug() << "开始录音，保存路径:" << currentRecordingPath;
}

void SingSub::stopRecording()
{
    if (!isRecording)
        return;

    // 停止录音
    if (audioInput)
    {
        audioInput->stop();

        // 更新WAV文件头
        updateWavHeader(audioFile);

        audioFile->close();
        delete audioInput;
        audioInput = nullptr;
        delete audioFile;
        audioFile = nullptr;
    }

    isRecording = false;

    // 检查文件是否创建成功
    QFile file(currentRecordingPath);
    if (!file.exists() || file.size() == 0)
    {
        QMessageBox::critical(this, "错误", "录音文件保存失败");
        // 如果保存失败，重置状态
        currentRecordingPath.clear();
        updateRecordingState();
        return;
    }

    // 更新UI
    updateRecordingState();

    qDebug() << "停止录音，文件大小:" << file.size() << "字节";

    // 对录音进行打分
    scoreRecording();
}
double scorePerformance(const Score &testScore, const Score &audioScore)
{
    if (testScore.notes.empty() || audioScore.notes.empty())
    {
        return 0.0; // 如果任一谱为空，返回0分
    }

    // 计算时间偏移量 - 将audioScore的时间调整为相对于其第一个音符的时间
    double audioStartTime = std::numeric_limits<double>::max();
    double testStartTime = std::numeric_limits<double>::max();

    // 找出两个谱的第一个音符时间
    for (const Note &note : audioScore.notes)
    {
        audioStartTime = std::min(audioStartTime, note.startTime);
    }

    for (const Note &note : testScore.notes)
    {
        testStartTime = std::min(testStartTime, note.startTime);
    }

    // 计算时间偏移，使两个谱的起始时间对齐
    double timeOffset = audioStartTime - testStartTime;

    double totalScore = 0.0;
    double maxPossibleScore = testScore.notes.size() * 100.0;

    // 对于标准谱中的每个音符
    for (const Note &testNote : testScore.notes)
    {
        // 寻找最匹配的音符
        double bestMatchScore = 0.0;

        for (const Note &audioNote : audioScore.notes)
        {
            // 调整audioNote的时间，使其与testNote基于相同的时间起点
            double adjustedAudioStartTime = audioNote.startTime - timeOffset;

            // 计算时间重叠
            double startOverlap = std::max(testNote.startTime, adjustedAudioStartTime);
            double endOverlap = std::min(testNote.startTime + testNote.duration,
                                         adjustedAudioStartTime + audioNote.duration);
            double overlapDuration = std::max(0.0, endOverlap - startOverlap);

            // 如果有时间重叠，计算匹配分数
            if (overlapDuration > 0)
            {
                // 音高匹配度 (0-50分)
                double pitchScore = 0.0;
                if (testNote.pitch == audioNote.pitch)
                {
                    pitchScore = 50.0;
                }
                else
                {
                    // 音高差越小，分数越高
                    int pitchDifference = std::abs(testNote.pitch - audioNote.pitch);
                    if (pitchDifference <= 2)
                    { // 允许小的音高偏差
                        pitchScore = 50.0 * (1.0 - pitchDifference / 4.0);
                    }
                }

                // 时间匹配度 (0-50分)
                // 计算重叠时间占原音符时长的比例
                double timeOverlapRatio = overlapDuration / testNote.duration;
                double timeScore = 50.0 * timeOverlapRatio;

                // 总分
                double matchScore = pitchScore + timeScore;

                // 更新最佳匹配
                if (matchScore > bestMatchScore)
                {
                    bestMatchScore = matchScore;
                }
            }
        }

        // 将该音符的最佳匹配分数加到总分中
        totalScore += bestMatchScore;
    }

    // 计算百分比分数 (0-100)
    double percentageScore = (totalScore / maxPossibleScore) * 100.0;

    // 确保分数在0-100范围内
    return std::min(100.0, std::max(0.0, percentageScore));
}
void SingSub::scoreRecording()
{
    // 这里实现打分逻辑
    audioScore.clear();

    // 1) 读取并解析 WAV 到 float 单声道
    QFile file(currentRecordingPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "错误", "无法打开录音文件进行分析");
        return;
    }
    QByteArray wavData = file.readAll();
    file.close();

    QDataStream stream(wavData);
    stream.setByteOrder(QDataStream::LittleEndian);
    // 验证 RIFF/WAVE
    char riff[4], wave[4];
    stream.readRawData(riff, 4);
    stream.skipRawData(4);
    stream.readRawData(wave, 4);
    if (memcmp(riff, "RIFF", 4) || memcmp(wave, "WAVE", 4))
    {
        QMessageBox::critical(this, "错误", "非法 WAV 文件");
        return;
    }
    // 找 fmt/data 块
    quint16 audioFormat = 0, numChannels = 0, bitsPerSample = 0;
    quint32 sampleRate = 0;
    while (!stream.atEnd())
    {
        char id[4];
        stream.readRawData(id, 4);
        quint32 size;
        stream >> size;
        if (!memcmp(id, "fmt ", 4))
        {
            stream >> audioFormat >> numChannels >> sampleRate;
            stream.skipRawData(6);
            stream >> bitsPerSample;
            if (size > 16)
                stream.skipRawData(size - 16);
        }
        else if (!memcmp(id, "data", 4))
        {
            break;
        }
        else
        {
            stream.skipRawData(size);
        }
    }
    int dataOffset = stream.device()->pos();
    QByteArray pcm = wavData.mid(dataOffset);

    // 解码到 float 单声道 [-1,1]
    int bytesPerSample = bitsPerSample / 8;
    int totalFrames = pcm.size() / (bytesPerSample * numChannels);
    std::vector<float> audio;
    audio.reserve(totalFrames);
    const char *d = pcm.constData();
    for (int i = 0; i < totalFrames; ++i)
    {
        float sum = 0;
        for (int c = 0; c < numChannels; ++c)
        {
            const char *ptr = d + (i * numChannels + c) * bytesPerSample;
            if (audioFormat == 1 && bitsPerSample == 16)
            {
                sum += *reinterpret_cast<const qint16 *>(ptr) / 32768.0f;
            }
            else if (audioFormat == 3 && bitsPerSample == 32)
            {
                sum += *reinterpret_cast<const float *>(ptr);
            }
        }
        audio.push_back(sum / numChannels);
    }

    qDebug() << "解码完成："
             << sampleRate << "Hz,"
             << numChannels << "声道,"
             << bitsPerSample << "位";

    // 2) 用 Gist 做音高检测
    const int frameSize = 2048;
    const int hopSize = 512;
    Gist<float> gist(frameSize, sampleRate);
    // gist.setFFTImpl<GistFFT_Kiss<float>>(); // 如需强制 Kiss FFT

    // 阈值设置
    const double minDur = 0.05;     // 丢弃短于50ms
    const float pitchTol = 1.5f;    // 判断新音符的音高跳变阈值
    const float energyThr = 0.001f; // RMS 能量阈值，避免把静音当音符

    bool noteOpen = false;
    double noteStart = 0;
    float lastPitch = 0;

    for (int i = 0; i + frameSize <= (int)audio.size(); i += hopSize)
    {
        // 送一帧给 Gist
        gist.processAudioFrame(audio.data() + i, frameSize);

        float pitch = gist.pitch();        // 当前帧音高
        float rms = gist.rootMeanSquare(); // 当前帧能量
        double t = double(i) / sampleRate;

        bool isTone = (pitch > 55 && pitch < 2000 && rms > energyThr);
        if (isTone)
        {
            if (!noteOpen)
            {
                // 新音符开始
                noteOpen = true;
                noteStart = t;
                lastPitch = pitch;
            }
            else if (std::abs(pitch - lastPitch) > pitchTol)
            {
                // 音高跳变过大，认为上一个音符结束
                double dur = t - noteStart;
                if (dur >= minDur)
                {
                    int midi = int(std::round(69 + 12 * std::log2(lastPitch / 440.0f)));
                    Note n{0, 0, midi, 80, noteStart, dur, 0, 0, 0, false};
                    audioScore.addNote(n);
                }
                // 开启新音符
                noteStart = t;
                lastPitch = pitch;
            }
            else
            {
                // 平滑更新
                lastPitch = lastPitch * 0.8f + pitch * 0.2f;
            }
        }
        else if (noteOpen)
        {
            // 静音——结束当前音符
            double dur = t - noteStart;
            if (dur >= minDur)
            {
                int midi = int(std::round(69 + 12 * std::log2(lastPitch / 440.0f)));
                Note n{0, 0, midi, 80, noteStart, dur, 0, 0, 0, false};
                audioScore.addNote(n);
            }
            noteOpen = false;
        }
    }
    // 文件尾部闭合
    if (noteOpen)
    {
        double tEnd = double(audio.size()) / sampleRate;
        double dur = tEnd - noteStart;
        if (dur >= minDur)
        {
            int midi = int(std::round(69 + 12 * std::log2(lastPitch / 440.0f)));
            Note n{0, 0, midi, 80, noteStart, dur, 0, 0, 0, false};
            audioScore.addNote(n);
        }
    }
    // 合并相邻且音高相同的音符
    std::vector<Note> mergedNotes;
    const double eps = 5e-2; // 容差，用于浮点比较

    for (const auto &n : audioScore.notes) {
        if (mergedNotes.empty()) {
            // 首个音符，直接放入
            mergedNotes.push_back(n);
        } else {
            Note &last = mergedNotes.back();
            // 判断是否与上一个音符相邻且音高相同
            bool samePitch = (n.pitch == last.pitch);
            bool adjacent = std::abs(n.startTime - (last.startTime + last.duration)) < eps;
            if (samePitch && adjacent) {
                // 合并：延长上一个音符的时长
                double newEnd = n.startTime + n.duration;
                last.duration = newEnd - last.startTime;
            } else {
                // 否则，作为新的音符加入
                mergedNotes.push_back(n);
            }
        }
    }

    // 用合并后的列表替换原始列表
    audioScore.notes = std::move(mergedNotes);
    qDebug() << "使用 Gist 检测，音符数 =" << audioScore.notes.size();
    qDebug() << "----- Audio Score Debug Start -----";
    for (unsigned long long i = 0; i < audioScore.notes.size(); ++i)
    {
        const Note &n = audioScore.notes[i];
        qDebug().nospace()
            << "Note[" << i << "]: "
            << "track=" << n.track << ", "
            << "channel=" << (n.channel + 1) << ", "
            << "pitch=" << n.pitch << ", "
            << "vel=" << n.velocity << ", "
            << "start=" << n.startTime << ", "
            << "dur=" << n.duration;
    }
    qDebug() << "----- Audio Score Debug End   -----";
    currentScore=scorePerformance(testScore,audioScore);
    updateScore();
    qDebug() << "录音打分完成，分数:" << currentScore;

}

void SingSub::startPlayback()
{
    if (currentRecordingPath.isEmpty())
    {
        QMessageBox::warning(this, "警告", "没有可播放的录音");
        return;
    }

    // 播放录音
    QUrl url = QUrl::fromLocalFile(currentRecordingPath);
    player->setSource(url);
    player->play();
}

void SingSub::stopPlayback()
{
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }
}

void SingSub::on_answerplay_clicked()
{
    QString dir = QCoreApplication::applicationDirPath();
    QString filePath(dir + "/sing_file/tmp.wav");

    // 如果当前正在播放答案音频，则暂停
    if (player->playbackState() == QMediaPlayer::PlayingState &&
        player->source().toLocalFile() == filePath)
    {
        player->pause();
        return;
    }

    // 如果当前正在播放录音，先停止
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    // 设置新的音频源
    QUrl url = QUrl::fromLocalFile(filePath);
    player->setSource(url);
    player->setAudioOutput(audioOutput);
    player->play();
}

void SingSub::on_rebegin_clicked()
{
    // 停止播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    // 清除录音文件
    if (!currentRecordingPath.isEmpty())
    {
        QFile::remove(currentRecordingPath);
        currentRecordingPath.clear();
    }

    // 重置状态
    isRecording = false;
    currentScore = 0;
    ui->scoreLabel->setText("");
    ui->recording->setText("开始录音");
}

void SingSub::saveScoreToFile()
{
    // 只有在用户真的录过音之后才保存分数
    if (currentRecordingPath.isEmpty()) {
        qDebug() << "No recording was made for the current sheet. Nothing to save.";
        return; // 如果没有录音，就不保存分数，直接返回
    }

    // 1. 定义日志文件路径
    QString logFilePath = QCoreApplication::applicationDirPath() + "/log_file/scores_log.txt";
    QFile logFile(logFilePath);

    // 2. 以追加模式打开文件，如果文件不存在会自动创建
    // QIODevice::Append: 在文件末尾追加
    // QIODevice::Text: 自动处理换行符
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream out(&logFile);

        // 3. 准备要写入的内容
        QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString logEntry = QString("%1, Mode: %2, Score: %3")
                               .arg(currentTime)
                               .arg(difficultyLevel)
                               .arg(QString::number(currentScore));

        // 4. 写入文件并换行
        out << logEntry << "\n";
        logFile.close();

        qDebug() << "Score saved to" << logFilePath << ":" << logEntry;
    }
    else
    {
        qWarning() << "Failed to open score log file for writing:" << logFilePath;
    }
}
