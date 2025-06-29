#ifndef SING_H
#define SING_H

#include <QWidget>
#include "singsub.h"

namespace Ui {
class Sing;
}

class Sing : public QWidget
{
    Q_OBJECT

public:
    explicit Sing(QWidget *parent = nullptr);
    ~Sing();
    void paintEvent(QPaintEvent * event);

private slots:
    void on_BackHome_clicked();

    void on_easybtn_clicked();

    void on_mediumbtn_clicked();

    void on_hardbtn_clicked();

signals:
    void back();

private:
    Ui::Sing *ui;
    SingSub *singsub;
};


#endif // SING_H
