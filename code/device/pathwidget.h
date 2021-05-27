#ifndef PATH_H
#define PATH_H

#include <QWidget>

namespace Ui {
    class path_widget;
}

struct path_data
{
    QString shop_1, shop_2;
    path_data(QString _shop_1, QString _shop_2) :
        shop_1(_shop_1), shop_2(_shop_2)
    {
    }
};

class path_widget : public QWidget
{
    Q_OBJECT

signals:
    void ButtonPressed();
public:
    path_data GetData() { return path_data(shop_1, shop_2); }
    explicit path_widget(QWidget *parent = 0);
    ~path_widget();
private slots:
void on_lineEdit_textEdited(const QString &arg1);

void on_lineEdit_1_textEdited(const QString &arg1);

void on_pushButton_released();

private:
    Ui::path_widget *ui;
    QString shop_1, shop_2;
};
#endif
