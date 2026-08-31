#include "Reader.h"
#include "src/MainWindow.h"

extern EpubReader* reader;

void Reader::setQMLHtml(QString htmlFile, QString htmlBuffer, QString skipID) {}

void Reader::loadQMLText(QString str) {}

QString Reader::getQMLText() { return ""; }

bool Reader::getQmlReadyEnd() { return false; }

void Reader::setQmlLandscape(bool isValue) {}

void Reader::showBookListWin() {}

void Reader::hideBookListWin() {}

bool Reader::isBookListWinVisible() { return false; }
