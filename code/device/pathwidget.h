#ifndef PATH_H
#define PATH_H

#include <QWidget>

namespace Ui {
    class path_widget;
}

struct path_data
{
    int floor1, floor2;
    float x1, x2, y1, y2;
    path_data(QString Floor1, QString Floor2, QString X1, QString X2, QString Y1, QString Y2) :
        floor1(Floor1.toInt()), floor2(Floor2.toInt()), x1(X1.toFloat()), x2(X2.toFloat()), y1(Y1.toFloat()), y2(Y2.toFloat())
    {
    }
};

class path_widget : public QWidget
{
    Q_OBJECT

signals:
    void ButtonPressed();
public:
    path_data GetData() { return path_data(Floor1, Floor2, X1, X2, Y1, Y2); }
    explicit path_widget(QWidget *parent = 0);
    ~path_widget();
private slots:
void on_lineEdit_textEdited(const QString &arg1);

void on_lineEdit_4_textEdited(const QString &arg1);

void on_lineEdit_2_textEdited(const QString &arg1);

void on_lineEdit_3_textEdited(const QString &arg1);

void on_lineEdit_5_textEdited(const QString &arg1);

void on_lineEdit_6_textEdited(const QString &arg1);

void on_pushButton_released();

private:
    Ui::path_widget *ui;
    QString Floor1, Floor2, X1, X2, Y1, Y2;
};
#endif
