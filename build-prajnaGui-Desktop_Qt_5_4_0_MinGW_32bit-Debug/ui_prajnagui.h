/********************************************************************************
** Form generated from reading UI file 'prajnagui.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PRAJNAGUI_H
#define UI_PRAJNAGUI_H

#include <QtCore/QVariant>
#include <QtWebKitWidgets/QWebView>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PrajnaGui
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QTabWidget *tabWidget;
    QWidget *tabTaskList;
    QVBoxLayout *verticalLayout_2;
    QVBoxLayout *verticalLayout;
    QLineEdit *lineEditTaskSearch;
    QTableView *tableViewTask;
    QWidget *tabTaskAdd;
    QWidget *tabTaskRuning;
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout_3;
    QLineEdit *lineEditTaskRuning;
    QTableView *tableViewTaskRuning;
    QWidget *tabTaskHistory;
    QGridLayout *gridLayout_3;
    QVBoxLayout *verticalLayout_4;
    QLineEdit *lineEditTaskHistory;
    QTableView *tableViewTaskHistory;
    QWidget *tabMachineManage;
    QWidget *tabGlobalSetting;
    QWidget *tabHelp;
    QGridLayout *gridLayout_4;
    QWebView *webViewHelp;

    void setupUi(QMainWindow *PrajnaGui)
    {
        if (PrajnaGui->objectName().isEmpty())
            PrajnaGui->setObjectName(QStringLiteral("PrajnaGui"));
        PrajnaGui->resize(1327, 805);
        centralWidget = new QWidget(PrajnaGui);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        tabWidget = new QTabWidget(centralWidget);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QFont font;
        font.setFamily(QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221 Light"));
        tabWidget->setFont(font);
        tabTaskList = new QWidget();
        tabTaskList->setObjectName(QStringLiteral("tabTaskList"));
        tabTaskList->setFont(font);
        verticalLayout_2 = new QVBoxLayout(tabTaskList);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setContentsMargins(11, 11, 11, 11);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(6);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        lineEditTaskSearch = new QLineEdit(tabTaskList);
        lineEditTaskSearch->setObjectName(QStringLiteral("lineEditTaskSearch"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEditTaskSearch->sizePolicy().hasHeightForWidth());
        lineEditTaskSearch->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setItalic(true);
        lineEditTaskSearch->setFont(font1);

        verticalLayout->addWidget(lineEditTaskSearch);

        tableViewTask = new QTableView(tabTaskList);
        tableViewTask->setObjectName(QStringLiteral("tableViewTask"));

        verticalLayout->addWidget(tableViewTask);


        verticalLayout_2->addLayout(verticalLayout);

        tabWidget->addTab(tabTaskList, QString());
        tabTaskAdd = new QWidget();
        tabTaskAdd->setObjectName(QStringLiteral("tabTaskAdd"));
        tabTaskAdd->setFont(font);
        tabWidget->addTab(tabTaskAdd, QString());
        tabTaskRuning = new QWidget();
        tabTaskRuning->setObjectName(QStringLiteral("tabTaskRuning"));
        gridLayout_2 = new QGridLayout(tabTaskRuning);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setObjectName(QStringLiteral("verticalLayout_3"));
        lineEditTaskRuning = new QLineEdit(tabTaskRuning);
        lineEditTaskRuning->setObjectName(QStringLiteral("lineEditTaskRuning"));

        verticalLayout_3->addWidget(lineEditTaskRuning);

        tableViewTaskRuning = new QTableView(tabTaskRuning);
        tableViewTaskRuning->setObjectName(QStringLiteral("tableViewTaskRuning"));

        verticalLayout_3->addWidget(tableViewTaskRuning);


        gridLayout_2->addLayout(verticalLayout_3, 0, 0, 1, 1);

        tabWidget->addTab(tabTaskRuning, QString());
        tabTaskHistory = new QWidget();
        tabTaskHistory->setObjectName(QStringLiteral("tabTaskHistory"));
        gridLayout_3 = new QGridLayout(tabTaskHistory);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setObjectName(QStringLiteral("verticalLayout_4"));
        lineEditTaskHistory = new QLineEdit(tabTaskHistory);
        lineEditTaskHistory->setObjectName(QStringLiteral("lineEditTaskHistory"));

        verticalLayout_4->addWidget(lineEditTaskHistory);

        tableViewTaskHistory = new QTableView(tabTaskHistory);
        tableViewTaskHistory->setObjectName(QStringLiteral("tableViewTaskHistory"));

        verticalLayout_4->addWidget(tableViewTaskHistory);


        gridLayout_3->addLayout(verticalLayout_4, 0, 0, 1, 1);

        tabWidget->addTab(tabTaskHistory, QString());
        tabMachineManage = new QWidget();
        tabMachineManage->setObjectName(QStringLiteral("tabMachineManage"));
        tabWidget->addTab(tabMachineManage, QString());
        tabGlobalSetting = new QWidget();
        tabGlobalSetting->setObjectName(QStringLiteral("tabGlobalSetting"));
        tabWidget->addTab(tabGlobalSetting, QString());
        tabHelp = new QWidget();
        tabHelp->setObjectName(QStringLiteral("tabHelp"));
        gridLayout_4 = new QGridLayout(tabHelp);
        gridLayout_4->setSpacing(6);
        gridLayout_4->setContentsMargins(11, 11, 11, 11);
        gridLayout_4->setObjectName(QStringLiteral("gridLayout_4"));
        webViewHelp = new QWebView(tabHelp);
        webViewHelp->setObjectName(QStringLiteral("webViewHelp"));
        webViewHelp->setUrl(QUrl(QStringLiteral("about:blank")));

        gridLayout_4->addWidget(webViewHelp, 0, 0, 1, 1);

        tabWidget->addTab(tabHelp, QString());

        gridLayout->addWidget(tabWidget, 0, 0, 1, 1);

        PrajnaGui->setCentralWidget(centralWidget);

        retranslateUi(PrajnaGui);

        tabWidget->setCurrentIndex(6);


        QMetaObject::connectSlotsByName(PrajnaGui);
    } // setupUi

    void retranslateUi(QMainWindow *PrajnaGui)
    {
        PrajnaGui->setWindowTitle(QApplication::translate("PrajnaGui", "PrajnaGui", 0));
        lineEditTaskSearch->setPlaceholderText(QApplication::translate("PrajnaGui", "\344\273\273\345\212\241\346\220\234\347\264\242\357\274\214\350\241\250\345\220\215 task\357\274\214\345\217\257\344\273\245\347\233\264\346\216\245\344\275\277\347\224\250sql\346\220\234\347\264\242:", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabTaskList), QApplication::translate("PrajnaGui", "\344\273\273\345\212\241\345\210\227\350\241\250", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabTaskAdd), QApplication::translate("PrajnaGui", "\344\273\273\345\212\241\346\267\273\345\212\240", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabTaskRuning), QApplication::translate("PrajnaGui", "\346\255\243\345\234\250\350\277\220\350\241\214\347\232\204\344\273\273\345\212\241", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabTaskHistory), QApplication::translate("PrajnaGui", "\345\216\206\345\217\262\344\273\273\345\212\241", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabMachineManage), QApplication::translate("PrajnaGui", "\346\234\272\345\231\250\347\256\241\347\220\206", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabGlobalSetting), QApplication::translate("PrajnaGui", "\345\205\250\345\261\200\350\256\276\347\275\256", 0));
        tabWidget->setTabText(tabWidget->indexOf(tabHelp), QApplication::translate("PrajnaGui", "\345\270\256\345\212\251\344\277\241\346\201\257", 0));
    } // retranslateUi

};

namespace Ui {
    class PrajnaGui: public Ui_PrajnaGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PRAJNAGUI_H
