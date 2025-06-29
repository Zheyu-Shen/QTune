#include "log.h"
#include "ui_log.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QValueAxis>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLegend>
#include <QtCharts/QLegendMarker>

Log::Log(QWidget *parent)
    : QWidget(parent), ui(new Ui::Log)
{
    ui->setupUi(this);
    logPath = QCoreApplication::applicationDirPath() + "/log_file";
    QDir dir(logPath);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    // 设置返回按钮样式
    ui->BackHome->setStyleSheet("QPushButton {"
                                "    border: none;"
                                "    background-color: transparent;"         // 透明背景
                                "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                "}"
                                "QPushButton:hover {" // 悬浮时保持透明
                                "    background-color: transparent;"
                                "}");
    // connect(ui->resetButton, &QPushButton::clicked, this, &Log::on_resetButton_clicked);
}

Log::~Log()
{
    delete ui;
}

void Log::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/log1.png"));
}

void Log::on_BackHome_clicked()
{
    emit this->back();
}

// 每次显示窗口时，重新加载和显示日志
void Log::showEvent(QShowEvent *event)
{
    loadAndDisplayLogs();
    QWidget::showEvent(event);
}

// --- 核心逻辑实现 ---

// 主调度函数
void Log::loadAndDisplayLogs()
{
    // 1. 读取登录天数
    int loginDays = 0;
    if (readLoginData(loginDays))
    {
        ui->loginDaysLabel->setText(QString::number(loginDays));
    }
    else
    {
        ui->loginDaysLabel->setText("0");
    }

    // 2. 读取听音和视唱日志
    QMap<QDate, QMap<int, QList<int>>> listenData;
    readListenLog(listenData);

    QMap<QDate, QMap<int, QList<int>>> scoresData;
    readScoresLog(scoresData);

    // 3. 生成并显示图表
    setupPieCharts(listenData, scoresData);
    setupScoresTrendChart(scoresData);
    setupListenTrendChart(listenData);
}

// 重置按钮槽函数
void Log::on_resetButton_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认操作", "确定要重置所有练习数据吗？此操作不可撤销。",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        clearLogFiles();
        // 重新加载以显示空状态
        loadAndDisplayLogs();
    }
}

// 清空所有日志文件
void Log::clearLogFiles()
{
    QStringList filesToDelete;
    filesToDelete << logPath + "/date.txt"
                  << logPath + "/listen_log.txt"
                  << logPath + "/scores_log.txt";

    foreach (const QString &filePath, filesToDelete)
    {
        QFile file(filePath);
        if (file.exists())
        {
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            file.close();
        }
    }
    qDebug() << "Log files cleared.";
}

// --- 文件读取函数 ---

bool Log::readLoginData(int &days)
{
    QFile file(logPath + "/date.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open date.txt for reading";
        return false;
    }

    QTextStream in(&file);
    in.readLine(); // 跳过日期行
    if (!in.atEnd())
    {
        days = in.readLine().toInt();
        file.close();
        return true;
    }
    file.close();
    return false;
}

bool Log::readListenLog(QMap<QDate, QMap<int, QList<int>>> &listenData)
{
    QFile file(logPath + "/listen_log.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open listen_log.txt for reading";
        return false;
    }
    listenData.clear();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(", ");
        if (parts.size() == 3)
        {
            QDateTime dateTime = QDateTime::fromString(parts[0], "yyyy-MM-dd hh:mm:ss");
            int mode = parts[1].split(": ")[1].toInt();
            int result = parts[2].split(": ")[1].toInt();
            listenData[dateTime.date()][mode].append(result);
        }
    }
    file.close();
    return true;
}

bool Log::readScoresLog(QMap<QDate, QMap<int, QList<int>>> &scoresData)
{
    QFile file(logPath + "/scores_log.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Cannot open scores_log.txt for reading";
        return false;
    }
    scoresData.clear();
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList parts = line.split(", ");
        if (parts.size() == 3)
        {
            QDateTime dateTime = QDateTime::fromString(parts[0], "yyyy-MM-dd hh:mm:ss");
            int mode = parts[1].split(": ")[1].toInt();
            int score = parts[2].split(": ")[1].toInt();
            scoresData[dateTime.date()][mode].append(score);
        }
    }
    file.close();
    return true;
}

// --- 图表生成函数 ---

void Log::setupPieCharts(const QMap<QDate, QMap<int, QList<int>>> &listenData,
                         const QMap<QDate, QMap<int, QList<int>>> &scoresData)
{
    // 1. 计算视唱总平均分
    double totalScore = 0;
    int scoreCount = 0;
    for (auto const &dailyScores : scoresData.values())
    {
        for (auto const &modeScores : dailyScores.values())
        {
            for (int score : modeScores)
            {
                totalScore += score;
                scoreCount++;
            }
        }
    }
    double avgScore = (scoreCount > 0) ? (totalScore / scoreCount) : 0;

    // 2. 计算听音总正确率
    double totalCorrect = 0;
    int listenCount = 0;
    for (auto const &dailyResults : listenData.values())
    {
        for (auto const &modeResults : dailyResults.values())
        {
            for (int result : modeResults)
            {
                totalCorrect += result; // result is 1 for correct, 0 for incorrect
                listenCount++;
            }
        }
    }
    double avgAccuracy = (listenCount > 0) ? (totalCorrect / listenCount * 100.0) : 0;

    // 3. 创建视唱饼图
    QPieSeries *scoreSeries = new QPieSeries();
    scoreSeries->append("平均得分", avgScore)->setBrush(QColor("#7ED6A5")); // 清新蓝绿
    scoreSeries->append(" ", 100 - avgScore)->setBrush(QColor("#FBF1E3"));  // 更柔和的辅助色
    scoreSeries->setLabelsVisible(true);
    scoreSeries->slices().at(1)->setLabelVisible(false); // 隐藏辅助块的标签
    scoreSeries->slices().at(0)->setPen(Qt::NoPen);      // 去除主块边框
    scoreSeries->slices().at(1)->setPen(Qt::NoPen);      // 去除辅助块边框

    QChart *scoreChart = new QChart();
    scoreChart->addSeries(scoreSeries);
    scoreChart->setTitle(QString("视唱总平均分: %1").arg(QString::number(avgScore, 'f', 1)));
    scoreChart->legend()->hide();
    scoreChart->setAnimationOptions(QChart::AllAnimations);
    ui->scoresPieChartView->setChart(scoreChart);
    ui->scoresPieChartView->setRenderHint(QPainter::Antialiasing);

    // 4. 创建听音饼图
    QPieSeries *listenSeries = new QPieSeries();
    listenSeries->append("正确率", avgAccuracy)->setBrush(QColor("#7ED6A5"));  // 稍微暗淡的绿色
    listenSeries->append(" ", 100 - avgAccuracy)->setBrush(QColor("#FBF1E3")); // 更柔和的辅助色
    listenSeries->setLabelsVisible(true);
    listenSeries->slices().at(1)->setLabelVisible(false);
    listenSeries->slices().at(0)->setPen(Qt::NoPen); // 去除主块边框
    listenSeries->slices().at(1)->setPen(Qt::NoPen); // 去除辅助块边框

    QChart *listenChart = new QChart();
    listenChart->addSeries(listenSeries);
    listenChart->setTitle(QString("听音总正确率: %1%").arg(QString::number(avgAccuracy, 'f', 1)));
    listenChart->legend()->hide();
    listenChart->setAnimationOptions(QChart::AllAnimations);
    ui->listenPieChartView->setChart(listenChart);
    ui->listenPieChartView->setRenderHint(QPainter::Antialiasing);
}

void Log::setupScoresTrendChart(const QMap<QDate, QMap<int, QList<int>>> &scoresData)
{
    // --- 数据准备 ---
    QMap<int, QLineSeries *> lineSeriesMap;
    QMap<int, QBarSet *> barSetMap;
    QStringList categories;
    QDate today = QDate::currentDate();

    // 初始化3个模式的线和柱状图集
    for (int mode = 1; mode <= 3; ++mode)
    {
        lineSeriesMap[mode] = new QLineSeries();
        lineSeriesMap[mode]->setName(QString("难度%1").arg(mode));
        barSetMap[mode] = new QBarSet("");
    }

    // 遍历过去7天
    for (int i = 6; i >= 0; --i)
    {
        QDate date = today.addDays(-i);
        categories.append(date.toString("MM-dd"));
        QMap<int, DailyStat> dailyStats;

        if (scoresData.contains(date))
        {
            const auto &dailyData = scoresData[date];
            for (int mode = 1; mode <= 3; ++mode)
            {
                if (dailyData.contains(mode))
                {
                    const QList<int> &scores = dailyData[mode];
                    double sum = 0;
                    for (int score : scores)
                        sum += score;
                    dailyStats[mode].average = scores.size() > 0 ? sum / scores.size() : 0.0;
                    dailyStats[mode].count = scores.size();
                }
            }
        }
        // 为每个模式追加数据点
        for (int mode = 1; mode <= 3; ++mode)
        {
            lineSeriesMap[mode]->append(6 - i, dailyStats[mode].average);
            *barSetMap[mode] << dailyStats[mode].count;
        }
    }

    // --- 图表配置 ---
    QChart *chart = new QChart();
    chart->setTitle("近7日视唱练习趋势");
    chart->setAnimationOptions(QChart::AllAnimations);

    // 添加线系列
    for (auto series : lineSeriesMap.values())
        chart->addSeries(series);

    // 添加柱状图系列
    QBarSeries *barSeries = new QBarSeries();
    for (auto set : barSetMap.values())
        barSeries->append(set);
    chart->addSeries(barSeries);

    // X轴
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    for (auto series : lineSeriesMap.values())
        series->attachAxis(axisX);
    barSeries->attachAxis(axisX);

    // 左Y轴 (分数)
    QValueAxis *axisY_line = new QValueAxis();
    axisY_line->setRange(0, 100);
    axisY_line->setTitleText("平均分");
    chart->addAxis(axisY_line, Qt::AlignLeft);
    for (auto series : lineSeriesMap.values())
        series->attachAxis(axisY_line);

    // 右Y轴 (次数)
    QValueAxis *axisY_bar = new QValueAxis();
    axisY_bar->setTitleText("练习次数");
    chart->addAxis(axisY_bar, Qt::AlignRight);
    barSeries->attachAxis(axisY_bar);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    const auto markers = chart->legend()->markers(barSeries);
    // 2. 遍历这些标记，并将它们设置为不可见
    for (QLegendMarker *marker : markers)
    {
        marker->setVisible(false);
    }

    ui->scoresTrendChartView->setChart(chart);
    ui->scoresTrendChartView->setRenderHint(QPainter::Antialiasing);
}

void Log::setupListenTrendChart(const QMap<QDate, QMap<int, QList<int>>> &listenData)
{
    // 此函数逻辑与 setupScoresTrendChart 非常相似，只是数据源和Y轴标签不同

    // --- 数据准备 ---
    QMap<int, QLineSeries *> lineSeriesMap;
    QMap<int, QBarSet *> barSetMap;
    QStringList categories;
    QDate today = QDate::currentDate();

    // 初始化4个模式
    QStringList modeNames;
    modeNames << "单音" << "双音" << "三音" << "七音";
    for (int mode = 1; mode <= 4; ++mode)
    {
        lineSeriesMap[mode] = new QLineSeries();
        lineSeriesMap[mode]->setName(QString("%1").arg(modeNames[mode - 1]));
        barSetMap[mode] = new QBarSet("");
    }

    for (int i = 6; i >= 0; --i)
    {
        QDate date = today.addDays(-i);
        categories.append(date.toString("MM-dd"));
        QMap<int, DailyStat> dailyStats;

        if (listenData.contains(date))
        {
            const auto &dailyData = listenData[date];
            for (int mode = 1; mode <= 4; ++mode)
            {
                if (dailyData.contains(mode))
                {
                    const QList<int> &results = dailyData[mode];
                    double correct = 0;
                    for (int res : results)
                        correct += res;
                    dailyStats[mode].average = results.size() > 0 ? (correct / results.size()) * 100.0 : 0.0; // 转换为百分比
                    dailyStats[mode].count = results.size();
                }
            }
        }
        for (int mode = 1; mode <= 4; ++mode)
        {
            lineSeriesMap[mode]->append(6 - i, dailyStats[mode].average);
            *barSetMap[mode] << dailyStats[mode].count;
        }
    }

    // --- 图表配置 ---
    QChart *chart = new QChart();
    chart->setTitle("近7日听音练习趋势");
    chart->setAnimationOptions(QChart::AllAnimations);

    for (auto series : lineSeriesMap.values())
        chart->addSeries(series);
    QBarSeries *barSeries = new QBarSeries();
    for (auto set : barSetMap.values())
        barSeries->append(set);
    chart->addSeries(barSeries);

    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    for (auto series : lineSeriesMap.values())
        series->attachAxis(axisX);
    barSeries->attachAxis(axisX);

    QValueAxis *axisY_line = new QValueAxis();
    axisY_line->setRange(0, 100);
    axisY_line->setTitleText("正确率 (%)");
    chart->addAxis(axisY_line, Qt::AlignLeft);
    for (auto series : lineSeriesMap.values())
        series->attachAxis(axisY_line);

    QValueAxis *axisY_bar = new QValueAxis();
    axisY_bar->setTitleText("练习次数");
    chart->addAxis(axisY_bar, Qt::AlignRight);
    barSeries->attachAxis(axisY_bar);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    const auto markers = chart->legend()->markers(barSeries);
    for (QLegendMarker *marker : markers)
    {
        marker->setVisible(false);
    }

    ui->listenTrendChartView->setChart(chart);
    ui->listenTrendChartView->setRenderHint(QPainter::Antialiasing);
}
