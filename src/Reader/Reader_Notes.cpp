#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <map>  // 用于页码排序

#include "Reader.h"
#include "src/MainWindow.h"

void Reader::addBookNote(const QString& preFillText) {}

void Reader::editBookNote(int index, int page, const QString& content) {
  if (dlgEditBookNote != nullptr) {
    dlgEditBookNote->close();
    dlgEditBookNote->deleteLater();
    dlgEditBookNote = nullptr;
  }

  dlgEditBookNote = new QDialog(mw_one);
  int dlgh = mw_one->geometry().height() / 2;
  if (dlgh < 350) dlgh = 350;
  dlgEditBookNote->setFixedSize(mw_one->geometry().width() - 2, dlgh);
  dlgEditBookNote->setWindowTitle(tr("Note"));

  QTextEdit* textEdit = new QTextEdit(dlgEditBookNote);
  textEdit->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);

  initTextToolbarDynamic(dlgEditBookNote);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, dlgEditBookNote);
  editFilter->setParent(dlgEditBookNote);
  textEdit->installEventFilter(editFilter);
  textEdit->viewport()->installEventFilter(editFilter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlgEditBookNote);
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dlgEditBookNote,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dlgEditBookNote,
                   &QDialog::reject);

  connect(dlgEditBookNote, &QDialog::finished, this, [](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
  });

  QVBoxLayout* vlayout = new QVBoxLayout(dlgEditBookNote);
  QHBoxLayout* layout = new QHBoxLayout(dlgEditBookNote);
  vlayout->addWidget(textEdit);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);

  m_Method->set_ToolButtonStyle(dlgEditBookNote);

  //----------------------------------------------

  layout->setSpacing(10);

  // 创建按钮组，设置互斥
  QButtonGroup* btnGroup = new QButtonGroup(this);
  btnGroup->setExclusive(true);

  QList<QPushButton*> colorButtons;

  QStringList colorList = {
      "#8500FF00",  // 保留：半透明纯绿（基准色，辅助标记）
      "#85FF0000",  // 半透明纯红（重点提醒/错误标记，醒目不刺眼）
      "#850000FF",  // 半透明纯蓝（关键信息/重要补充，与红色对比强烈）
      "#85FFFF00",  // 半透明纯黄（核心高亮/荧光笔平替，视觉焦点）
      "#8500FFFF",  // 半透明纯青（特殊注释/技术细节，独特不相近）
      "#85FF00FF"   // 半透明纯洋红（个性化标记/主观标注，区分度拉满）
  };

  for (const QString& colorStr : colorList) {
    QPushButton* btn = new QPushButton(this);
    btn->setFixedSize(50, 50);
    btn->setCheckable(true);

    // 设置样式
    QString style = QString(R"(
    QPushButton {
        border: 2px outset #666;
        background-color: %1;
    }
    QPushButton:checked {
        border: 2px solid red;         /* 选中时用红色边框 */
        background-color: %1;
    }
    )")
                        .arg(colorStr);

    btn->setStyleSheet(style);

    // 把原始颜色字符串存到按钮属性里
    btn->setProperty("colorCode", colorStr);

    layout->addWidget(btn);
    btnGroup->addButton(btn);

    colorButtons.append(btn);
  }

  // 默认选中第一个按钮
  if (!colorButtons.isEmpty()) {
    colorButtons.first()->setChecked(true);
  }

  // 连接信号槽（获取选中的颜色）
  strColor = "#8500FF00";  //(默认)
  connect(btnGroup,
          QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this,
          [=](QAbstractButton* btn) {
            strColor = btn->property("colorCode").toString();
          });

  //-------------------------------------------------------------------

  QRect parentRect = mw_one->geometry();
  int x = parentRect.x() + (parentRect.width() - dlgEditBookNote->width()) / 2;
  int y = parentRect.y() + 1;
  dlgEditBookNote->move(x, y);

  textEdit->setPlainText(content);

  if (dlgEditBookNote->exec() == QDialog::Accepted) {
    QString noteText = textEdit->toPlainText();

    int mpage = 0;
    if (page == -1)
      mpage = cPage;
    else
      mpage = page;

    updateReadNote(mpage, index, noteText, strColor);

    // if (mui->qwViewBookNote->isVisible()) {
    //   modifyText2(currentNoteListIndex, noteText);
    // }

    qDebug() << "Note added:" << noteText;
  } else {
    qDebug() << "Note canceled.";
  }
}

void Reader::viewBookNote() {
  initBookNoteValue(-1, -1);

  appendNoteDataToQmlList();
}

void Reader::closeViewBookNote() {}

void Reader::appendNoteDataToQmlList() {
  // 清空旧数据
  notesModel->clear();

  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  QFile jsonFile(file);
  if (!jsonFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开文件:" << file;
    return;
  }

  QByteArray data = jsonFile.readAll();
  jsonFile.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qWarning() << "JSON 解析失败";
    return;
  }

  QJsonObject rootObj = doc.object();

  // 遍历所有页码
  for (auto it = rootObj.begin(); it != rootObj.end(); ++it) {
    int page = it.key().toInt();  // 页码
    QJsonArray notesArray = it.value().toArray();

    // 遍历该页的所有笔记
    for (int i = 0; i < notesArray.size(); ++i) {
      QJsonObject noteObj = notesArray[i].toObject();
      QString content = noteObj["content"].toString();
      QString quote = noteObj["quote"].toString();

      // 创建 item
      QStandardItem* item = new QStandardItem();

      // 存三个字段到不同角色
      item->setData(quote, Qt::UserRole + 1);    // quote
      item->setData(page, Qt::UserRole + 2);     // page
      item->setData(content, Qt::UserRole + 3);  // content
      item->setData(i, Qt::UserRole + 4);        // 保存 page 内索引

      notesModel->appendRow(item);
    }
  }
}

void Reader::saveReadNote(const QString& searchContext, const QString& keyword,
                          const QString& noteContent,
                          const QString& currentPage) {
  QString filePath = iniDir + "memo/readnote/" + currentBookName + ".json";
  QDir().mkpath(QFileInfo(filePath).path());

  // 1. 读取已有 JSON（带容错）
  QJsonObject root;
  if (QFile::exists(filePath)) {
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
      f.close();
      if (err.error == QJsonParseError::NoError && doc.isObject()) {
        root = doc.object();
      } else {
        qWarning() << "[Reader] JSON parse failed:" << err.errorString();
      }
    }
  }

  // 2. 构建笔记对象（字段名与参数名严格对齐）
  QJsonObject noteObj;
  qint64 ts = QDateTime::currentMSecsSinceEpoch();

  noteObj["id"] = ts;
  noteObj["time"] =
      QDateTime::fromMSecsSinceEpoch(ts).toString(Qt::ISODateWithMs);
  noteObj["currentPage"] = currentPage;  // ✅ 对齐参数名
  noteObj["color"] = QStringLiteral("#FFEB3B");
  noteObj["searchContext"] = searchContext;  // ✅ 对齐参数名
  noteObj["keyword"] = keyword;              // ✅ 对齐参数名
  noteObj["noteContent"] = noteContent;      // ✅ 对齐参数名

  // 3. 追加到对应页码数组
  QJsonArray pageArray = root.value(currentPage).toArray();
  pageArray.append(noteObj);
  root[currentPage] = pageArray;

  // 4. 原子写入防损坏
  QString tmpPath = filePath + ".tmp";
  QFile tmpFile(tmpPath);
  if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    tmpFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmpFile.flush();
    tmpFile.close();

    if (QFile::exists(filePath)) QFile::remove(filePath);
    QFile::rename(tmpPath, filePath);
  } else {
    qCritical() << "[Reader] Failed to write note:" << tmpPath;
  }
}

void Reader::updateReadNote(const QString& noteId, const QString& searchContext,
                            const QString& keyword,
                            const QString& noteContent) {
  QString filePath = iniDir + "memo/readnote/" + currentBookName + ".json";
  if (!QFile::exists(filePath)) {
    qWarning() << "[Reader] updateReadNote: file not found:" << filePath;
    return;
  }

  // ✅ Java传来的是字符串，JSON里存的是qint64，必须转成数值比较
  bool ok = false;
  qint64 targetId = noteId.toLongLong(&ok);
  if (!ok) {
    qWarning() << "[Reader] updateReadNote: invalid noteId format:" << noteId;
    return;
  }

  // 1. 读取已有 JSON
  QJsonObject root;
  QFile f(filePath);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
      root = doc.object();
    } else {
      qWarning() << "[Reader] JSON parse failed:" << err.errorString();
      return;
    }
  } else {
    qCritical() << "[Reader] Cannot open note file for update:" << filePath;
    return;
  }

  // 2. 遍历查找目标笔记并原地更新
  bool found = false;
  qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
  QString nowStr =
      QDateTime::fromMSecsSinceEpoch(nowMs).toString(Qt::ISODateWithMs);

  for (auto it = root.begin(); it != root.end(); ++it) {
    if (!it.value().isArray()) continue;

    QJsonArray pageArray = it.value().toArray();
    for (int i = 0; i < pageArray.size(); ++i) {
      QJsonObject obj = pageArray[i].toObject();

      // ✅ 使用 qint64 数值精确匹配 id
      if (obj["id"].toVariant().toLongLong() == targetId) {
        // 覆盖内容字段
        obj["searchContext"] = searchContext;
        obj["keyword"] = keyword;
        obj["noteContent"] = noteContent;

        // ✅ 直接覆盖 time 为当前编辑时间，简洁明了
        obj["time"] = nowStr;

        // ✅ QJsonValue 是值语义，修改后必须显式写回数组和根对象
        pageArray[i] = obj;
        root[it.key()] = pageArray;
        found = true;
        break;
      }
    }
    if (found) break;
  }

  if (!found) {
    qWarning() << "[Reader] updateReadNote: noteId not found:" << targetId;
    return;
  }

  // 3. 原子写入防损坏
  QString tmpPath = filePath + ".tmp";
  QFile tmpFile(tmpPath);
  if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    tmpFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmpFile.flush();
    tmpFile.close();

    if (QFile::exists(filePath)) QFile::remove(filePath);
    if (!QFile::rename(tmpPath, filePath)) {
      qCritical() << "[Reader] Failed to rename tmp file after update";
    }
  } else {
    qCritical() << "[Reader] Failed to write updated note:" << tmpPath;
  }
}

QStringList Reader::readReadNote() {
  QStringList result;
  QString filePath = iniDir + "memo/readnote/" + currentBookName + ".json";

  if (!QFile::exists(filePath)) {
    return result;  // 文件不存在，返回空数组
  }

  QFile f(filePath);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "[Reader] Failed to open readnote file:" << filePath;
    return result;
  }

  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
  f.close();

  if (err.error != QJsonParseError::NoError || !doc.isObject()) {
    qWarning() << "[Reader] JSON parse failed:" << err.errorString();
    return result;
  }

  QJsonObject root = doc.object();

  // ✅ 使用 std::map 自动按页码（数字大小）排序，防止安卓端列表乱序
  std::map<int, QJsonArray> sortedPages;
  for (auto it = root.begin(); it != root.end(); ++it) {
    bool isNum;
    int pageNum = it.key().toInt(&isNum);
    if (isNum && it.value().isArray()) {
      sortedPages[pageNum] = it.value().toArray();
    }
  }

  // 遍历排序后的页码，组装数据
  for (const auto& pair : sortedPages) {
    const QJsonArray& pageArray = pair.second;

    for (const QJsonValue& val : pageArray) {
      if (!val.isObject()) continue;
      QJsonObject note = val.toObject();

      // 1. 提取基础字段（带默认值防崩溃）
      QString id = QString::number(note["id"].toVariant().toLongLong());
      QString currentPage = note["currentPage"].toString();
      QString time = note["time"].toString();
      QString noteContent = note["noteContent"].toString();
      QString searchContext = note["searchContext"].toString();
      QString keyword = note["keyword"].toString();
      QString color = note["color"].toString("#FFEB3B");

      // 2. 生成高亮 HTML (处理大小写及多次出现)
      QString contextHtml = searchContext;
      if (!keyword.isEmpty() && !searchContext.isEmpty()) {
        // ✅ 关键改动：
        // 1. style 值用单引号包裹，避免双引号嵌套被 Android 解析器吞掉
        // 2. 加上 padding 和 border-radius，与安卓端已验证的样式一致
        // 3. color 用深色确保在浅色背景上可读
        QString highlightTag =
            QString(
                "<span style='background-color:%1; color:#000000; "
                "padding:2px; border-radius:2px;'>%2</span>")
                .arg(color, keyword);

        contextHtml.replace(keyword, highlightTag, Qt::CaseInsensitive);
      }

      // 3. 采用 === 拼接（共 8 个字段）
      // 顺序：id === 页码 === 时间 === HTML上下文 === 笔记内容 === 纯文本上下文
      // === 关键词 === 颜色
      QString item = QString("%1===%2===%3===%4===%5===%6===%7===%8")
                         .arg(id)
                         .arg(currentPage)
                         .arg(time)
                         .arg(contextHtml)
                         .arg(noteContent)
                         .arg(searchContext)
                         .arg(keyword)
                         .arg(color);

      result.append(item);
    }
  }

  m_Method->refreshJavaData("mPdfActivity", "showNoteListDialog",
                            "artifex/mupdf/mini/DocumentActivity", result);

  return result;
}

void Reader::delReadNote(const QString& noteId) {
  QString filePath = iniDir + "memo/readnote/" + currentBookName + ".json";
  if (!QFile::exists(filePath)) {
    qWarning() << "[Reader] delReadNote: file not found:" << filePath;
    return;
  }

  // ✅ Java传来的是字符串，JSON里存的是qint64，必须转成数值比较
  bool ok = false;
  qint64 targetId = noteId.toLongLong(&ok);
  if (!ok) {
    qWarning() << "[Reader] delReadNote: invalid noteId format:" << noteId;
    return;
  }

  // 1. 读取已有 JSON
  QJsonObject root;
  QFile f(filePath);
  if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    f.close();
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
      root = doc.object();
    } else {
      qWarning() << "[Reader] JSON parse failed:" << err.errorString();
      return;
    }
  } else {
    qCritical() << "[Reader] Cannot open note file for delete:" << filePath;
    return;
  }

  // 2. 遍历查找目标笔记并移除
  bool found = false;
  for (auto it = root.begin(); it != root.end(); ++it) {
    if (!it.value().isArray()) continue;

    QJsonArray pageArray = it.value().toArray();
    int originalSize = pageArray.size();

    // ✅ 使用 erase-remove 模式安全移除匹配项
    // 注意：不能用 range-for + removeAt，会导致索引错位
    for (int i = pageArray.size() - 1; i >= 0; --i) {
      QJsonObject obj = pageArray[i].toObject();
      if (obj["id"].toVariant().toLongLong() == targetId) {
        pageArray.removeAt(i);
        found = true;
        // id 是唯一的，找到后即可停止当前页的遍历
        break;
      }
    }

    // 有变动才写回
    if (pageArray.size() != originalSize) {
      if (pageArray.isEmpty()) {
        // ✅ 该页笔记已清空，移除整个页码 key，保持 JSON 整洁
        root.erase(it);
      } else {
        root[it.key()] = pageArray;
      }
      break;  // id 全局唯一，无需继续遍历其他页
    }
  }

  if (!found) {
    qWarning() << "[Reader] delReadNote: noteId not found:" << targetId;
    return;
  }

  // 3. 原子写入防损坏
  QString tmpPath = filePath + ".tmp";
  QFile tmpFile(tmpPath);
  if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    tmpFile.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    tmpFile.flush();
    tmpFile.close();

    if (QFile::exists(filePath)) QFile::remove(filePath);
    if (!QFile::rename(tmpPath, filePath)) {
      qCritical() << "[Reader] Failed to rename tmp file after delete";
    }
  } else {
    qCritical() << "[Reader] Failed to write note file after delete:"
                << tmpPath;
  }
}

void Reader::updateReadNote(int page, int index, const QString& content,
                            const QString& color) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 文件不存在则无法更新
  if (!QFile::exists(file)) {
    qDebug() << "Note file not exists:" << file;
    return;
  }

  // 读取 JSON 文件
  QFile f(file);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for reading:" << f.errorString();
    return;
  }

  QByteArray data = f.readAll();
  f.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qDebug() << "Failed to parse JSON";
    return;
  }

  QJsonObject root = doc.object();
  QString pageKey = QString::number(page);

  // 如果 page 不存在，直接返回
  if (!root.contains(pageKey)) {
    qDebug() << "No notes for page:" << page;
    return;
  }

  QJsonArray notesArray = root[pageKey].toArray();

  // 检查 index 是否有效
  if (index < 0 || index >= notesArray.size()) {
    qDebug() << "Invalid note index:" << index;
    return;
  }

  // 更新
  QJsonObject noteObj = notesArray[index].toObject();
  noteObj["content"] = content;
  noteObj["color"] = color;
  notesArray.replace(index, noteObj);

  // 写回 JSON
  root[pageKey] = notesArray;
  doc.setObject(root);

  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for writing:" << f.errorString();
    return;
  }

  f.write(doc.toJson(QJsonDocument::Indented));
  f.close();

  qDebug() << "Note at index" << index << "on page" << page
           << "has been updated.";

  // 刷新 QML 模型
  if (page == cPage) {
    readReadNote();
  }
}

void Reader::initBookNoteValue(int cindex, int cpage) {}

void Reader::setShowNoteValue(bool value) { isShowNote = value; }

void Reader::setNoteListCurrentIndexValue(int value) {
  currentNoteListIndex = value;
}