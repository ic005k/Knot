
#pragma once

#include <qglobal.h>

#include <QFile>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QSettings>
#include <QTabWidget>
#include <QTextBlock>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <atomic>

#include "src/CloudBackup.h"
#include "src/Comm/Method.h"
#include "src/Comm/TextEditToolbar.h"

// 用“前向声明”替代“#include 具体头文件”，避免循环引用
class CloudBackup;
class Method;
class MainWindow;
class Notes;
class NotesList;
class StepsOptions;
class Steps;
class ReaderSet;
class ShowMessage;
class CategoryList;
class ColorDialog;
class PrintPDF;
namespace Ui {
class MainWindow;
}

#ifdef Q_OS_ANDROID
#define ANDROID_MAIN_ACTIVITY "com/x/MyActivity"
#endif

inline QSettings* iniPreferences;
inline ReaderSet* m_ReaderSet;
inline CloudBackup* m_CloudBackup;
inline QTreeWidgetItem* parentItem;
inline MainWindow* mw_one;
inline Ui::MainWindow* mui;
inline Method* m_Method;
inline Notes* m_Notes;
inline NotesList* m_NotesList;
inline StepsOptions* m_StepsOptions;
inline Steps* m_Steps;
inline QTabWidget* tabData;
inline CategoryList* m_CategoryList;
inline ColorDialog* colorDlg;
inline PrintPDF* m_PrintPDF;
inline QTreeWidget *twrb, *tw;

inline TextEditToolbar* textToolbar = nullptr;
inline TextEditToolbar* textToolbarDynamic = nullptr;

inline QRegularExpression regxNumber("^-?[0-9.]*$");

inline QString iniDir, privateDir, bakfileDir, strDate, readDate, noteText,
    strStats, SaveType, strY, strM, btnYText, btnMText, btnDText, errorInfo,
    CurrentYearMonth, zipfile, txt, searchStr, currentMDFile, copyText,
    imgFileName, defaultFontFamily, customFontFamily, encPassword, btnYearText,
    btnMonthText, strPage, ebookFile, strTitle, fileName, strOpfPath,
    catalogueFile, strShowMsg, strStartTotalTime, strOpfFile, oldOpfPath,
    strEpubTitle, strPercent;

inline QString ver;
inline QString appName = "Knot";

inline QStringList readTextList, htmlFiles, listCategory, ncxList, tempHtmlList,
    listM, ymdList;

inline QList<QPointF> freqPointList;
inline QList<QPointF> amountList;
inline QList<double> doubleList;

inline int fontSize, red, iPage, sPos, totallines, s_y1, s_m1, s_d1, s_y2, s_m2,
    s_d2, totalPages, currentPage, infoProgBarValue, infoProgBarMax,
    currentTabIndex, today;

inline int nMainChartType = 0;

inline int chartMax = 5;
inline int baseLines = 50;
inline int htmlIndex = 0;
inline int minBytes = 200000;
inline int maxBytes = 400000;

inline int zlibMethod = 1;
inline int readerFontSize = 18;
inline int epubFileMethod = 2;

inline double yMaxMonth, yMaxDay;

inline bool isAndroid, isLinux, isMacOS, isZipOK, isMenuImport, isDownData,
    loading, isEncrypt, isIOS, isEpub, isEpubError, isText, isPDF,
    isInitThemeEnd, isUpData, isRemovedTopItem, isReport, isReadTWEnd,
    isWindows, isEBook;

inline bool isPasswordError = false;
inline bool isrbFreq = true;
inline bool isAdd = false;
inline bool isReadEBookEnd = true;
inline std::atomic<bool> isSaveEnd = true;
inline std::atomic<bool> isReadEnd = true;
inline std::atomic<bool> isBreak = false;
inline bool isDark = false;
inline bool isDelData = false;
inline bool isWholeMonth = true;
inline bool isDateSection = false;
inline bool isOpen = false;
inline bool isZH_CN = false;
inline bool isNeedExecDeskShortcut = false;

inline QString loadText(QString textFile);
inline void TextEditToFile(QTextEdit* txtEdit, QString fileName);
inline int deleteDirfile(QString dirName);
inline QString getTextEditLineText(QTextEdit* txtEdit, int i);
inline bool StringToFile(QString buffers, QString fileName);
inline bool unzipToDir(const QString& zipPath, const QString& destDir);
inline QString markdownToHtmlWithMath(const QString& md);
inline int deleteDirfile(QString dirName);
inline WebDavHelper* listWebDavFiles(const QString& url,
                                     const QString& username,
                                     const QString& password);
inline void closeTextToolBar();
inline void initTextToolbarDynamic(QWidget* parent);

void closeTextToolBar() {
  if (textToolbar != nullptr && textToolbar->isVisible()) {
    textToolbar->hide();
  }

  if (textToolbarDynamic != nullptr && textToolbarDynamic->isVisible()) {
    textToolbarDynamic->hide();
  }
}

void initTextToolbarDynamic(QWidget* parent) {
  if (textToolbarDynamic != nullptr) {
    textToolbarDynamic->deleteLater();
    textToolbarDynamic = nullptr;
  }
  textToolbarDynamic = new TextEditToolbar(parent);
}

QString loadText(QString textFile) {
  bool isExists = QFile(textFile).exists();
  if (isExists) {
    QFile file(textFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
      qDebug() << "Cannot read file";

    } else {
      QByteArray data = file.readAll();
      QString text = m_Method->convertDataToUnicode(data);
      return text;
    }
    file.close();
  }

  return "";
}

bool StringToFile(QString buffers, QString fileName) {
  bool isValue;
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out(file);
    out << buffers;
    file->close();
    isValue = true;
  } else {
    qDebug() << "Write failure!" << fileName;
    isValue = false;
  }

  delete file;
  return isValue;
}

void TextEditToFile(QTextEdit* txtEdit, QString fileName) {
  QFile* file;
  file = new QFile;
  file->setFileName(fileName);
  bool ok = file->open(QIODevice::WriteOnly | QIODevice::Text);
  if (ok) {
    QTextStream out(file);
    out << txtEdit->toPlainText();
    file->close();

  } else
    qDebug() << "Write failure!" << fileName;

  delete file;
}

QString getTextEditLineText(QTextEdit* txtEdit, int i) {
  QTextBlock block = txtEdit->document()->findBlockByNumber(i);
  txtEdit->setTextCursor(QTextCursor(block));
  QString lineText = txtEdit->document()->findBlockByNumber(i).text();
  return lineText;
}

int deleteDirfile(QString dirName) {
  QDir directory(dirName);
  if (!directory.exists()) {
    return true;
  }

  QString srcPath = QDir::toNativeSeparators(dirName);
  if (!srcPath.endsWith(QDir::separator())) srcPath += QDir::separator();

  QStringList fileNames = directory.entryList(
      QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
  bool error = false;
  for (QStringList::size_type i = 0; i != fileNames.size(); ++i) {
    QString filePath = srcPath + fileNames.at(i);
    QFileInfo fileInfo(filePath);
    if (fileInfo.isFile() || fileInfo.isSymLink()) {
      QFile::setPermissions(filePath, QFile::WriteOwner);
      if (!QFile::remove(filePath)) {
        error = true;
      }
    } else if (fileInfo.isDir()) {
      if (!deleteDirfile(filePath)) {
        error = true;
      }
    }
  }

  if (!directory.rmdir(QDir::toNativeSeparators(directory.path()))) {
    error = true;
  }
  return !error;
}
