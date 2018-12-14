#include "trigram.h"
#include <QFile>
#include <QTextStream>
#include <QVector>
#include <iostream>

bool Trigram::isSubset(TrigramSet& a, TrigramSet& b) {
    for (auto it = a.trigrams.begin(); it != a.trigrams.end(); ++it) {
        if (b.trigrams.find(*it) == b.trigrams.end()) {
            return false;
        }
    }
    return true;
}

void Trigram::processFile(TrigramSet& res) {
    QFile file(res.filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return;
    }
    if (file.size() > MAX_FILE_SIZE) {
        return;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString str = in.readLine();
        if (str.length() > MAX_LINE_LEN) {
            res.trigrams.clear();
            return;
        }
        addFromString(res, str);
        if (res.trigrams.size() > MAX_TRIGRAMS) {
            res.trigrams.clear();
            return;
        }
    }
    res.good = true;
}

TrigramSet Trigram::processString(QString const& str) {
    TrigramSet res(str);
    addFromString(res, str);
    res.good = true;
    return res;
}

void Trigram::addFromString(TrigramSet& set, QString const& str) {
    for (int i = 0; i + 2 < str.length(); ++i) {
        set.trigrams.insert(std::make_pair(str[i], std::make_pair(str[i + 1], str[i + 2])));
    }
}

bool Trigram::isText(const TrigramSet& set) {
    return set.good;
}

QVector<QPair<int, QString>> Trigram::findInFile(QString& filename, QString& pattern) {
    QFile file(filename);
    QVector<QPair<int, QString>> ans;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return ans;
    }
    QTextStream in(&file);
    int patlen = pattern.length();
    while (!in.atEnd()) {
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
            ans.push_back(QPair<int, QString>(count, str));
        }
    }
    return ans;
}
