#include "noteaudiosource.h"

NoteAudioSource::NoteAudioSource(double frequency, double durationSeconds, QObject* parent)
    : QIODevice(parent), m_frequency(frequency), m_duration(durationSeconds),
    m_sampleRate(44100), m_sampleSize(16), m_position(0) {
    m_totalSamples = int(m_duration * m_sampleRate);
}

void NoteAudioSource::start() {
    open(QIODevice::ReadOnly);
    m_position = 0;
}

void NoteAudioSource::stop() {
    close();
}

qint64 NoteAudioSource::readData(char* data, qint64 maxlen) {
    qint16* output = reinterpret_cast<qint16*>(data);
    qint64 samplesToWrite = qMin(maxlen / 2, (m_totalSamples - m_position));

    for (int i = 0; i < samplesToWrite; ++i) {
        double t = double(m_position) / m_sampleRate;
        double value = qSin(2.0 * M_PI * m_frequency * t);
        output[i] = qint16(value * 32767);
        ++m_position;
    }

    return samplesToWrite * 2; // 每个采样点2字节
}

qint64 NoteAudioSource::bytesAvailable() const {
    return (m_totalSamples - m_position) * 2;
}
