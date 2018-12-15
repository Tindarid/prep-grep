#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDirIterator>
#include <QtConcurrent/qtconcurrentrun.h>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentFilter>
#include <QFutureWatcher>
#include <iostream>

main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->actionIndex_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionToggle_indexing->setIcon(style.standardIcon(QCommonStyle::SP_MediaPause));
    ui->actionStop_indexing->setIcon(style.standardIcon(QCommonStyle::SP_MediaStop));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionIndex_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionStop_indexing, &QAction::triggered, this, &main_window::stopIndexing);
    connect(ui->actionToggle_indexing, &QAction::triggered, this, &main_window::pauseIndexing);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(&result, SIGNAL(finished()), SLOT(indexingFinished()));
    connect(&result, &QFutureWatcher<void>::progressRangeChanged, this->ui->progressBar, &QProgressBar::setRange);
    connect(&result, &QFutureWatcher<void>::progressValueChanged, this->ui->progressBar, &QProgressBar::setValue);
    connect(&result, &QFutureWatcher<void>::started, this->ui->progressBar, &QProgressBar::reset);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &main_window::startSearching);
}

main_window::~main_window() {}

void main_window::startSearching() {
    QString pattern = ui->lineEdit->text();
    ui->label->setText(pattern);
    ui->treeWidget->clear();
    if (pattern.length() > 0) {
        TrigramSet patternSet = Trigram::processString(pattern);
        for (int i = 0; i < files.size(); ++i) {
            if (Trigram::isSubset(patternSet, files[i])) {
                auto ans = Trigram::findInFile(files[i].filename, pattern);
                if (!ans.empty()) {
                    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
                    item->setText(0, QString("Found in ") + files[i].filename);
                    for (auto it = ans.begin(); it != ans.end(); ++it) {
                        QTreeWidgetItem* childItem = new QTreeWidgetItem();
                        childItem->setText(0, it->second);
                        item->addChild(childItem);
                    }
                    ui->treeWidget->addTopLevelItem(item);
                }
            }
        }
    }
}

void main_window::stopIndexing() {
    result.cancel();
}

void main_window::pauseIndexing() {
    bool flag = result.isPaused();
    if (flag) {
        ui->actionToggle_indexing->setIcon(style.standardIcon(QCommonStyle::SP_MediaPause));
    } else {
        ui->actionToggle_indexing->setIcon(style.standardIcon(QCommonStyle::SP_MediaPlay));
    }
    result.togglePaused();
}

void main_window::indexingFinished() {
    ui->label->setText(QString("Filtering text files from index ..."));
    QtConcurrent::blockingFilter(files, Trigram::isText);
    ui->label->setText(QString("Indexing time: ") + QString::number(timer.elapsed() / 1000.0) +
                       QString(". Indexed files: " + QString::number(files.size()) + QString(".")));
    ui->lineEdit->setDisabled(false);
    ui->actionIndex_Directory->setDisabled(false);
}

void main_window::select_directory() {
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.length() > 0) {
        scan_directory(dir);
    }
}

void main_window::scan_directory(QString const& dir) {
    ui->treeWidget->clear();
    ui->lineEdit->setDisabled(true);
    timer.start();

    QDirIterator it(dir, QDir::Hidden | QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    files.clear();
    while (it.hasNext()) {
        files.push_back(TrigramSet(it.next()));
    }
    ui->label->setText(QString("Indexing: ..."));
    ui->actionIndex_Directory->setDisabled(true);
    result.setFuture(QtConcurrent::map(files, &Trigram::processFile));
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
