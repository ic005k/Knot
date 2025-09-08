#include "epubreader.h"

#include <QDebug>

EpubReader::EpubReader(QObject *parent) : QObject(parent) {
  // 初始化时指针为 nullptr
}

EpubReader::~EpubReader() {
  close();  // 析构时确保释放
}

bool EpubReader::open(const QString &epubPath) {
  close();  // 先关闭已打开的文件

  // 用 new 创建 QuaZip 对象（通过构造函数传入路径，适配最新版）
  m_zip = new QuaZip(epubPath);
  if (!m_zip) {
    qWarning() << "Failed to create QuaZip instance";
    return false;
  }

  // 调用 open 方法，仅传入模式（最新版 QuaZip 无需其他参数）
  if (!m_zip->open(QuaZip::mdUnzip)) {
    qWarning() << "Failed to open EPUB:" << m_zip->getZipError()
               << "Path:" << epubPath;
    delete m_zip;  // 打开失败时释放内存
    m_zip = nullptr;
    return false;
  }

  // 设置文件名编码（处理中文/特殊字符）
  m_zip->setFileNameCodec("UTF-8");
  m_epubPath = epubPath;
  return true;
}

void EpubReader::close() {
  if (m_zip) {
    m_zip->close();  // 关闭文件
    delete m_zip;    // 释放内存
    m_zip = nullptr;
  }
  m_epubPath.clear();
}

QByteArray EpubReader::readFile(const QString &internalPath) {
  if (!isOpen()) {
    qWarning() << "EPUB file not open";
    return QByteArray();
  }

  QString path = internalPath;
  path.replace("\\", "/");

  // 定位文件（指针用 -> 调用）
  if (!m_zip->setCurrentFile(path)) {
    qWarning() << "File not found:" << path << "Error:" << m_zip->getZipError();
    return QByteArray();
  }

  // 读取文件内容（QuaZipFile 关联指针，直接传 m_zip 即可）
  QuaZipFile file(m_zip);  // 指针本身就是地址，无需 &
  if (!file.open(QIODevice::ReadOnly)) {
    qWarning() << "Open file failed:" << path << "Error:" << file.getZipError();
    return QByteArray();
  }

  QByteArray content = file.readAll();
  file.close();
  return content;
}

bool EpubReader::fileExists(const QString &internalPath) {
  if (!isOpen()) return false;
  QString path = internalPath;
  path.replace("\\", "/");
  return m_zip->setCurrentFile(path);
}

QStringList EpubReader::getAllFilePaths() {
  QStringList paths;
  if (!isOpen()) return paths;

  for (bool more = m_zip->goToFirstFile(); more; more = m_zip->goToNextFile()) {
    paths.append(m_zip->getCurrentFileName());
  }
  return paths;
}

qint64 EpubReader::getFileSize(const QString &internalPath) {
  if (!isOpen()) {
    qWarning() << "EPUB file not open";
    return -1;
  }

  QString path = internalPath;
  path.replace("\\", "/");

  // 定位到指定文件
  if (!m_zip->setCurrentFile(path)) {
    qWarning() << "File not found:" << path << "Error:" << m_zip->getZipError();
    return -1;
  }

  // 获取文件信息（无需打开文件）
  QuaZipFileInfo fileInfo;
  if (!m_zip->getCurrentFileInfo(&fileInfo)) {
    qWarning() << "Failed to get file info:" << path
               << "Error:" << m_zip->getZipError();
    return -1;
  }

  // 返回未压缩的文件大小（也可根据需要返回压缩后的大小fileInfo.compressedSize）
  return fileInfo.uncompressedSize;
}
