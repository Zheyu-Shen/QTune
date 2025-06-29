#ifndef WRITESUB_H
#define WRITESUB_H

#include <QWidget>
#include <QImage>
#include <QPainter>
namespace Ui
{
    class WriteSub;
}

class WriteSub : public QWidget
{
    Q_OBJECT

public:
    explicit WriteSub(QWidget *parent = nullptr);
    ~WriteSub();

    void setResultImage(const QImage &image);
    void paintEvent(QPaintEvent * event);

private slots:
    void on_download_clicked();
    void on_BackWriteSub_clicked();

signals:
    void back();

private:
    Ui::WriteSub *ui;
    QImage resultImage;
};

#endif // WRITESUB_H
