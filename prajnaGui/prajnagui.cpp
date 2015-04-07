#include "prajnagui.h"
#include "ui_prajnagui.h"

PrajnaGui::PrajnaGui(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PrajnaGui)
{
    ui->setupUi(this);
}

PrajnaGui::~PrajnaGui()
{
    delete ui;
}
