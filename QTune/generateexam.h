#ifndef GENERATEEXAM_H
#define GENERATEEXAM_H

#include <QWidget>

namespace Ui {
class GenerateExam;
}

class GenerateExam : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateExam(QWidget *parent = nullptr);
    ~GenerateExam();

private:
    Ui::GenerateExam *ui;
};

#endif // GENERATEEXAM_H 