#ifndef PRAJNAGUI_H
#define PRAJNAGUI_H

#include <QWebView>
#include <QMainWindow>

namespace Ui {
class PrajnaGui;
}

class PrajnaGui : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrajnaGui(QWidget *parent = 0);
    ~PrajnaGui();

private:
    Ui::PrajnaGui *ui;
};

#endif // PRAJNAGUI_H
