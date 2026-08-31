#ifndef SEARCHWORKER_H
#define SEARCHWORKER_H

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

#include "src/defines.h"

// 前向声明或定义 SearchItem 结构体
/*struct SearchItem {
  QString tabName;
  QString strYear;
  QString weeks;
  QString day;
  QString strTime;
  QString txt1;
  QString txt2;
  QString txt3;
};*/

// 前向声明 Method 类
class Method;

class SearchWorker : public QObject {
  Q_OBJECT

 public:
  explicit SearchWorker(QObject* parent = nullptr)
      : QObject(parent), m_Method(nullptr) {}

  ~SearchWorker() override = default;

  QList<QString> resultsList;

  // 设置 Method 实例（用于高亮文本等功能）
  void setMethod(Method* method) { m_Method = method; }

 public slots:
  void startSearch(QList<SearchItem> data, const QString& searchStr) {
    QList<QString> localResults;

    for (const SearchItem& item : data) {
      QString tabStr = item.tabName;
      QString strYear = item.strYear;
      QString weeks = item.weeks;
      QString day = item.day;
      QString strTime = item.strTime;
      QString txt1 = item.txt1;
      QString txt2 = item.txt2;
      QString txt3 = item.txt3;

      QString txt0;
      if (strTime.split(".").count() == 2) {
        txt0 = strYear + " " + day + " " + weeks + " " +
               strTime.split(".").at(1).trimmed();
      }

      QStringList list;
      bool isYes = false;

      if (searchStr.contains("&")) {
        // ===== 多关键词搜索（用 & 分隔） =====
        list = searchStr.split("&");
        bool is0 = false, is1 = false, is2 = false, is3 = false;

        for (int n = 0; n < list.count(); n++) {
          QString str = list.at(n).trimmed();

          if (str.length() > 0) {
            if (strYear.contains(str) || day.contains(str) ||
                weeks.contains(str)) {
              is0 = true;
              txt0 = m_Method->highlightTextInHtml(txt0, str);
            }
            if (txt1.contains(str)) {
              is1 = true;
              txt1 = m_Method->highlightTextInHtml(txt1, str);
            }
            if (txt2.contains(str)) {
              is2 = true;
              txt2 = m_Method->highlightTextInHtml(txt2, str);
            }
            if (txt3.contains(str)) {
              is3 = true;
              txt3 = m_Method->highlightTextInHtml(txt3, str);
            }
          }
        }

        // 判断是否满足"任意N个字段匹配"的条件
        if (list.count() == 2) {
          if (is0 && is1) isYes = true;
          if (is0 && is2) isYes = true;
          if (is0 && is3) isYes = true;
          if (is1 && is2) isYes = true;
          if (is1 && is3) isYes = true;
          if (is2 && is3) isYes = true;
        }

        if (list.count() == 3) {
          if (is0 && is1 && is2) isYes = true;
          if (is0 && is1 && is3) isYes = true;
          if (is0 && is2 && is3) isYes = true;
          if (is1 && is2 && is3) isYes = true;
        }

        if (list.count() >= 4) {
          if (is0 && is1 && is2 && is3) isYes = true;
        }

        // 二次校验：确保所有关键词都至少出现在某个字段中
        QString s_total = txt0 + txt1 + txt2 + txt3;
        int n_count = 0;
        for (int x = 0; x < list.count(); x++) {
          QString str = list.at(x);
          if (str.length() > 0) {
            if (s_total.contains(str)) {
              n_count++;
            }
          }
        }

        if (isYes) {
          if (n_count < list.count()) isYes = false;
        }

      } else {
        // ===== 单关键词搜索 =====
        if (txt1.contains(searchStr) || txt2.contains(searchStr) ||
            txt3.contains(searchStr)) {
          isYes = true;

          if (txt1.contains(searchStr)) {
            txt1 = m_Method->highlightTextInHtml(txt1, searchStr);
          }
          if (txt2.contains(searchStr)) {
            txt2 = m_Method->highlightTextInHtml(txt2, searchStr);
          }
          if (txt3.contains(searchStr)) {
            txt3 = m_Method->highlightTextInHtml(txt3, searchStr);
          }
        }
      }

      if (isYes) {
        localResults.append(tabStr + "=|=" + txt0 + "=|=" + txt1 +
                            "=|=" + txt2 + "=|=" + txt3);
      }
    }

    emit searchFinished(localResults);
  }

 signals:
  void searchFinished(const QList<QString>& results);

 private:
  Method* m_Method;
};

#endif  // SEARCHWORKER_H