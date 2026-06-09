#include "Reader.h"

void Reader::addBookNote() {
  if (mui->editSetText->text().trimmed() == "") return;

  if (dlgAddBookNote != nullptr) {
    dlgAddBookNote->close();
    dlgAddBookNote->deleteLater();
    dlgAddBookNote = nullptr;
  }

  dlgAddBookNote = new QDialog(mw_one);
  dlgAddBookNote->setFixedSize(mw_one->geometry().width() - 2, 350);
  dlgAddBookNote->setWindowTitle(tr("Note"));

  QTextEdit* textEdit = new QTextEdit(dlgAddBookNote);
  textEdit->verticalScrollBar()->setStyleSheet(m_Method->vsbarStyleBig);
  textEdit->setAcceptRichText(false);

  initTextToolbarDynamic(dlgAddBookNote);
  EditEventFilter* editFilter =
      new EditEventFilter(textToolbarDynamic, dlgAddBookNote);
  editFilter->setParent(dlgAddBookNote);
  textEdit->installEventFilter(editFilter);
  textEdit->viewport()->installEventFilter(editFilter);

  QDialogButtonBox* buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, dlgAddBookNote);
  buttonBox->button(QDialogButtonBox::Ok)->setText(tr("Ok"));
  buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Cancel"));

  QObject::connect(buttonBox, &QDialogButtonBox::accepted, dlgAddBookNote,
                   &QDialog::accept);
  QObject::connect(buttonBox, &QDialogButtonBox::rejected, dlgAddBookNote,
                   &QDialog::reject);

  connect(dlgAddBookNote, &QDialog::finished, this, [this](int result) {
    Q_UNUSED(result);
    closeTextToolBar();
  });

  QVBoxLayout* vlayout = new QVBoxLayout(dlgAddBookNote);
  QHBoxLayout* layout = new QHBoxLayout(dlgAddBookNote);
  vlayout->addWidget(textEdit);
  vlayout->addLayout(layout);
  vlayout->addWidget(buttonBox);

  m_Method->set_ToolButtonStyle(dlgAddBookNote);

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
  int x = parentRect.x() + (parentRect.width() - dlgAddBookNote->width()) / 2;
  int y = parentRect.y() + 1;
  dlgAddBookNote->move(x, y);

  if (dlgAddBookNote->exec() == QDialog::Accepted) {
    QString noteText = textEdit->toPlainText();
    // 在这里处理用户输入的笔记内容

    qDebug() << strColor;
    saveReadNote(cPage, startNote, endNote, strColor, noteText,
                 mui->editSetText->text().trimmed());
    readReadNote(cPage);

    qDebug() << "Note added:" << noteText;
  } else {
    qDebug() << "Note canceled.";
  }
}

void Reader::editBookNote(int index, int page, const QString& content) {
  if (dlgEditBookNote != nullptr) {
    dlgEditBookNote->close();
    dlgEditBookNote->deleteLater();
    dlgEditBookNote = nullptr;
  }

  dlgEditBookNote = new QDialog(mw_one);
  dlgEditBookNote->setFixedSize(mw_one->geometry().width() - 2, 350);
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

  connect(dlgEditBookNote, &QDialog::finished, this, [this](int result) {
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

    if (mui->qwViewBookNote->isVisible()) {
      modifyText2(currentNoteListIndex, noteText);
    }

    qDebug() << "Note added:" << noteText;
  } else {
    qDebug() << "Note canceled.";
  }
}

void Reader::viewBookNote() {
  if (mui->qwViewBookNote->source().isEmpty()) {
    mui->qwViewBookNote->rootContext()->setContextProperty("m_Reader",
                                                           m_Reader);
    mui->qwViewBookNote->rootContext()->setContextProperty("fontSize",
                                                           fontSize);
    mui->qwViewBookNote->rootContext()->setContextProperty("isDark", isDark);
    mui->qwViewBookNote->rootContext()->setContextProperty("notesModel",
                                                           notesModel);

    mui->qwViewBookNote->setSource(
        QUrl(QStringLiteral("qrc:/src/qmlsrc/viewbooknote.qml")));
  }

  initBookNoteValue(-1, -1);

  mui->qwReader->hide();
  mui->qwViewCate->hide();
  mui->qwBookmark->hide();
  mui->qwViewBookNote->show();
  appendNoteDataToQmlList();

  mui->btnBackReaderSet->click();
}

void Reader::closeViewBookNote() {
  if (mui->qwViewBookNote->isHidden()) return;
  mui->qwViewBookNote->hide();
  mui->qwReader->show();
}

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

void Reader::saveReadNote(int page, int start, int end, const QString& color,
                          const QString& content, const QString& quote) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 确保目录存在
  QDir().mkpath(QFileInfo(file).path());

  // 读取已有 JSON 数据
  QJsonDocument doc;
  if (QFile::exists(file)) {
    QFile f(file);
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QByteArray data = f.readAll();
      doc = QJsonDocument::fromJson(data);
      f.close();
    }
  }

  // 如果文件为空，创建一个空对象
  if (doc.isNull()) {
    doc.setObject(QJsonObject());
  }

  QJsonObject root = doc.object();

  // 获取 page 对应的数组（如果不存在则新建）
  QJsonArray pageArray;
  if (root.contains(QString::number(page))) {
    pageArray = root[QString::number(page)].toArray();
  }

  // 创建新的笔记对象
  QJsonObject noteObj;
  noteObj["start"] = start;
  noteObj["end"] = end;
  noteObj["color"] = color;
  noteObj["content"] = content;
  noteObj["quote"] = quote;
  noteObj["timestamp"] = QDateTime::currentMSecsSinceEpoch();  // 增加时间戳

  // 追加到数组
  pageArray.append(noteObj);

  // 更新 root 对象
  root[QString::number(page)] = pageArray;
  doc.setObject(root);

  // 写回文件
  QFile f(file);
  if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
  }
}

void Reader::readReadNote(int page) {
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  QFile f(file);
  if (!f.exists()) {
    qDebug() << "Note file not exists:" << file;
    emit notesLoaded(QVariantList());  // 发送空列表
    return;
  }

  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file:" << f.errorString();
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

  // 获取 page 对应的笔记数组
  if (root.contains(QString::number(page))) {
    QJsonArray pageArray = root[QString::number(page)].toArray();

    // 转换为 QVariantList，方便 QML 接收
    QVariantList notesList;
    for (int i = 0; i < pageArray.size(); ++i) {
      QJsonObject noteObj = pageArray[i].toObject();
      QVariantMap noteMap;
      noteMap["start"] = noteObj["start"].toInt();
      noteMap["end"] = noteObj["end"].toInt();
      noteMap["color"] = noteObj["color"].toString();
      noteMap["content"] = noteObj["content"].toString();
      notesList.append(noteMap);
    }

    // TODO: 将 notesList 传递给 QML 的 notesModel
    emit notesLoaded(notesList);
  } else {
    qDebug() << "No notes for page:" << page;
    emit notesLoaded(QVariantList());  // 发送空列表
  }
}

void Reader::delReadNote(int index) {
  int page = cPage;
  QString file = iniDir + "memo/readnote/" + currentBookName + ".json";

  // 如果文件不存在，直接返回
  if (!QFile::exists(file)) {
    qDebug() << "Note file not exists:" << file;
    return;
  }

  // 打开并读取 JSON
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

  // 检查 page 是否存在
  QString pageKey = QString::number(page);
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

  // 删除指定索引的笔记
  notesArray.removeAt(index);

  // 如果删除后该 page 没有笔记，可将其从 JSON 中移除（可选）
  if (notesArray.isEmpty()) {
    root.remove(pageKey);
  } else {
    root[pageKey] = notesArray;
  }

  // 写回文件
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Failed to open note file for writing:" << f.errorString();
    return;
  }

  doc.setObject(root);
  f.write(doc.toJson(QJsonDocument::Indented));
  f.close();

  qDebug() << "Note at index" << index << "on page" << page
           << "has been deleted.";

  // 刷新 QML 中的笔记模型(备选)
  // readReadNote(page);
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
    readReadNote(page);
  }
}

void Reader::initBookNoteValue(int cindex, int cpage) {
  QQuickItem* root = mui->qwViewBookNote->rootObject();
  if (!root) {
    qWarning("Error: QML root object not found!");
    return;
  }

  // 调用 QML 函数 initValue(cindex, cpage)
  QMetaObject::invokeMethod(root, "initValue",
                            Qt::DirectConnection,  // 连接方式：同步调用
                            Q_ARG(QVariant, cindex), Q_ARG(QVariant, cpage));
}

void Reader::setShowNoteValue(bool value) { isShowNote = value; }

void Reader::setNoteListCurrentIndexValue(int value) {
  currentNoteListIndex = value;
}