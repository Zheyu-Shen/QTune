#ifndef WRITE_H
#define WRITE_H

#include <QWidget>
#include <QMediaRecorder>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioInput>
#include <QAudioFormat>
#include <QFile>
#include <QAudioSource>
#include "note.h"
#include <QPainter>
#include "writesub.h"

namespace Ui
{
    class Write;
}

class Write : public QWidget
{
    Q_OBJECT

public:
    explicit Write(QWidget *parent = nullptr);
    ~Write();

    void paintEvent(QPaintEvent *event);

private slots:
    void on_midi_clicked();
    void on_sound_clicked();
    void on_conduct_clicked();
    void on_refresh_clicked();
    void on_BackHome_clicked();

signals:
    void back();

private:
    Ui::Write *ui;
    WriteSub *writesub = nullptr;    // ← 新增子窗口指针
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QAudioSource *audioInput;
    QFile *audioFile;
    bool isRecording;
    bool hasMidi;
    bool hasSound;
    QString currentMidiPath;
    QString currentSoundPath;
    QAudioFormat fmt;

    void resetState();
    void updatePlayButtonState();
    void startRecording();
    void stopRecording();
    void processAudio();
    void writeWavHeader(QFile *file, const QAudioFormat &format);
    void updateWavHeader(QFile *file);
    Score midiScore; // 用来存放 parseMidiToScore 的结果
    Score audioScore;
};

#endif // WRITE_H
