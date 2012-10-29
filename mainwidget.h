#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>
#include "windowwidget.h"
#include "topmenu.h"
#include "aboutwidget.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QToolBox>
#include <QSplitter>
#include <QMenuBar>
#include <QApplication>

class TimeLine;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow(){}
    void setHostedWindow(QWindow *window);
    void keyPress(QKeyEvent* e);
//    void resizeEvent(QResizeEvent *e);

protected:
    QSplitter *splitter;
    WindowWidget* glwgt;
    TopMenu* topMenu;
    TimeLine* timeLine;
    QToolBox* rightWgt;

    QVBoxLayout* vlayout;
    QHBoxLayout* hlayout;
    QWidget* centralwgt;

    AboutWidget* aboutWgt;

    void createMenuBar();
    void createPythonConsole();
//    void keyPressEvent(QKeyEvent *e);

public slots:
    void aboutProgram();
    void showTimeLine();
};

#endif // MAINWIDGET_H
