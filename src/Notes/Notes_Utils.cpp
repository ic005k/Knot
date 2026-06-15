#include "src/Notes/Notes.h"

QString Notes::getDateTimeStr() {
  int y = QDate::currentDate().year();
  int m = QDate::currentDate().month();
  int d = QDate::currentDate().day();
  int hh = QTime::currentTime().hour();
  int mm = QTime::currentTime().minute();
  int s = QTime::currentTime().second();

  auto pad = [](int n) {
    return n < 10 ? "0" + QString::number(n) : QString::number(n);
  };
  return QString::number(y) + pad(m) + pad(d) + "_" + pad(hh) + pad(mm) +
         pad(s);
}

qreal Notes::getVPos() { return sliderPos; }
qreal Notes::getVHeight() { return textHeight; }
QColor Notes::StringToColor(QString mRgbStr) {
  return QColor(mRgbStr.toUInt(NULL, 16));
}

bool Notes::isSetNewNoteTitle() {
  QString title = m_NotesList->noteTitle;
  return (title.trimmed() == "无标题笔记" ||
          title.trimmed() == "Untitled Note");
}

void Notes::renameTitle(bool isOk) {
  if (m_NotesList->getNoteBookCurrentIndex() < 0) return;

  if (isOk) {
    m_NotesList->renameCurrentItem(new_title);
    m_NotesList->saveNotesList();
  } else {
    m_NotesList->on_btnRename_clicked();
  }
}

void Notes::delLink(QString link) {
  QString mdBuffers = loadText(currentMDFile);

  if (!mdBuffers.contains(link)) {
    link.replace("http://", "");
  }

  mdBuffers.replace(link, "");

  mdBuffers.replace("[]()", "");

  if (mdBuffers.contains("]()")) {
    int startPos, endPos, length;

    int index = 0;

    QStringList titleList;

    while (mdBuffers.indexOf("]()", index) != -1) {
      startPos = mdBuffers.indexOf("[", index) + 1;

      if (startPos - 2 >= 0) {
        endPos = mdBuffers.indexOf("]()", startPos + 1);

        length = endPos - startPos;

        QString subStr = mdBuffers.mid(startPos, length);

        titleList.append(subStr);

        qDebug() << "delLink():" << startPos << length << subStr;

        index = endPos + 1;
      }
    }

    for (int i = 0; i < titleList.count(); i++) {
      QString title = titleList.at(i);

      QString str = "[" + title + "]()";

      mdBuffers.replace(str, "");
    }
  }

  mdBuffers = formatMDText(mdBuffers);

  StringToFile(mdBuffers, currentMDFile);
}

bool Notes::openUrl(const QString& url) {
#ifdef __linux__

  // 方案 A：尝试直接调用默认浏览器（优先）

  QProcess mimeProcess;

  mimeProcess.start("xdg-mime", {"query", "default", "text/html"});

  mimeProcess.waitForFinished();

  QString browser = mimeProcess.readAllStandardOutput().trimmed();

  bool browserSuccess = false;

  if (!browser.isEmpty()) {
    // 提取浏览器可执行文件名（去掉 .desktop 后缀）

    if (browser.endsWith(".desktop")) {
      browser = browser.left(browser.length() - 8);
    }

    // 保存并清除环境变量

    const char* originalLdPath = getenv("LD_LIBRARY_PATH");

    QString originalLdPathStr(originalLdPath ? originalLdPath : "");

    unsetenv("LD_LIBRARY_PATH");

    unsetenv("LD_PRELOAD");  // 额外清除可能的预加载库

    // 直接用浏览器打开

    browserSuccess = QProcess::startDetached(browser, {url});

    // 恢复环境变量

    if (!originalLdPathStr.isEmpty()) {
      setenv("LD_LIBRARY_PATH", originalLdPathStr.toUtf8().constData(), 1);

    } else {
      unsetenv("LD_LIBRARY_PATH");
    }
  }

  // 如果方案 A 失败，尝试方案 B：直接用系统库启动 kde-open

  if (!browserSuccess) {
    // 系统原生库路径（不同发行版可能略有差异，覆盖常见路径）

    QStringList systemLibPaths = {"/usr/lib64",       "/usr/lib",

                                  "/lib64",           "/lib",

                                  "/usr/local/lib64", "/usr/local/lib"};

    QString ldLibraryPath = systemLibPaths.join(":");

    // 保存原始环境变量

    const char* originalLdPath = getenv("LD_LIBRARY_PATH");

    QString originalLdPathStr(originalLdPath ? originalLdPath : "");

    const char* originalQtPluginPath = getenv("QT_PLUGIN_PATH");

    QString originalQtPluginPathStr(originalQtPluginPath ? originalQtPluginPath

                                                         : "");

    // 强制设置为系统库路径，清除所有可能影响 Qt 的环境变量

    setenv("LD_LIBRARY_PATH", ldLibraryPath.toUtf8().constData(), 1);

    unsetenv("LD_PRELOAD");

    unsetenv("QT_PLUGIN_PATH");

    unsetenv("QT_QPA_PLATFORM_PLUGIN_PATH");

    // 直接调用 kde-open（如果存在），否则用 xdg-open

    bool success = false;

    if (!QStandardPaths::findExecutable("kde-open").isEmpty()) {
      success = QProcess::startDetached("kde-open", {url});

    } else {
      success = QProcess::startDetached("xdg-open", {url});
    }

    // 恢复原始环境变量

    if (!originalLdPathStr.isEmpty()) {
      setenv("LD_LIBRARY_PATH", originalLdPathStr.toUtf8().constData(), 1);

    } else {
      unsetenv("LD_LIBRARY_PATH");
    }

    if (!originalQtPluginPathStr.isEmpty()) {
      setenv("QT_PLUGIN_PATH", originalQtPluginPathStr.toUtf8().constData(), 1);

    } else {
      unsetenv("QT_PLUGIN_PATH");
    }

    return success;
  }

  return browserSuccess;

#else

  Q_UNUSED(url);

#endif

  return true;
}

void Notes::openBrowserOnce(const QString& htmlPath) {
  if (isLinux)

    openUrl(htmlPath);

  else

    QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
}
