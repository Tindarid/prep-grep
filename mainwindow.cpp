#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDesktopWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QtConcurrent/QtConcurrentMap>
#include <QtConcurrent/QtConcurrentFilter>

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

    connect(ui->actionIndex_Directory, &QAction::triggered, this, &main_window::indexDirectory);
    connect(ui->actionStop_indexing, &QAction::triggered, this, &main_window::stopIndexing);
    connect(ui->actionToggle_indexing, &QAction::triggered, this, &main_window::pauseIndexing);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(&result, &QFutureWatcher<void>::finished, this, &main_window::indexingFinished);
    connect(&result, &QFutureWatcher<void>::progressRangeChanged, this->ui->progressBar, &QProgressBar::setRange);
    connect(&result, &QFutureWatcher<void>::progressValueChanged, this->ui->progressBar, &QProgressBar::setValue);
    connect(&result, &QFutureWatcher<void>::started, this->ui->progressBar, &QProgressBar::reset);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &main_window::patternChanged);
    connect(ui->searchButton, &QPushButton::clicked, this, &main_window::resolveSearch);

    setWindowTitle("Prep-Grep");

    qRegisterMetaType<QVector<TrigramSet>>("QVector<TrigramSet>");
    qRegisterMetaType<QVector<QPair<int, QString>>>("QVector<QPair<int, QString>>");
    searchThread.start();
    searchThread.quit();
    searchThread.wait();
}

main_window::~main_window() {
    searchThread.quit();
    searchThread.wait();
}

void main_window::resolveSearch() {
    QString text = ui->searchButton->text();
    if (text == "Stop") {
        searchThread.requestInterruption();
    } else {
        initSearch(ui->lineEdit->text());
    }
}

void main_window::patternChanged() {
    if (!ui->checkBox->isChecked()) {
        return;
    }
    QString pattern = ui->lineEdit->text();
    ui->treeWidget->clear();
    if (pattern.length() > 2) {
        if (!searchThread.isFinished()) {
            worker->blockSignals(true);
            searchThread.quit();
            searchThread.wait();
        }
        initSearch(pattern);
    } else {
        ui->label->clear();
    }
}

void main_window::initSearch(QString const& pattern) {
    timer.start();
    ui->treeWidget->clear();
    ui->label->setText("Searching...");
    ui->searchButton->setText("Stop");

    worker = new Worker;
    worker->moveToThread(&searchThread);
    connect(&searchThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &main_window::startSearching, worker, &Worker::doSearch);
    connect(worker, &Worker::result, this, &main_window::handleResult);
    connect(worker, &Worker::finished, this, &main_window::searchingFinished);
    searchThread.start();

    emit startSearching(files, pattern);
}

void main_window::handleResult(QString const& filename, QVector<QPair<int, QString>> entries) {
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, QString("Found in ") + curdir.relativeFilePath(filename));
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        QTreeWidgetItem* childItem = new QTreeWidgetItem(item);
        childItem->setText(0, QString("Line ") + QString::number(it->first) + QString(": \n") + it->second);
        item->addChild(childItem);
    }
    ui->treeWidget->addTopLevelItem(item);
}

void main_window::searchingFinished() {
    ui->label->setText(QString("Search time: ") + QString::number(timer.elapsed() / 1000.0) + QString(" sec."));
    if (!ui->treeWidget->topLevelItem(0)) {
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, QString("No entries"));
        ui->treeWidget->addTopLevelItem(item);
    }
    ui->searchButton->setText("Search");
    searchThread.quit();
    searchThread.wait();
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
    QtConcurrent::blockingFilter(files, TrigramUtil::isText);
    ui->label->setText(QString("Indexing time: ") + QString::number(timer.elapsed() / 1000.0) +
                       QString("sec. Indexed files: " + QString::number(files.size()) + QString(".")));
    ui->lineEdit->setDisabled(false);
    ui->actionIndex_Directory->setDisabled(false);
    ui->actionToggle_indexing->setDisabled(true);
    ui->actionStop_indexing->setDisabled(true);
}

void main_window::indexDirectory() {
    if (!searchThread.isFinished()) {
        searchThread.quit();
        searchThread.wait();
    }
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.length() == 0) {
        return;
    }

    curdir = QDir(dir);
    setWindowTitle(QString("Directory - %1").arg(dir));
    timer.start();
    ui->treeWidget->clear();
    ui->lineEdit->setDisabled(true);
    ui->lineEdit->clear();

    QDirIterator it(dir, QDir::Hidden | QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    files.clear();
    while (it.hasNext()) {
        files.push_back(TrigramSet(it.next()));
    }
    ui->label->setText(QString("Indexing ..."));
    ui->actionIndex_Directory->setDisabled(true);
    ui->actionToggle_indexing->setDisabled(false);
    ui->actionStop_indexing->setDisabled(false);
    result.setFuture(QtConcurrent::map(files, &TrigramUtil::processFile));
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}
