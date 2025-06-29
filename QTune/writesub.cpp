#include "writesub.h"
#include "ui_writesub.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QImage>
#include <QPainter>
WriteSub::WriteSub(QWidget *parent)
    : QWidget(parent), ui(new Ui::WriteSub)
{
    ui->setupUi(this);
    // 隐藏scrollArea的水平滚动条
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 设置返回按钮样式
    ui->BackWriteSub->setStyleSheet("QPushButton {"
                                    "    border: none;"
                                    "    background-color: transparent;"         // 透明背景
                                    "    border-image: url(:/MyPicture/rt.png);" // 使用border-image
                                    "}"
                                    "QPushButton:hover {" // 悬浮时保持透明
                                    "    background-color: transparent;"
                                    "}");
}

WriteSub::~WriteSub()
{
    delete ui;
}

void WriteSub::setResultImage(const QImage &image)
{
    resultImage = image;
    QPixmap pixmap = QPixmap::fromImage(image);
    if (pixmap.isNull())
    {
        ui->resultLabel->setText("无法加载乐谱图片");
        return;
    }

    // 获取标签宽度
    int labelWidth = ui->resultLabel->width();

    // 计算等比例缩放后的高度
    int scaledHeight = (pixmap.height() * labelWidth) / pixmap.width();

    // 缩放图片到标签宽度，保持宽高比
    pixmap = pixmap.scaled(labelWidth, scaledHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 设置图片
    ui->resultLabel->setPixmap(pixmap);
    ui->resultLabel->setScaledContents(false);
    ui->resultLabel->setFixedWidth(labelWidth);
    ui->resultLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft); // 设置图片顶部左对齐
}

void WriteSub::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, width(), height(), QPixmap(":/MyPicture/ws1.png"));
}

void WriteSub::on_download_clicked()
{
    if (resultImage.isNull())
    {
        QMessageBox::warning(this, "警告", "没有可保存的图片");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "保存图片",
                                                    QDir::homePath(),
                                                    "图片文件 (*.png *.jpg *.bmp)");

    if (!fileName.isEmpty())
    {
        if (resultImage.save(fileName))
        {
            QMessageBox::information(this, "成功", "图片已保存");
        }
        else
        {
            QMessageBox::critical(this, "错误", "保存图片失败");
        }
    }
}

void WriteSub::on_BackWriteSub_clicked()
{
    emit back();
}
