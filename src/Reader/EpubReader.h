#ifndef EPUBREADER_H
#define EPUBREADER_H

#include <lib/quazip/quazip.h>
#include <lib/quazip/quazipfile.h>

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

class EpubReader : public QObject {
  Q_OBJECT
 public:
  explicit EpubReader(QObject *parent = nullptr);
  ~EpubReader() override;

  bool open(const QString &epubPath);
  void close();
  QByteArray readFile(const QString &internalPath);
  bool fileExists(const QString &internalPath);
  QStringList getAllFilePaths();
  bool isOpen() const { return m_zip != nullptr && m_zip->isOpen(); }

  qint64 getFileSize(const QString &internalPath);
  private:
  QuaZip *m_zip = nullptr;  // 指针方式，避免赋值运算符问题
  QString m_epubPath;
};

#endif  // EPUBREADER_H
