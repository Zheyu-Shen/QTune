#ifndef HOMEPAGE_H
#define HOMEPAGE_H

#include <QWidget>
#include "sing.h"
#include "listen.h"
#include "write.h"
#include "log.h"
#include "aiassistant.h"
#include <QPainter>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class Homepage;
}
QT_END_NAMESPACE

class Homepage : public QWidget
{
    Q_OBJECT

public:
    Homepage(QWidget *parent = nullptr);
    ~Homepage();

private slots:
    void on_OpenSing_clicked();

    void on_OpenListen_clicked();

    void on_OpenWrite_clicked();

    void on_OpenLog_clicked();

    void on_OpenAIAssistant_clicked();

private:
    Ui::Homepage *ui;
    Sing *sing; // 保存下一级对象的实例化地址
    Listen *listen;
    Write *write;
    Log *log;
    AIAssistant *aiAssistant;

    void paintEvent(QPaintEvent *event);
    void updateConsecutiveDays();
};

#endif // HOMEPAGE_H
