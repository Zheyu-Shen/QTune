#ifndef AIASSISTANT_H
#define AIASSISTANT_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QPainter>
namespace Ui {
class AIAssistant;
}

class AIAssistant : public QWidget
{
    Q_OBJECT

public:
    explicit AIAssistant(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);
    ~AIAssistant();

signals:
    void back(); // 定义一个返回主页的信号

private slots:
    // UI 控件的槽函数
    void on_sendButton_clicked();
    void on_backButton_clicked();
    void on_refreshButton_clicked();

    // 网络处理的槽函数
    void onReplyReadyRead();
    void onReplyFinished();

private:
    Ui::AIAssistant *ui;
    QNetworkAccessManager *m_networkManager;
    QNetworkReply *m_currentReply; // 保存当前的网络应答对象

    // 用于保存对话历史
    QJsonArray m_chatHistory;

    // 保存和加载 API Key
    void saveApiKey();
    void loadApiKey();

protected:
    // 重写 closeEvent 以便在关闭窗口时保存 API Key
    void closeEvent(QCloseEvent *event) override;
};

#endif // AIASSISTANT_H
