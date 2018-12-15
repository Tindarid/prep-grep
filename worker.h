#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QVector>
#include <QThread>
#include "trigram.h"

class Worker : public QObject
{
    Q_OBJECT

private:
    QVector<QPair<int, QString>> findInFile(QString const& filename, QString const& pattern);
signals:
    void result(QString const& filename, QVector<QPair<int, QString>> entries);
    //void progressState(int progress);
    void finished();
public slots:
    void doSearch(QVector<TrigramSet> const& files, QString const& pattern);
};

#endif // WORKER_H
