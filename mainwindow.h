#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "worker.h"
#include "shareddata.h"

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QQueue>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_loadFileBtn_clicked();

    void on_startCountingBtn_clicked();

    void getSamples();

private:
    Ui::MainWindow *ui;
    Worker *worker;
    QThread *thread;
    SharedData *queue;
    QVector <complex> byteVector;
    qreal number = 0;
    qreal maxSample = 0;
    qreal minSample = 0;
    QString valueStr;
};

#endif // MAINWINDOW_H
