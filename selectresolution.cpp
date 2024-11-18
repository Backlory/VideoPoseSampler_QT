#include "selectresolution.h"
#include "ui_selectresolution.h"

selectResolution::selectResolution(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::selectResolution)
{
    ui->setupUi(this);
}

selectResolution::~selectResolution()
{
    delete ui;
}

int selectResolution::getResolution(int &width, int &height) {
    auto text = ui->cB_Resolution->currentText();
    width = text.split("×")[0].toInt();
    height = text.split("×")[1].toInt();
    return 0;
}

