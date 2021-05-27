#include "pathwidget.h"
#include "ui_path_widget.h"

path_widget::path_widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::path_widget)
{
    setMinimumWidth(250);
    ui->setupUi(this);
}
path_widget::~path_widget()
{
    delete ui;
}

void path_widget::on_lineEdit_textEdited(const QString &arg1)
{
    shop_1 = arg1;
}

void path_widget::on_lineEdit_1_textEdited(const QString &arg1)
{
    shop_2 = arg1;
}

void path_widget::on_pushButton_released()
{
    int a;
    a++;
    emit ButtonPressed();
}
