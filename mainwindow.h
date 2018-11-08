#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "worker.h"
#include "shareddata.h"

#include <QMainWindow>
#include <QThread>
#include <QVector>
#include <QQueue>
#include <QTimer>

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

    void on_recurseReadingFlag_stateChanged(int checkFlag);

private:
    Ui::MainWindow *ui;
    Worker *worker;
    QThread *thread;
    SharedData *queue;

    //Working with queue elem
    QVector <Complex> byteVector;
    qreal number;
    qreal maxSample;
    qreal minSample;
    QString valueStr;
    const int SAMPLE_BLOCK = 8192;

    //For thread freezing
    QTimer timer;
};

#endif // MAINWINDOW_H
