#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QVector>

class DataManager : public QObject {
  Q_OBJECT
 public:
  // 构造函数（需传入数据存储目录）
  explicit DataManager(const QString &dataDir = "", QObject *parent = nullptr);

  // 加载数据到指定的QTreeWidget（自动读取2022到当前年的文件）
  void loadData(QTreeWidget *treeWidget);

  // 重载1：默认保存当前年的数据（兼容原有逻辑）
  void saveData(QTreeWidget *treeWidget);

  // 重载2：保存指定年份的数据（新增功能）
  void saveData(QTreeWidget *treeWidget, int targetYear);

  // 设置数据存储目录
  void setDataDir(const QString &dataDir);

  // 数据起始年份（App开发年份）
  static const int kDataStartYear;

 private:
  // 辅助函数：将JSON数据解析并追加到树控件
  void appendJsonToTree(QTreeWidget *tree, const QByteArray &jsonData,
                        const QString &expectedYear,
                        const QString &expectedTab);

  // 辅助函数：保存单一年份的数据到JSON文件
  void saveYearData(const QString &year, const QString &tabName,
                    const QVector<QTreeWidgetItem *> &items);

 private:
  QString m_dataDir;  // 数据存储根目录
};

#endif  // DATAMANAGER_H
