#ifndef TRIGRAM_H
#define TRIGRAM_H

#include <QSet>
#include <QString>
#include <QPair>

struct TrigramSet {
    QString filename;
    QSet<quint64> trigrams;
    bool good;
    TrigramSet(QString const& str) {
        filename = str;
        good = false;
    }
    TrigramSet() {
        good = false;
    }
};

class Trigram {
public:
    static const int MAX_TRIGRAMS = 20000;
    static const int MAX_LINE_LEN = 2000;
    static const int MAX_FILE_SIZE = 1 << 30;

    static bool isSubset(TrigramSet& subset, TrigramSet& set);
    static void processFile(TrigramSet&);
    static TrigramSet processString(QString const& str);
    static bool isText(const TrigramSet& set);
    static QVector<QPair<int, QString>> findInFile(QString& filename, QString& pattern);
private:
    static void addFromString(TrigramSet& set, QString const& str);
};

#endif // TRIGRAM_H
