#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filelistmodel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void transactionComplete();
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnSend_clicked();

private:
    Ui::MainWindow *ui;
    FileListModel *attachments;
};

#endif // MAINWINDOW_H
