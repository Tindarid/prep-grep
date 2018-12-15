#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QVector>
#include "trigram.h"

class Worker : public QObject
{
    Q_OBJECT

private:
    QVector<QPair<QPair<int, int>, QString>> findInFile(QString const& filename, QString const& pattern);
signals:
    void result(QString const& filename, QVector<QPair<QPair<int, int>, QString>> entries);
    void finished();
public slots:
    void doSearch(QVector<TrigramSet> const& files, QString const& pattern);
};

#endif // WORKER_H
