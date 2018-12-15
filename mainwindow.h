#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "trigram.h"
#include "worker.h"
#include <QFutureWatcher>
#include <QTime>
#include <QCommonStyle>
#include <QThread>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
    Q_OBJECT
    QThread searchThread;
    Worker* worker;

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
    void searchingFinished();
    void patternChanged();
    void initSearch(QString const& pattern);
    void handleResult(QString const& filename, QVector<QPair<int, QString>> entries);
    void resolveSearch();

signals:
    void startSearching(QVector<TrigramSet> const& files, QString const& pattern);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QFutureWatcher<void> result;
    QTime timer;
    QVector<TrigramSet> files;
    QCommonStyle style;
};

#endif // MAINWINDOW_H
