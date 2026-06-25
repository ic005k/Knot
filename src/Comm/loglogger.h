#ifndef LOGLOGGER_H
#define LOGLOGGER_H

#include <QMessageLogContext>
#include <QMutex>
#include <QString>

#include "src/defines.h"

class AppLogger {
 public:
  static AppLogger& instance();
  void initLogger(const QString& appName);
  static void msgHandler(QtMsgType type, const QMessageLogContext& ctx,
                         const QString& msg);
  QString getLogRootDir() const;

 private:
  AppLogger();
  ~AppLogger();
  AppLogger(const AppLogger&) = delete;
  AppLogger& operator=(const AppLogger&) = delete;

  void writeToFile(QtMsgType type, const QString& msg);
  void rotateLogFile();
  void clearExpiredLogs();

  QString m_appName;
  QString m_logRootDir;
  QString m_currentLogPath;
  QMutex m_writeMutex;

  inline static constexpr qint64 MAX_LOG_FILE_SIZE = 10 * 1024 * 1024;
  inline static constexpr int MAX_KEEP_DAYS = 7;
};

#endif
