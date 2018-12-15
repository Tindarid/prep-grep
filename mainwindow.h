#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QFutureWatcher>
#include <QTime>
#include <QCommonStyle>
#include <QThread>
#include <QDir>
#include "trigram.h"
#include "worker.h"

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
    void indexDirectory();
    void show_about_dialog();
    void indexingFinished();
    void stopIndexing();
    void pauseIndexing();
    void searchingFinished();
    void patternChanged();
    void initSearch(QString const&);
    void handleResult(QString const&, QVector<QPair<int, QString>>);
    void resolveSearch();

signals:
    void startSearching(QVector<TrigramSet> const& files, QString const& pattern);

private:
    std::unique_ptr<Ui::MainWindow> ui;
    QFutureWatcher<void> result;
    QTime timer;
    QVector<TrigramSet> files;
    QCommonStyle style;
    QDir curdir;
};

#endif // MAINWINDOW_H
