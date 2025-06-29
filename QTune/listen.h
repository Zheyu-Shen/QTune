#ifndef LISTEN_H
#define LISTEN_H

#include <QWidget>
#include "listensub.h"

namespace Ui {
class Listen;
}

class Listen : public QWidget
{
    Q_OBJECT

public:
    explicit Listen(QWidget *parent = nullptr);
    ~Listen();
    void paintEvent(QPaintEvent * event);
signals:
    void back();

private slots:
    void on_BackHome_clicked();

    void on_danyin_clicked();

    void on_qiyin_clicked();

    void on_shuangyin_clicked();

    void on_sanyin_clicked();

private:
    Ui::Listen *ui;
    ListenSub *listensub;
};

#endif // LISTEN_H
