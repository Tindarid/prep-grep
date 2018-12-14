#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "trigram.h"
#include <QFutureWatcher>
#include <QTime>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

private slots:
    void select_directory();
    void scan_directory(QString const& dir);
    void show_about_dialog();
    void indexingFinished();
    void stopIndexing();
    void pauseIndexing();
    void startSearching();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QFutureWatcher<void> result;
    QFutureWatcher<void> search;
    QTime timer;
    QVector<TrigramSet> files;
};

#endif // MAINWINDOW_H
