#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "worker.h"

#include <QMainWindow>
#include <QThread>
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

    void getSamples(QQueue <qint8> *sampleQueue);

signals:
    //void startReading();

private:
    Ui::MainWindow *ui;
    Worker *worker;
    QThread *thread;
};

#endif // MAINWINDOW_H
