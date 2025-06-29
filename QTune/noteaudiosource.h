#ifndef NOTEAUDIOSOURCE_H
#define NOTEAUDIOSOURCE_H

#pragma once
#include <QIODevice>
#include <QAudioFormat>
#include <QtMath>

class NoteAudioSource : public QIODevice {
    Q_OBJECT

public:
    NoteAudioSource(double frequency, double durationSeconds, QObject* parent = nullptr);
    void start();
    void stop();

    qint64 readData(char* data, qint64 maxlen) override;
    qint64 writeData(const char*, qint64) override { return 0; }
    qint64 bytesAvailable() const override;

private:
    double m_frequency;
    double m_duration;
    int m_sampleRate;
    int m_sampleSize;
    int m_totalSamples;
    int m_position;
};

#endif // NOTEAUDIOSOURCE_H
