#ifndef TRIGRAM_H
#define TRIGRAM_H

#include <set>
#include <QString>
#include <QPair>

struct TrigramSet {
    QString filename;
    std::set<std::pair<QChar, std::pair<QChar, QChar>>> trigrams;
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
