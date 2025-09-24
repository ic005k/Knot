#include "DataManager.h"

#include <QDate>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QTreeWidget>
#include <QTreeWidgetItem>

// 静态常量定义（App开发起始年份）
const int DataManager::kDataStartYear = 2022;

DataManager::DataManager(const QString &dataDir, QObject *parent)
    : QObject(parent), m_dataDir(dataDir) {
  // 确保数据目录以"/"结尾（避免路径拼接错误）
  if (!m_dataDir.endsWith("/")) {
    m_dataDir += "/";
  }
}

// 加载指定QTreeWidget的数据（2022到当前年，全部加载）
void DataManager::loadData(QTreeWidget *treeWidget) {
  if (!treeWidget) {
    qWarning() << "DataManager::loadData: 传入的QTreeWidget为空";
    return;
  }

  const QString tabName = treeWidget->objectName();
  const int currentYear = QDate::currentDate().year();

  // 清空现有数据
  treeWidget->clear();

  // 按年份顺序加载（2022 → 当前年）
  for (int year = kDataStartYear; year <= currentYear; ++year) {
    const QString yearStr = QString::number(year);
    const QString fileName = QString("%1-%2.json").arg(yearStr).arg(tabName);
    const QString filePath = m_dataDir + fileName;

    // 跳过不存在的文件
    if (!QFile::exists(filePath)) {
      continue;
    }

    // 读取并解析文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qWarning() << "无法打开文件:" << filePath
                 << "错误:" << file.errorString();
      continue;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // 解析JSON并追加到树控件
    appendJsonToTree(treeWidget, jsonData, yearStr, tabName);
  }
}

// 重载1：默认保存当前年的数据（兼容原有逻辑）
void DataManager::saveData(QTreeWidget *treeWidget) {
  // 调用重载2，传入当前年
  saveData(treeWidget, QDate::currentDate().year());
}

// 重载2：保存指定年份的数据（核心新增功能）
void DataManager::saveData(QTreeWidget *treeWidget, int targetYear) {
  if (!treeWidget) {
    qWarning() << "DataManager::saveData: 传入的QTreeWidget为空";
    return;
  }

  const QString tabName = treeWidget->objectName();
  const int currentYear = QDate::currentDate().year();

  // 校验目标年份是否有效（必须≥起始年，且≤当前年）
  if (targetYear < kDataStartYear || targetYear > currentYear) {
    qWarning() << "无效的目标年份:" << targetYear
               << "（有效范围:" << kDataStartYear << "~" << currentYear << "）";
    return;
  }

  const QString targetYearStr = QString::number(targetYear);

  // 提取目标年份的节点
  QVector<QTreeWidgetItem *> targetYearItems;
  for (int i = 0; i < treeWidget->topLevelItemCount(); ++i) {
    QTreeWidgetItem *item = treeWidget->topLevelItem(i);
    const QString itemYearStr = item->text(3);  // 第3列是年份

    if (itemYearStr == targetYearStr) {
      targetYearItems.append(item);
    }
  }

  // 保存目标年份的数据（如果有）
  if (!targetYearItems.isEmpty()) {
    saveYearData(targetYearStr, tabName, targetYearItems);
    qDebug() << "已保存" << targetYear << "年的数据（" << targetYearItems.size()
             << "条）";
  } else {
    qDebug() << targetYear << "年无数据可保存";
  }
}

// 设置数据存储目录
void DataManager::setDataDir(const QString &dataDir) {
  m_dataDir = dataDir;
  if (!m_dataDir.endsWith("/")) {
    m_dataDir += "/";
  }
}

// 辅助函数：将JSON数据解析并追加到树控件
void DataManager::appendJsonToTree(QTreeWidget *tree,
                                   const QByteArray &jsonData,
                                   const QString &expectedYear,
                                   const QString &expectedTab) {
  QJsonParseError parseErr;
  QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseErr);
  if (parseErr.error != QJsonParseError::NoError) {
    qWarning() << "JSON解析失败:" << parseErr.errorString();
    return;
  }

  QJsonObject root = doc.object();

  // 校验文件标识（年份和tab名称必须匹配）
  if (root["year"].toString() != expectedYear ||
      root["dataType"].toString() != expectedTab) {
    qWarning() << "文件标识不匹配，跳过该文件";
    return;
  }

  // 解析顶层节点
  QJsonArray topItems = root["topItems"].toArray();
  for (const QJsonValue &itemVal : topItems) {
    QJsonObject itemObj = itemVal.toObject();
    QTreeWidgetItem *topItem = new QTreeWidgetItem();

    // 填充顶层节点数据（对应原INI的4个字段）
    topItem->setText(0, itemObj["topDate"].toString());
    topItem->setText(1, itemObj["topFreq"].toString());
    topItem->setText(2, itemObj["topAmount"].toString());
    topItem->setText(3, itemObj["topYear"].toString());

    // 解析子节点
    QJsonArray childItems = itemObj["children"].toArray();
    for (const QJsonValue &childVal : childItems) {
      QJsonObject childObj = childVal.toObject();
      QTreeWidgetItem *childItem = new QTreeWidgetItem(topItem);

      // 填充子节点数据（对应原INI的4个字段）
      childItem->setText(0, childObj["childTime"].toString());
      childItem->setText(1, childObj["childAmount"].toString());
      childItem->setText(2, childObj["childDesc"].toString());
      childItem->setText(3, childObj["childDetails"].toString());
    }

    // 追加到树控件
    tree->addTopLevelItem(topItem);
  }
}

// 辅助函数：保存单一年份的数据到JSON文件（支持任意有效年份）
void DataManager::saveYearData(const QString &year, const QString &tabName,
                               const QVector<QTreeWidgetItem *> &items) {
  const QString fileName = QString("%1-%2.json").arg(year).arg(tabName);
  const QString tempPath = m_dataDir + fileName + ".tmp";  // 临时文件
  const QString targetPath = m_dataDir + fileName;         // 目标文件

  // 构建JSON根对象
  QJsonObject root;
  root["year"] = year;
  root["dataType"] = tabName;
  root["topCount"] = items.size();

  // 构建顶层节点数组
  QJsonArray topItemsArray;
  for (QTreeWidgetItem *item : items) {
    QJsonObject itemObj;

    // 存储顶层节点数据
    itemObj["topDate"] = item->text(0);
    itemObj["topFreq"] = item->text(1);
    itemObj["topAmount"] = item->text(2);
    itemObj["topYear"] = item->text(3);

    // 构建子节点数组
    QJsonArray childrenArray;
    for (int i = 0; i < item->childCount(); ++i) {
      QTreeWidgetItem *child = item->child(i);
      QJsonObject childObj;

      childObj["childTime"] = child->text(0);
      childObj["childAmount"] = child->text(1);
      childObj["childDesc"] = child->text(2);
      childObj["childDetails"] = child->text(3);

      childrenArray.append(childObj);
    }
    itemObj["children"] = childrenArray;
    topItemsArray.append(itemObj);
  }
  root["topItems"] = topItemsArray;

  // 写入临时文件
  QFile tempFile(tempPath);
  if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qWarning() << "无法创建临时文件:" << tempPath
               << "错误:" << tempFile.errorString();
    return;
  }

  // 写入格式化的JSON（便于调试）
  tempFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
  tempFile.close();

  // 原子性替换目标文件（确保写入完整后再替换）
  if (QFile::exists(targetPath)) {
    QFile::remove(targetPath);
  }
  if (!QFile::rename(tempPath, targetPath)) {
    qWarning() << "文件替换失败:" << tempPath << "→" << targetPath;
    QFile::remove(tempPath);  // 清理临时文件
  }
}
