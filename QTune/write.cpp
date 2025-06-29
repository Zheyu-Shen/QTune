#include "write.h"
#include "ui_write.h"
#include "writesub.h"
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QMediaFormat>
#include <QMediaDevices>
// #include <QStandardPaths>
#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QCoreApplication>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSource>
#include <QBuffer>
#include <QDataStream>
#include "midifile/MidiFile.h"
#include "note.h"
#define USE_KISS_FFT
#include "Gist/Gist.h"
#include "drawscore.h"
using namespace smf;

Write::Write(QWidget *parent)
    : QWidget(parent), ui(new Ui::Write)
{
    ui->setupUi(this);

    isRecording = false;

    // 初始化播放器
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);

    // 初始化状态
    hasMidi = false;
    hasSound = false;
    resetState();

    // 初始禁用播放按钮
    // ui->play->setEnabled(false);

    // 连接录音错误信号
    /*
    connect(recorder, &QMediaRecorder::errorOccurred, this, [this](QMediaRecorder::Error error, const QString &errorString)
            {
        qDebug() << "录音错误:" << error << errorString;
        QMessageBox::critical(this, "录音错误", "录音失败: " + errorString); });
    */

    ui->BackHome->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: transparent;"         // 透明背景
                                "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                "}"
                                "QPushButton:hover {" // 悬浮时保持透明
                                "    background-color: transparent;"
                                "}");
    this->writesub = new WriteSub;
    connect(writesub, &WriteSub::back, this, [this]() {
        writesub->hide();
        this->show();
    });
}

Write::~Write()
{
    delete ui;
}

void Write::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/w4.png"));
}

void Write::resetState()
{
    // 重置所有状态
    hasMidi = false;
    hasSound = false;
    isRecording = false;
    currentMidiPath.clear();
    currentSoundPath.clear();

    // 更新UI
    ui->midi->setText("选择midi文件");
    ui->conduct->setText("开始处理");
    ui->sound->setText("开始录音");
    ui->sound->setEnabled(true);

    // 更新播放按钮状态
    updatePlayButtonState();
}

void Write::updatePlayButtonState()
{
    // 只有在有MIDI文件或录音文件时才启用播放按钮
    ui->sound->setEnabled(true);
}

void Write::on_midi_clicked()
{
    if (isRecording)
    {
        QMessageBox::warning(this, "警告", "请先停止录音");
        return;
    }

    if (hasSound)
    {
        QMessageBox::warning(this, "警告", "请先清除当前录音");
        return;
    }

    // 打开文件选择对话框
    QString filePath = QFileDialog::getOpenFileName(this, "选择MIDI文件", "", "MIDI文件 (*.mid *.midi)"); // 可以在这里改成其他类型的音频文件
    if (filePath.isEmpty())
    {
        return;
    }

    // 更新状态
    currentMidiPath = filePath;
    hasMidi = true;
    hasSound = false;

    // 输出当前MIDI文件路径
    qDebug() << "当前MIDI文件路径:" << currentMidiPath;

    // 更新UI
    ui->midi->setText(QFileInfo(filePath).fileName());

    // 更新播放按钮状态
    updatePlayButtonState();
}

void Write::on_sound_clicked()
{
    if (hasMidi)
    {
        QMessageBox::warning(this, "警告", "请先清除当前MIDI文件");
        return;
    }

    if (isRecording)
    {
        stopRecording();
        // 录音完成后，将按钮文本改为"播放录音"
        ui->sound->setText("播放录音");
    }
    else if (hasSound)
    {
        // 播放录音文件
        QUrl url = QUrl::fromLocalFile(currentSoundPath);
        player->setSource(url);
        player->play();
    }
    else
    {
        startRecording();
    }
}


void Write::startRecording()
{
    // 创建录音文件夹
    QString dir = QCoreApplication::applicationDirPath();
    QDir luyinDir(dir + "/luyin");
    if (!luyinDir.exists())
    {
        if (!luyinDir.mkpath("."))
        {
            QMessageBox::critical(this, "错误", "无法创建录音文件夹");
            return;
        }
    }

    // 设置录音文件路径
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    currentSoundPath = QString("%1/luyin/录音_%2.wav").arg(dir).arg(timestamp);

    // 检查默认音频输入设备
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    if (inputDevice.isNull())
    {
        QMessageBox::critical(this, "错误", "无可用麦克风设备");
        qDebug() << "无可用麦克风设备";
        return;
    }

    // 获取设备支持的最佳格式，而不是强制设置格式
    fmt = inputDevice.preferredFormat();

    // 如果需要特定格式，确保检查兼容性
    QAudioFormat desiredFormat;
    desiredFormat.setSampleRate(44100);
    desiredFormat.setChannelCount(1); // 使用单声道可能减少失真
    desiredFormat.setSampleFormat(QAudioFormat::Int16);

    if (inputDevice.isFormatSupported(desiredFormat))
    {
        fmt = desiredFormat;
        qDebug() << "使用自定义音频格式";
    }
    else
    {
        qDebug() << "设备不支持请求的格式，使用设备首选格式:" << fmt.sampleRate()
                 << "Hz," << fmt.channelCount() << "声道";
    }

    // 使用QAudioSource进行录音
    audioInput = new QAudioSource(inputDevice, fmt, this);

    // 设置适当的缓冲区大小，避免数据丢失
    audioInput->setBufferSize(16384); // 16KB缓冲区

    audioFile = new QFile(currentSoundPath, this);
    if (!audioFile->open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::critical(this, "错误", "无法创建录音文件");
        delete audioInput;
        delete audioFile;
        audioInput = nullptr;
        audioFile = nullptr;
        return;
    }

    // 写入WAV文件头
    writeWavHeader(audioFile, fmt);

    // 设置IO设备
    QIODevice *io = audioInput->start();
    if (io)
    {
        // 使用QIODevice进行数据读写，避免直接写入文件
        connect(io, &QIODevice::readyRead, this, [this, io]()
                {
                    if (audioFile && audioFile->isOpen()) {
                        // 读取数据并写入文件
                        QByteArray buffer = io->readAll();
                        if (!buffer.isEmpty()) {
                            audioFile->write(buffer);
                        }
                    } });
    }
    else
    {
        QMessageBox::critical(this, "错误", "无法启动录音");
        audioFile->close();
        delete audioInput;
        delete audioFile;
        audioInput = nullptr;
        audioFile = nullptr;
        return;
    }

    isRecording = true;

    // 更新UI
    ui->sound->setText("停止录音");

    qDebug() << "开始录音，保存路径:" << currentSoundPath
             << "格式:" << fmt.sampleRate() << "Hz,"
             << fmt.channelCount() << "声道,"
             << (fmt.sampleFormat() == QAudioFormat::Int16 ? "16位" : "其他格式");
}

void Write::writeWavHeader(QFile *file, const QAudioFormat &format)
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

void Write::updateWavHeader(QFile *file)
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

void Write::stopRecording()
{
    if (!isRecording)
        return;

    isRecording = false;

    if (audioInput)
    {
        audioInput->stop();

        if (audioFile && audioFile->isOpen())
        {
            // 更新WAV文件头
            updateWavHeader(audioFile);
            audioFile->close();
        }

        delete audioInput;
        audioInput = nullptr;

        if (audioFile)
        {
            delete audioFile;
            audioFile = nullptr;
        }
    }

    // 检查文件是否创建成功
    QFile file(currentSoundPath);
    if (!file.exists() || file.size() <= 44)
    { // 44是WAV头的大小
        QMessageBox::critical(this, "错误", "录音文件保存失败或为空");
        hasSound = false;
        currentSoundPath.clear();
        ui->sound->setText("开始录音");
        return;
    }

    // 更新状态
    hasSound = true;
    hasMidi = false;

    // 更新UI
    ui->sound->setText("开始录音");
    ui->midi->setText("选择midi文件");

    // 更新播放按钮状态
    updatePlayButtonState();

    qDebug() << "停止录音，文件大小:" << file.size() << "字节";
}

void Write::on_conduct_clicked()
{
    if (!hasMidi && !hasSound)
    {
        QMessageBox::warning(this, "警告", "请先载入MIDI文件或录音");
        return;
    }

    // 处理音频
    processAudio();

    QString dir = QCoreApplication::applicationDirPath();
    QString yuepuFolder = dir + "/write_file";
    QString imagePath = yuepuFolder + "/tmp.png";
    QImage img(imagePath);
    if (img.isNull()) {
        QMessageBox::warning(this, "警告", "未能加载生成的谱图！");
    } else {
        this->writesub->setResultImage(img);
        this->hide();               // 隐藏主窗口
        this->writesub->show();     // 弹出子窗口
    }
    ui->conduct->setText("开始处理");
}

bool parseMidiToScore(const QString &filepath, Score &outScore)
{
    MidiFile midi; // midifile 对象
    if (!midi.read(filepath.toStdString()))
    {
        qWarning() << "无法读取 MIDI 文件：" << filepath;
        return false;
    }
    midi.doTimeAnalysis(); // 计算每个事件的绝对时间（tick）
    midi.linkNotePairs();  // 将 NoteOn/NoteOff 配对，方便计算时长

    // 获取 BPM 信息（如果有多段 tempo，可遍历获取）
    double ticksPerQuarter = midi.getTicksPerQuarterNote();

    // 遍历所有 track 的所有事件
    for (int track = 0; track < midi.getTrackCount(); ++track)
    {
        for (int event = 0; event < midi[track].size(); ++event)
        {
            auto &mfe = midi[track][event];
            if (!mfe.isNoteOn())
                continue; // 只处理 NoteOn（配对了 NoteOff）

            // 配对的 NoteOff 事件
            auto *offEvent = mfe.getLinkedEvent();
            if (!offEvent)
                continue;

            // 计算秒数： midifile 提供了 ticksToSeconds()
            double startSec = midi.getTimeInSeconds(mfe.tick);
            double endSec = midi.getTimeInSeconds(offEvent->tick);

            Note note;
            note.track = track;
            note.channel = mfe.getChannelNibble();
            note.pitch = mfe.getKeyNumber();
            note.velocity = mfe.getVelocity();
            note.startTime = startSec;
            note.duration = endSec - startSec;
            outScore.addNote(note);
        }
    }
    return true;
}
void Write::processAudio()
{
    // TODO: 实现音频处理逻辑
    // currentMidiPath选择的midi文件路径
    // currentSoundPath录音路径
    ui->conduct->setText("处理中...");
    QApplication::processEvents(); // 保证 UI 立即刷新

    // 如果当前是 MIDI 文件，就解析
    if (hasMidi)
    {
        // 清空上次的解析结果
        midiScore.clear();

        // 调用解析函数
        bool ok = parseMidiToScore(currentMidiPath, midiScore);
        if (!ok)
        {
            QMessageBox::critical(this, "错误", "MIDI 解析失败");
            ui->conduct->setText("开始处理");
            return;
        }
        qDebug() << "----- MIDI Score Debug Start -----";
        for (unsigned long long i = 0; i < midiScore.notes.size(); ++i)
        {
            const Note &n = midiScore.notes[i];
            qDebug().nospace()
                << "Note[" << i << "]: "
                << "track=" << n.track << ", "
                << "channel=" << (n.channel + 1) << ", "
                << "pitch=" << n.pitch << ", "
                << "vel=" << n.velocity << ", "
                << "start=" << n.startTime << ", "
                << "dur=" << n.duration;
        }
        qDebug() << "----- MIDI Score Debug End   -----";
        QString dir = QCoreApplication::applicationDirPath();
        QString yuepuFolder(dir + "/write_file");
        drawScore(midiScore, yuepuFolder);

    }

    else
    {
        audioScore.clear();

        // 1) 读取并解析 WAV 到 float 单声道
        QFile file(currentSoundPath);
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, "错误", "无法打开录音文件进行分析");
            ui->conduct->setText("开始处理");
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
        ui->conduct->setText(
            QString("检测完成，共 %1 个音符").arg(audioScore.notes.size()));
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
        QString dir = QCoreApplication::applicationDirPath();
        QString yuepuFolder(dir + "/write_file");
        drawScore(audioScore, yuepuFolder);
    }
}

void Write::on_refresh_clicked()
{
    // 停止录音
    if (isRecording)
    {
        stopRecording();
    }

    // 停止播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    // 重置所有状态
    resetState();
}

void Write::on_BackHome_clicked()
{
    // 停止录音
    if (isRecording)
    {
        // recorder->stop();
    }

    // 停止播放
    if (player->playbackState() == QMediaPlayer::PlayingState)
    {
        player->stop();
    }

    emit this->back();
}
