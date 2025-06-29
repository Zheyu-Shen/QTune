#include "aiassistant.h"
#include "ui_aiassistant.h"

#include <QCoreApplication>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSettings>
#include <QCloseEvent>
#include <QFile>
#include <QTextStream>
#include <QDebug>

AIAssistant::AIAssistant(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AIAssistant),
    m_currentReply(nullptr)
{
    ui->setupUi(this);

    // 初始化网络管理器
    m_networkManager = new QNetworkAccessManager(this);

    // 从配置文件加载 API Key
    loadApiKey();

    // 为 AI 添加一个初始的系统级提示，让它的回答更专业
    on_refreshButton_clicked();
}

void AIAssistant::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/ai1.png"));
}

AIAssistant::~AIAssistant()
{
    // 确保在析构时，如果还有网络请求在进行，则中止它
    if(m_currentReply && m_currentReply->isRunning()) {
        m_currentReply->abort();
    }
    delete ui;
}

// --- UI 按钮处理 ---

void AIAssistant::on_backButton_clicked()
{
    saveApiKey(); // 返回前保存一下 Key
    emit back(); // 发送返回信号 (根据你的代码，信号名为 back)
}

void AIAssistant::on_refreshButton_clicked()
{
    ui->resultTextEdit->clear();
    m_chatHistory = QJsonArray(); // 清空对话历史

    // 添加一个系统提示，引导 AI 的角色
    // 这会让 AI 的回答更贴近你的软件主题
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个专业的音乐理论和视唱练耳助手。请用中文回答用户在学习过程中遇到的问题。";
    m_chatHistory.append(systemMessage);

    ui->resultTextEdit->setPlaceholderText("你好，我是你的音乐学习助手，有什么可以帮你的吗？");
}

void AIAssistant::on_sendButton_clicked()
{
    QString apiKey = ui->apiKeyLineEdit->text().trimmed();
    QString userInput = ui->promptTextEdit->toPlainText().trimmed();

    if (apiKey.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入您的 DeepSeek API Key。");
        return;
    }
    if (userInput.isEmpty()) {
        QMessageBox::warning(this, "提示", "请输入您的问题。");
        return;
    }

    // --- 1. 准备网络请求 ---
    const QString apiUrl = "https://api.deepseek.com/chat/completions";
    QNetworkRequest request(apiUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QByteArray("Bearer ") + apiKey.toUtf8());

    // --- 2. 构建 JSON 请求体 ---
    // 将用户的当前输入添加到对话历史中
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = userInput;
    m_chatHistory.append(userMessage);

    // 创建请求体
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";
    requestBody["messages"] = m_chatHistory;
    requestBody["stream"] = true; // 关键：开启流式响应

    QJsonDocument doc(requestBody);
    QByteArray postData = doc.toJson();

    // --- 3. 发送请求并连接信号 ---
    m_currentReply = m_networkManager->post(request, postData);

    // 连接信号以处理流式数据和完成事件
    connect(m_currentReply, &QNetworkReply::readyRead, this, &AIAssistant::onReplyReadyRead);
    connect(m_currentReply, &QNetworkReply::finished, this, &AIAssistant::onReplyFinished);

    // --- 4. 更新 UI ---
    ui->sendButton->setEnabled(false);
    ui->promptTextEdit->clear();
    ui->resultTextEdit->append("\n\n**我:**\n" + userInput + "\n\n**AI助手:**\n");
}

// --- 网络响应处理 ---

void AIAssistant::onReplyReadyRead()
{
    if (!m_currentReply) return;

    // 读取所有新到达的数据
    QByteArray data = m_currentReply->readAll();
    QString strData = QString::fromUtf8(data);

    // SSE (Server-Sent Events) 的格式是 "data: {...}\n\n"
    // 一次 readyRead 可能包含多个 data 块
    QStringList lines = strData.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        if (line.startsWith("data: ")) {
            QString jsonData = line.mid(6).trimmed();
            if (jsonData == "[DONE]") {
                // 流结束的标志
                return;
            }

            QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                QString deltaContent = obj["choices"].toArray().at(0).toObject()["delta"].toObject()["content"].toString();

                // 将增量内容追加到文本框
                ui->resultTextEdit->moveCursor(QTextCursor::End);
                ui->resultTextEdit->insertPlainText(deltaContent);
            }
        }
    }
}

void AIAssistant::onReplyFinished()
{
    if (!m_currentReply) return;

    // 检查是否有网络错误
    if (m_currentReply->error() != QNetworkReply::NoError) {
        QString errorMsg = "网络错误: " + m_currentReply->errorString();
        // 尝试读取API返回的错误信息
        QByteArray errorData = m_currentReply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(errorData);
        if(doc.isObject()){
            errorMsg += "\nAPI Message: " + doc.object()["error"].toObject()["message"].toString();
        }
        ui->resultTextEdit->append("\n<font color='red'>" + errorMsg + "</font>");
    } else {
        // 流式响应完成后，需要将完整的 AI 回答存入历史记录
        QString fullResponse;
        // 简单地从 QTextEdit 获取最后一次的回答
        QString allText = ui->resultTextEdit->toPlainText();
        int lastAssistantPos = allText.lastIndexOf("**AI助手:**\n");
        if (lastAssistantPos != -1) {
            fullResponse = allText.mid(lastAssistantPos + 11); // 11 is the length of "**AI助手:**\n"
        }

        if (!fullResponse.isEmpty()) {
            QJsonObject assistantMessage;
            assistantMessage["role"] = "assistant";
            assistantMessage["content"] = fullResponse.trimmed();
            m_chatHistory.append(assistantMessage);
        }
    }

    // 清理工作
    ui->sendButton->setEnabled(true);
    m_currentReply->deleteLater(); // 必须调用，防止内存泄漏
    m_currentReply = nullptr;
}

// --- API Key 持久化 (修改后版本) ---

void AIAssistant::saveApiKey()
{
    // 1. 定义配置文件的路径，位于程序可执行文件同目录下
    QString configFilePath = QCoreApplication::applicationDirPath() + "/config.txt";
    QFile file(configFilePath);

    // 2. 尝试打开文件进行写入。
    // WriteOnly: 只写模式
    // Truncate: 如果文件已存在，清空其内容
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        // 如果打开失败（比如因为权限问题），打印一个警告信息
        qWarning() << "无法打开 config.txt 文件进行写入: " << file.errorString();
        return;
    }

    // 3. 获取、编码 API Key
    QString apiKey = ui->apiKeyLineEdit->text();
    QByteArray encodedApiKey = apiKey.toUtf8().toBase64();

    // 4. 创建一个文本流，并将编码后的 Key 写入文件
    QTextStream out(&file);
    out << QString(encodedApiKey);

    // 5. 文件在 file 对象析构时会自动关闭，无需手动调用 file.close()
}

void AIAssistant::loadApiKey()
{
    // 1. 定义配置文件的路径
    QString configFilePath = QCoreApplication::applicationDirPath() + "/config.txt";
    QFile file(configFilePath);

    // 2. 首先检查文件是否存在，如果不存在（比如首次运行），就直接返回
    if (!file.exists()) {
        return;
    }

    // 3. 尝试打开文件进行读取
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开 config.txt 文件进行读取: " << file.errorString();
        return;
    }

    // 4. 创建一个文本流，并读取文件的全部内容
    QTextStream in(&file);
    QString encodedApiKey = in.readAll().trimmed(); // 使用 trimmed() 以防万一有空白字符

    // 5. 如果读取到的内容不为空，则解码并设置到输入框
    if (!encodedApiKey.isEmpty()) {
        QByteArray decodedApiKey_bytes = QByteArray::fromBase64(encodedApiKey.toUtf8());
        ui->apiKeyLineEdit->setText(QString(decodedApiKey_bytes));
    }

    // 6. 文件同样会自动关闭
}

void AIAssistant::closeEvent(QCloseEvent *event)
{
    saveApiKey(); // 在关闭窗口（例如点击右上角X）时保存Key
    QWidget::closeEvent(event);
}
