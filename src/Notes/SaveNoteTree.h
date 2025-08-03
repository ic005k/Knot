#ifndef BINARY_HANDLER_H
#define BINARY_HANDLER_H

#include <QDataStream>
#include <QFile>
#include <QTreeWidget>
#include <QtGlobal>  // 包含基本类型定义

class BinaryHandler {
 public:
  // 写入树形结构到二进制文件
  static bool writeToBinary(const QString& filePath, QTreeWidget* tw) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
      return false;
    }

    QDataStream out(&file);
    // 修正：使用QDataStream命名空间下的BigEndian
    out.setByteOrder(QDataStream::BigEndian);  // 这里是关键修正
    out.setVersion(QDataStream::Qt_6_5);

    // 写入文件头
    out << qint32('N' << 24 | 'o' << 16 | 't' << 8 | 'e');  // 标识"Note"
    out << quint16(0x0100);                                 // 版本号v1.0

    // 写入顶层项数量
    int topCount = tw->topLevelItemCount();
    out << quint32(topCount);

    // 逐个写入顶层项
    for (int i = 0; i < topCount; ++i) {
      QTreeWidgetItem* topItem = tw->topLevelItem(i);
      // 写入顶层项文本和颜色标记
      out << topItem->text(0);  // strtop
      out << topItem->text(2);  // strtopcolorflag

      // 写入子项数量
      int childCount = topItem->childCount();
      out << quint32(childCount);

      // 逐个写入子项
      for (int j = 0; j < childCount; ++j) {
        QTreeWidgetItem* childItem = topItem->child(j);
        QString strChild0 = childItem->text(0);
        QString strChild1 = childItem->text(1);

        out << strChild0;  // childItem->text(0)
        out << strChild1;  // childItem->text(1)

        // 如果子项有更深层次的子项（strChild1为空时）
        if (strChild1.isEmpty()) {
          int deepChildCount = childItem->childCount();
          out << quint32(deepChildCount);

          // 写入深层子项
          for (int n = 0; n < deepChildCount; ++n) {
            out << childItem->child(n)->text(0);  // strChild00
            out << childItem->child(n)->text(1);  // strChild11
          }
        } else {
          // 无深层子项，写入0作为标记
          out << quint32(0);
        }
      }
    }

    file.close();
    return true;
  }

  // 从二进制文件读取并重建树形结构
  static bool readFromBinary(const QString& filePath, QTreeWidget* tw) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
      return false;
    }

    QDataStream in(&file);
    // 修正：使用QDataStream命名空间下的BigEndian
    in.setByteOrder(QDataStream::BigEndian);  // 这里是关键修正
    in.setVersion(QDataStream::Qt_6_5);

    // 验证文件头
    qint32 identifier;
    quint16 version;
    in >> identifier >> version;
    if (identifier != ('N' << 24 | 'o' << 16 | 't' << 8 | 'e') ||
        version != 0x0100) {
      file.close();
      return false;  // 不是目标格式或版本不兼容
    }

    // 清空现有树形结构
    tw->clear();

    // 读取顶层项数量
    quint32 topCount;
    in >> topCount;

    // 重建顶层项
    for (int i = 0; i < topCount; ++i) {
      QString strtop, strtopcolorflag;
      in >> strtop >> strtopcolorflag;

      QTreeWidgetItem* topItem = new QTreeWidgetItem;
      topItem->setText(0, strtop);
      topItem->setText(2, strtopcolorflag);
      tw->addTopLevelItem(topItem);

      // 读取子项数量
      quint32 childCount;
      in >> childCount;

      // 重建子项
      for (int j = 0; j < childCount; ++j) {
        QString strChild0, strChild1;
        in >> strChild0 >> strChild1;

        QTreeWidgetItem* childItem = new QTreeWidgetItem;
        childItem->setText(0, strChild0);
        childItem->setText(1, strChild1);
        topItem->addChild(childItem);

        // 读取深层子项数量
        quint32 deepChildCount;
        in >> deepChildCount;

        // 重建深层子项
        for (int n = 0; n < deepChildCount; ++n) {
          QString strChild00, strChild11;
          in >> strChild00 >> strChild11;

          QTreeWidgetItem* deepChild = new QTreeWidgetItem;
          deepChild->setText(0, strChild00);
          deepChild->setText(1, strChild11);
          childItem->addChild(deepChild);
        }
      }
    }

    file.close();
    return true;
  }
};

#endif  // BINARY_HANDLER_H
