#ifndef LISTENSUBSUB_H
#define LISTENSUBSUB_H

#include <QWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDir>
#include <QFileInfoList>
#include <QRandomGenerator>
#include <QTextStream>
#include <QFile>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>

namespace Ui
{
    class ListenSubSub;
}

class ListenSubSub : public QWidget
{
    Q_OBJECT

public:
    explicit ListenSubSub(QWidget *parent = nullptr);
    ~ListenSubSub();
    void setsubMode(int modenum);
    void paintEvent(QPaintEvent * event);

signals:
    void back();

private slots:
    void on_play_clicked();
    void on_refresh_clicked();


    void on_BackListenSub_clicked();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    Ui::ListenSubSub *ui;
    int mode;
    QString answer;

    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QFileInfoList mp3Files;
    int currentMp3Index;
    int mp3Count;

    void loadMp3Files();
    QString getAnswerFromFile();
    bool compareAnswer(const QString &userInput, const QString &correctAnswer);
    void checkAnswer();
    void logListenEvent(bool isCorrect);
};

#endif // LISTENSUBSUB_H
