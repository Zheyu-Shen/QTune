#ifndef SINGSUB_H
#define SINGSUB_H

#include <QWidget>
#include <QDir>
#include <QFileInfoList>
#include <QImage>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QAudioFormat>
#include <QAudioSource>
#include <QFile>
#include <QAudioOutput>
#include "note.h"

namespace Ui
{
    class SingSub;
}

class SingSub : public QWidget
{
    Q_OBJECT

public:
    explicit SingSub(QWidget *parent = nullptr);
    ~SingSub();
    void setDifficulty(int level,Score x);
    void paintEvent(QPaintEvent *event);
private slots:
    void on_BackSing_clicked();

    void on_recording_clicked();

    void on_refresh_clicked();

    void on_answerplay_clicked();

    void on_rebegin_clicked();

    void updateRecordingState(); // 录音状态更新

signals:
    void back();

private:
    Ui::SingSub *ui;
    int difficultyLevel;

    // 录音相关
    QAudioSource *audioInput;
    QFile *audioFile;
    QString currentRecordingPath;
    bool isRecording;
    QAudioFormat fmt;

    // 播放相关
    QMediaPlayer *player;
    QAudioOutput *audioOutput;

    // 乐谱相关
    QDir yuepuDir;
    QFileInfoList yuepuFiles;
    int currentYuepuIndex;
    int imageCount;          // 乐谱图片总数
    QString yuepuFolderName; // 当前难度对应的乐谱文件夹名称

    // 分数相关
    int currentScore;
    Score testScore;
    Score audioScore;

    // 辅助函数
    void showYuepu();
    void clearRecording();
    void updateScore();
    void startRecording();                                        // 开始录音
    void stopRecording();                                         // 停止录音
    void startPlayback();                                         // 开始播放
    void stopPlayback();                                          // 停止播放
    void updateYuepuFolder();                                     // 更新乐谱文件夹
    void writeWavHeader(QFile *file, const QAudioFormat &format); // 写入WAV文件头
    void updateWavHeader(QFile *file);                            // 更新WAV文件头
    void scoreRecording();                                        // 对录音进行打分
    void saveScoreToFile();                                       // 存储成绩
};

#endif // SINGSUB_H
