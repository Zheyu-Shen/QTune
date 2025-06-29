#ifndef PLAYSCORE_H
#define PLAYSCORE_H

#pragma once
#include <QProcess>
#include <QTemporaryFile>
#include <generatescore.h>
#include <QDir>

struct MidiEvent {
    int track;
    int deltaTime;
    QByteArray data;
    int absoluteTime;

    MidiEvent(int t, int dt, const QByteArray& d, int at)
        : track(t), deltaTime(dt), data(d), absoluteTime(at) {}
};

QByteArray intToVLQ(int value);
void writeInt32BE(QDataStream& stream, quint32 value);
void writeInt16BE(QDataStream& stream, quint16 value);
bool scoreToMIDI(const Score& score, const QString& outputPath);
bool scoreToWAV(const Score& score, const QString& outputPath);

#endif // PLAYSCORE_H
