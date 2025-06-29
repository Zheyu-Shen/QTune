#include <QProcess>
#include <QTemporaryFile>
#include <generatescore.h>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <vector>
#include <algorithm>
#include <map>
#include <QCoreApplication>
#include "playscore.h"

// 将整数转换为变长数量 (Variable Length Quantity)
QByteArray intToVLQ(int value)
{
    QByteArray result;

    if (value == 0)
    {
        result.append(static_cast<char>(0));
        return result;
    }

    std::vector<quint8> bytes;
    while (value > 0)
    {
        bytes.push_back(value & 0x7F);
        value >>= 7;
    }

    // 反转并设置continuation bits
    for (int i = bytes.size() - 1; i >= 0; i--)
    {
        quint8 byte = bytes[i];
        if (i > 0)
        {
            byte |= 0x80; // 设置continuation bit
        }
        result.append(static_cast<char>(byte));
    }

    return result;
}

// 写入32位大端整数
void writeInt32BE(QDataStream &stream, quint32 value)
{
    stream << static_cast<quint8>((value >> 24) & 0xFF);
    stream << static_cast<quint8>((value >> 16) & 0xFF);
    stream << static_cast<quint8>((value >> 8) & 0xFF);
    stream << static_cast<quint8>(value & 0xFF);
}

// 写入16位大端整数
void writeInt16BE(QDataStream &stream, quint16 value)
{
    stream << static_cast<quint8>((value >> 8) & 0xFF);
    stream << static_cast<quint8>(value & 0xFF);
}

bool scoreToMIDI(const Score &score, const QString &outputPath)
{
    QFile file(outputPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        return false;
    }

    QDataStream stream(&file);

    // MIDI文件参数
    const int ticksPerQuarter = 480;
    const double secondsPerQuarter = 0.5; // 120 BPM

    // 收集所有轨道
    std::set<int> trackNumbers;
    for (const auto &note : score.notes)
    {
        trackNumbers.insert(note.track);
    }

    // 创建MIDI事件列表
    std::vector<std::vector<MidiEvent>> tracks(trackNumbers.size());
    std::map<int, int> trackMapping;
    int trackIndex = 0;

    for (int trackNum : trackNumbers)
    {
        trackMapping[trackNum] = trackIndex++;
    }

    // 转换音符为MIDI事件
    for (const auto &note : score.notes)
    {
        int midiTrack = trackMapping[note.track];
        int startTick = int(note.startTime / secondsPerQuarter * ticksPerQuarter);
        int endTick = int((note.startTime + note.duration) / secondsPerQuarter * ticksPerQuarter);

        // Note On事件
        QByteArray noteOnData;
        noteOnData.append(static_cast<char>(0x90)); // Note On, channel 0
        noteOnData.append(static_cast<char>(note.pitch));
        noteOnData.append(static_cast<char>(std::max(1, std::min(127, note.velocity))));

        tracks[midiTrack].emplace_back(midiTrack, 0, noteOnData, startTick);

        // Note Off事件
        QByteArray noteOffData;
        noteOffData.append(static_cast<char>(0x80)); // Note Off, channel 0
        noteOffData.append(static_cast<char>(note.pitch));
        noteOffData.append(static_cast<char>(64)); // Release velocity

        tracks[midiTrack].emplace_back(midiTrack, 0, noteOffData, endTick);
    }

    // 排序每个轨道的事件
    for (auto &track : tracks)
    {
        std::sort(track.begin(), track.end(),
                  [](const MidiEvent &a, const MidiEvent &b)
                  {
                      return a.absoluteTime < b.absoluteTime;
                  });
    }

    // 计算delta time
    for (auto &track : tracks)
    {
        int lastTime = 0;
        for (auto &event : track)
        {
            int deltaTime = event.absoluteTime - lastTime;
            event.deltaTime = deltaTime;
            lastTime = event.absoluteTime;
        }

        // 添加End of Track事件
        if (!track.empty())
        {
            QByteArray endTrackData;
            endTrackData.append(static_cast<char>(0xFF)); // Meta event
            endTrackData.append(static_cast<char>(0x2F)); // End of Track
            endTrackData.append(static_cast<char>(0x00)); // Length = 0
            track.emplace_back(0, 0, endTrackData, track.back().absoluteTime);
        }
    }

    // 写入MIDI文件头
    stream.writeRawData("MThd", 4);                            // Header chunk ID
    writeInt32BE(stream, 6);                                   // Header length
    writeInt16BE(stream, 1);                                   // Format type 1 (multiple tracks)
    writeInt16BE(stream, static_cast<quint16>(tracks.size())); // Number of tracks
    writeInt16BE(stream, ticksPerQuarter);                     // Ticks per quarter note

    // 写入每个轨道
    for (const auto &track : tracks)
    {
        // 计算轨道数据大小
        QByteArray trackData;
        QDataStream trackStream(&trackData, QIODevice::WriteOnly);

        for (const auto &event : track)
        {
            // 写入delta time
            QByteArray deltaTimeVLQ = intToVLQ(event.deltaTime);
            trackStream.writeRawData(deltaTimeVLQ.data(), deltaTimeVLQ.size());

            // 写入事件数据
            trackStream.writeRawData(event.data.data(), event.data.size());
        }

        // 写入轨道头
        stream.writeRawData("MTrk", 4);                               // Track chunk ID
        writeInt32BE(stream, static_cast<quint32>(trackData.size())); // Track length

        // 写入轨道数据
        stream.writeRawData(trackData.data(), trackData.size());
    }

    file.close();
    return true;
}

bool scoreToWAV(const Score &score, const QString &outputPath)
{
    QDir dir;
    if (!dir.exists(outputPath)) {
        dir.mkpath(outputPath);
    }
    // 1. 先将Score转换为MIDI文件
    QString tempMidiPath = outputPath + "/tmp.mid";
    if (!scoreToMIDI(score, tempMidiPath))
    {
        return false;
    }

    // 2. 使用FluidSynth将MIDI转换为WAV
    // 构造 fluidsynth 路径（项目相对路径）
    QString projectPath = QString(PROJECT_SOURCE_DIR);
    const QString exePath = projectPath + "/fluidsynth/bin/fluidsynth.exe";
    const QString sf2Path = projectPath + "/GeneralUser-GS/GeneralUser-GS.sf2";

    // 安全检查
    if (!QFile::exists(exePath))
    {
        qWarning() << "fluidsynth.exe not found:" << exePath;
        return false;
    }
    if (!QFile::exists(sf2Path))
    {
        qWarning() << "SoundFont (.sf2) not found:" << sf2Path;
        return false;
    }

    QString tempWavPath = outputPath + "/tmp.wav";
    QProcess fluidsynth;
    QStringList args;
    args << "-ni" << sf2Path << tempMidiPath << "-F" << tempWavPath
         << "-r" << "44100" << "-o" << "synth.gain=0.8"
         << "-o" << "synth.sample-rate=44100"
         << "-o" << "audio.period-size=1024"
         << "-o" << "audio.periods=8";
    // << "-o" <<"audio.enabled=no";//<< "audio.driver=file";
    fluidsynth.setProcessChannelMode(QProcess::MergedChannels); // 捕捉输出
    fluidsynth.start(exePath, args);
    fluidsynth.waitForFinished();
    QString output = fluidsynth.readAllStandardOutput();
    qDebug() << "fluidsynth output:" << output;

    if (!QFile::exists(tempWavPath))
        qDebug() << "Unable to transform to wav.";
    return QFile::exists(tempWavPath);
}
