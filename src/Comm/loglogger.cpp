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
#include <iostream>

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
  Q_UNUSED(ctx);
  // 所有qDebug全部写入日志，不做过滤
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

  QString logLine = QString("[%1][TID:%2][%3] %4\n")
                        .arg(timeStr)
                        .arg(threadId, 6)
                        .arg(levelTag)
                        .arg(msg);

  // 控制台输出
  std::cout << logLine.toStdString();

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
      m_logRootDir + QString("%1_%2.log").arg(m_appName).arg(todayStr);

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

  for (const QFileInfo& info : logFiles) {
    if (info.birthTime() < expireTime) QFile::remove(info.absoluteFilePath());
  }
}

QString AppLogger::getTodayLogText() {
  QString todayStr = QDate::currentDate().toString("yyyy-MM-dd");
  QString logFile =
      m_logRootDir + QString("%1_%2.log").arg(m_appName).arg(todayStr);

  QFile f(logFile);
  if (!f.exists() || !f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return "暂无今日日志";
  }
  // 限制最大读取 5MB，避免超大日志卡死界面
  const qint64 MAX_READ = 5 * 1024 * 1024;
  QByteArray data = f.read(MAX_READ);
  f.close();

  return QString::fromUtf8(data);
}

// 对外工具函数：直接复制当日日志到剪贴板
void AppLogger::copyTodayLogToClipboard() {
  QString logText = getTodayLogText();
  QClipboard* clip = QGuiApplication::clipboard();
  clip->setText(logText);
}
