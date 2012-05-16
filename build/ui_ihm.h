/********************************************************************************
** Form generated from reading UI file 'ihm.ui'
**
** Created: Wed May 9 23:08:31 2012
**      by: Qt User Interface Compiler version 4.7.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_IHM_H
#define UI_IHM_H

#include <QtCore/QVariant>
#include <QtDeclarative/QDeclarativeView>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QTextEdit>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Ihm
{
public:
    QAction *actionQuit;
    QAction *actionNewGame;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QDeclarativeView *canvas;
    QTextEdit *textEdit;
    QMenuBar *menubar;
    QMenu *menuGame;
    QMenu *menuRules;
    QMenu *menuTime;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *Ihm)
    {
        if (Ihm->objectName().isEmpty())
            Ihm->setObjectName(QString::fromUtf8("Ihm"));
        Ihm->resize(778, 900);
        Ihm->setMinimumSize(QSize(760, 900));
        Ihm->setMaximumSize(QSize(778, 900));
        actionQuit = new QAction(Ihm);
        actionQuit->setObjectName(QString::fromUtf8("actionQuit"));
        actionNewGame = new QAction(Ihm);
        actionNewGame->setObjectName(QString::fromUtf8("actionNewGame"));
        centralwidget = new QWidget(Ihm);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        canvas = new QDeclarativeView(centralwidget);
        canvas->setObjectName(QString::fromUtf8("canvas"));
        canvas->setMinimumSize(QSize(760, 760));
        canvas->setMaximumSize(QSize(760, 760));

        gridLayout->addWidget(canvas, 0, 0, 1, 1);

        textEdit = new QTextEdit(centralwidget);
        textEdit->setObjectName(QString::fromUtf8("textEdit"));

        gridLayout->addWidget(textEdit, 1, 0, 1, 1);

        Ihm->setCentralWidget(centralwidget);
        menubar = new QMenuBar(Ihm);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 778, 27));
        menuGame = new QMenu(menubar);
        menuGame->setObjectName(QString::fromUtf8("menuGame"));
        menuRules = new QMenu(menuGame);
        menuRules->setObjectName(QString::fromUtf8("menuRules"));
        menuTime = new QMenu(menubar);
        menuTime->setObjectName(QString::fromUtf8("menuTime"));
        Ihm->setMenuBar(menubar);
        statusbar = new QStatusBar(Ihm);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        Ihm->setStatusBar(statusbar);

        menubar->addAction(menuGame->menuAction());
        menubar->addAction(menuTime->menuAction());
        menuGame->addAction(actionNewGame);
        menuGame->addAction(menuRules->menuAction());
        menuGame->addAction(actionQuit);

        retranslateUi(Ihm);

        QMetaObject::connectSlotsByName(Ihm);
    } // setupUi

    void retranslateUi(QMainWindow *Ihm)
    {
        Ihm->setWindowTitle(QApplication::translate("Ihm", "Gomoku", 0, QApplication::UnicodeUTF8));
        actionQuit->setText(QApplication::translate("Ihm", "Quit", 0, QApplication::UnicodeUTF8));
        actionNewGame->setText(QApplication::translate("Ihm", "New Game", 0, QApplication::UnicodeUTF8));
        menuGame->setTitle(QApplication::translate("Ihm", "Game", 0, QApplication::UnicodeUTF8));
        menuRules->setTitle(QApplication::translate("Ihm", "Rules", 0, QApplication::UnicodeUTF8));
        menuTime->setTitle(QApplication::translate("Ihm", "Time", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class Ihm: public Ui_Ihm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_IHM_H
