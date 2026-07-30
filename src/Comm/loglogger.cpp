#include "loglogger.h"

#include <QClipboard>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QTextStream>
#include <QThread>
#include <QtGlobal>
#include <iostream>
#ifdef Q_OS_ANDROID
#include <android/log.h>
#endif

AppLogger& AppLogger::instance() {
  static AppLogger obj;
  return obj;
}

AppLogger::AppLogger() = default;
AppLogger::~AppLogger() {
  m_writeMutex.lock();
  m_writeMutex.unlock();
}

void AppLogger::initLogger(const QString& appName) {
  m_appName = appName;

  // privateDir 末尾自带 /，直接拼接 logs/
  m_logRootDir = privateDir + "logs/";

  QDir logDir(m_logRootDir);
  if (!logDir.exists()) logDir.mkpath(".");

  qInstallMessageHandler(AppLogger::msgHandler);

  rotateLogFile();
  clearExpiredLogs();
}

QString AppLogger::getLogRootDir() const { return m_logRootDir; }

void AppLogger::msgHandler(QtMsgType type, const QMessageLogContext& ctx,
                           const QString& msg) {
  // ✅ 关键：在 Android 上显式调用原生日志接口，确保 Logcat 完整捕获
#ifdef Q_OS_ANDROID
  int androidLevel;
  switch (type) {
    case QtDebugMsg:
      androidLevel = ANDROID_LOG_DEBUG;
      break;
    case QtInfoMsg:
      androidLevel = ANDROID_LOG_INFO;
      break;
    case QtWarningMsg:
      androidLevel = ANDROID_LOG_WARN;
      break;
    case QtCriticalMsg:
      androidLevel = ANDROID_LOG_ERROR;
      break;
    case QtFatalMsg:
      androidLevel = ANDROID_LOG_FATAL;
      break;
    default:
      androidLevel = ANDROID_LOG_DEFAULT;
      break;
  }
  // 使用 "qt" 作为 tag，与 Qt 默认行为一致，Creator 才能正确识别和着色
  __android_log_print(androidLevel, "qt", "%s", qPrintable(msg));
#else
  // 非 Android 平台保持原有 cout 输出
  QString timeStr =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  quint64 tidRaw = reinterpret_cast<quint64>(QThread::currentThreadId());
  qint64 threadId = static_cast<qint64>(tidRaw);

  QString levelTag;
  switch (type) {
    case QtDebugMsg:
      levelTag = "DEBUG";
      break;
    case QtInfoMsg:
      levelTag = "INFO";
      break;
    case QtWarningMsg:
      levelTag = "WARN";
      break;
    case QtCriticalMsg:
      levelTag = "CRIT";
      break;
    case QtFatalMsg:
      levelTag = "FATAL";
      break;
    default:
      levelTag = "UNKNOWN";
      break;
  }

  QString logLine =
      QString("[%1][TID:%2][%3] %4\n")
          .arg(timeStr, QString::number(threadId, 10, 6), levelTag, msg);
  std::cout << logLine.toStdString() << std::flush;
#endif

  // 文件写入逻辑在所有平台都执行（保持不变）
  instance().writeToFile(type, msg);
}

void AppLogger::writeToFile(QtMsgType type, const QString& msg) {
  m_writeMutex.lock();

  QString timeStr =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
  quint64 tidRaw = reinterpret_cast<quint64>(QThread::currentThreadId());
  qint64 threadId = static_cast<qint64>(tidRaw);

  QString levelTag;
  switch (type) {
    case QtDebugMsg:
      levelTag = "DEBUG";
      break;
    case QtInfoMsg:
      levelTag = "INFO";
      break;
    case QtWarningMsg:
      levelTag = "WARN";
      break;
    case QtCriticalMsg:
      levelTag = "CRIT";
      break;
    case QtFatalMsg:
      levelTag = "FATAL";
      break;
    default:
      levelTag = "UNKNOWN";
  }

  QString logLine =
      QString("[%1][TID:%2][%3] %4\n")
          .arg(timeStr, QString::number(threadId, 10, 6), levelTag, msg);

  rotateLogFile();
  QFile logFile(m_currentLogPath);
  if (logFile.open(QIODevice::Append | QIODevice::Text)) {
    QTextStream stream(&logFile);
    stream << logLine;
    stream.flush();
    logFile.close();
  }

  m_writeMutex.unlock();

  if (type == QtFatalMsg) {
    // 可在此添加崩溃dump生成逻辑
  }
}

void AppLogger::rotateLogFile() {
  QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
  // 直接拼接路径，m_logRootDir末尾已有/
  QString targetLogName =
      m_logRootDir + QString("%1_%2.log").arg(m_appName, todayStr);

  if (m_currentLogPath != targetLogName) {
    m_currentLogPath = targetLogName;
    return;
  }

  QFileInfo fileInfo(m_currentLogPath);
  if (fileInfo.size() >= MAX_LOG_FILE_SIZE) {
    QString backupName = m_currentLogPath;
    backupName.replace(
        ".log", QString("_%1.log").arg(QDateTime::currentMSecsSinceEpoch()));
    QFile::rename(m_currentLogPath, backupName);
  }
}

void AppLogger::clearExpiredLogs() {
  QDir dir(m_logRootDir);
  dir.setNameFilters({"*.log"});
  dir.setFilter(QDir::Files);
  QFileInfoList logFiles = dir.entryInfoList();
  QDateTime expireTime = QDateTime::currentDateTime().addDays(-MAX_KEEP_DAYS);

  // 传统下标循环，无detach警告
  for (int i = 0; i < logFiles.size(); ++i) {
    const QFileInfo& info = logFiles[i];
    // lastModified 全平台稳定，替代不可靠的 birthTime
    if (info.lastModified() < expireTime) {
      QFile::remove(info.absoluteFilePath());
    }
  }
}

QString AppLogger::getTodayLogText() {
  QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
  QString logFile =
      m_logRootDir + QString("%1_%2.log").arg(m_appName, todayStr);

  QFileInfo fi(logFile);
  if (!fi.exists()) {
    return "No today's log for today.";
  }
  qint64 totalSize = fi.size();

  QFile f(logFile);
  if (!f.open(QIODevice::ReadOnly)) {
    return "No today's log for today.";
  }

  // 只读最后128k的内容
  const qint64 SAFE_MAX = 128 * 1024;
  QByteArray raw;
  if (totalSize <= SAFE_MAX) {
    raw = f.readAll();
  } else {
    f.seek(totalSize - SAFE_MAX);
    raw = f.read(SAFE_MAX);
    int firstNewline = raw.indexOf('\n');
    if (firstNewline != -1) {
      raw = raw.mid(firstNewline + 1);
    }
  }
  f.close();

  const QByteArray launchFlag = "The app has started to launch...";
  int lastLaunchPos = raw.lastIndexOf(launchFlag);
  QString finalLog;

  if (lastLaunchPos != -1) {
    // 提取启动标记之前的全部日志文本
    QByteArray preLaunchRaw = raw.left(lastLaunchPos);
    QString preText = QString::fromUtf8(preLaunchRaw);
    // 分割行并过滤空行
    QStringList preLines = preText.split('\n', Qt::SkipEmptyParts);

    // 最多取启动行上方5行退出日志
    int MAX_PRE_EXIT_LINES = 5;
#ifdef Q_OS_ANDROID
    MAX_PRE_EXIT_LINES = 25;
#endif

    QStringList exitLines;
    if (!preLines.isEmpty()) {
      int startIdx = qMax(0, preLines.size() - MAX_PRE_EXIT_LINES);
      for (int i = startIdx; i < preLines.size(); ++i) {
        exitLines.append(preLines[i]);
      }
    }

    QString exitSection = exitLines.join("\n");
    QString runSection = QString::fromUtf8(raw.mid(lastLaunchPos));

    // 英文分割线，仅当前置存在日志时才拼接分割线
    if (!exitSection.isEmpty()) {
      finalLog = exitSection;
      finalLog +=
          "\n==================== APP LAUNCH SEPARATOR ====================\n";
      finalLog += runSection;
    } else {
      // 启动标记前0行，直接输出本次运行日志，无多余分隔线
      finalLog = runSection;
    }
  } else {
    // 未找到任何启动标记，直接输出读取到的全部日志
    finalLog = QString::fromUtf8(raw);
  }

  return finalLog;
}

// 对外工具函数：直接复制当日日志到剪贴板
void AppLogger::copyTodayLogToClipboard() {
  QString logText = getTodayLogText();
  QClipboard* clip = QGuiApplication::clipboard();
  clip->setText(logText);
}
