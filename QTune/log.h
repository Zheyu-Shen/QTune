#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <QPainter>
#include <QDate>
#include <QMap>
#include <QList>

QT_BEGIN_NAMESPACE
namespace QtCharts {
class QChartView;
class QChart;
}
QT_END_NAMESPACE


namespace Ui {
class Log;
}

struct DailyStat {
    double average = 0.0; // 平均分或准确率
    int count = 0;       // 练习次数
};


class Log : public QWidget
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr);
    ~Log();
    void paintEvent(QPaintEvent * event) override;
    void showEvent(QShowEvent *event) override;
signals:
    void back();

private slots:
    void on_BackHome_clicked();
    void on_resetButton_clicked();

private:
    Ui::Log *ui;
    QString logPath; // 日志文件目录
    // --- 数据处理函数 ---
    void loadAndDisplayLogs();
    void clearLogFiles();
    bool readLoginData(int& days);
    bool readListenLog(QMap<QDate, QMap<int, QList<int>>>& listenData);
    bool readScoresLog(QMap<QDate, QMap<int, QList<int>>>& scoresData);

    // --- 图表生成函数 ---
    void setupPieCharts(const QMap<QDate, QMap<int, QList<int>>>& listenData,
                        const QMap<QDate, QMap<int, QList<int>>>& scoresData);

    void setupScoresTrendChart(const QMap<QDate, QMap<int, QList<int>>>& scoresData);
    void setupListenTrendChart(const QMap<QDate, QMap<int, QList<int>>>& listenData);
};

#endif // LOG_H
