#include "worker.h"
#include "trigram.h"
#include <QFile>
#include <QTextStream>
#include <QThread>

void Worker::doSearch(QVector<TrigramSet> const& files, QString const& pattern) {
    QThread* thread = QThread::currentThread();
    TrigramSet patternSet = TrigramUtil::processString(pattern);
    for (int i = 0; i < files.size(); ++i) {
        if (thread->isInterruptionRequested()) {
            break;
        }
        if (TrigramUtil::isSubset(patternSet, files[i])) {
            QString const& filename = files[i].filename;
            auto ans = findInFile(filename, pattern);
            if (!ans.empty()) {
                emit result(filename, ans);
            }
        }
    }
    emit finished();
}

QVector<QPair<QPair<int, int>, QString>> Worker::findInFile(QString const& filename, QString const& pattern) {
    QFile file(filename);
    QVector<QPair<QPair<int, int>, QString>> ans;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ans;
    }
    QTextStream in(&file);
    int patlen = pattern.length();
    int number = 0;
    while (!in.atEnd()) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            return ans;
        }
        number++;
        QString str = in.readLine();
        int count = 0;
        for (int i = 0; i + patlen - 1 < str.length(); ++i) {
            if (str[i] == pattern[0]) {
                bool flag = true;
                for (int j = 0; j < patlen; ++j) {
                    if (pattern[j] != str[i + j]) {
                        flag = false;
                        break;
                    }
                }
                if (flag) {
                    count++;
                }
            }
        }
        if (count > 0) {
            ans.push_back(QPair<QPair<int, int>, QString>(QPair<int, int>(number, count), str));
        }
    }
    return ans;
}
