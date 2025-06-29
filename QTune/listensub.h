#ifndef LISTENSUB_H
#define LISTENSUB_H

#include <QWidget>
#include <QDir>
#include <QFileInfoList>
#include <QImage>
#include <QPixmap>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "listensubsub.h"
#include <QPainter>

namespace Ui
{
    class ListenSub;
}

class ListenSub : public QWidget
{
    Q_OBJECT

public:
    explicit ListenSub(QWidget *parent = nullptr);
    ~ListenSub();
    void setMode(int modenum);
    void paintEvent(QPaintEvent * event);


signals:
    void back();

private slots:
    void on_BackListen_clicked();

    void on_download_clicked();

    void on_play_clicked();

    void on_refresh_clicked();

    void on_test_clicked();

private:
    Ui::ListenSub *ui;
    ListenSubSub *listensubsub;
    int mode;

    // 乐谱相关
    QDir yuepuDir;
    QFileInfoList yuepuFiles;
    int currentYuepuIndex;
    int imageCount;          // 乐谱图片总数
    QString yuepuFolderName; // 当前难度对应的乐谱文件夹名称

    // 播放器相关
    QMediaPlayer *player;
    QAudioOutput *audioOutput;

    // 辅助函数
    void showYuepu();
};

#endif // LISTENSUB_H
