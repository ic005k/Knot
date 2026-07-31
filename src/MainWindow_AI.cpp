#include "MainWindow.h"

void MainWindow::sendAiChatRequest(const AiSingleRecord& cfg,
                                   const QString& userQuestion) {
  QWidget* parentWnd = this;
  if (m_Preferences->isTestBtnClicked) {
    m_Preferences->isTestBtnClicked = false;
    parentWnd = m_Preferences;
  }

  if (m_NotesList->isAINoteRename) {
    parentWnd = m_NotesList->m_RenameNotes;
  }

  // 规范：发起提问前先执行连通校验，校验通过后再发正式对话
  // 先执行连通检测（异步，检测成功后再执行真实提问，这里做分层回调）
  QUrl url = buildAiApiUrl(cfg.endpoint);
  if (!url.isValid()) {
    safeCloseProgress();
    auto msg = std::make_unique<ShowMessage>(parentWnd);
    msg->showMsg(tr("Error"), tr("Endpoint URL invalid"), 1);
    return;
  }

  QNetworkRequest req(url);

  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization",
                   QString("Bearer %1").arg(cfg.apiKey).toUtf8());

  // 完整用户提问请求体，使用配置内自定义参数
  QJsonObject body;
  body["model"] = cfg.modelId;
  body["temperature"] = cfg.temperature;
  body["max_tokens"] = cfg.maxTokens;
  body["thinking_enable"] = false;
  QJsonArray messages;
  QJsonObject msgUser;
  msgUser["role"] = "user";
  msgUser["content"] = userQuestion;
  messages.append(msgUser);
  body["messages"] = messages;

  QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);
  QNetworkReply* reply = m_ainetMgr->post(req, postData);

  connect(
      reply, &QNetworkReply::finished, this,
      [this, reply, userQuestion, parentWnd]() {
        reply->deleteLater();
        QString reqUrl = reply->request().url().toString();
        if (reply->error() != QNetworkReply::NoError) {
          QString errMsg = reply->errorString();
          QString content =
              tr("Network Error") + ":\n%1\n" + tr("Request URL") + ":\n%2";
          content = content.arg(errMsg, reqUrl);
          safeCloseProgress();
          auto msg = std::make_unique<ShowMessage>(parentWnd);
          msg->showMsg(tr("Connect Failed"), content, 1);
          return;
        }
        // 此处增加业务逻辑：读取返回JSON、解析AI回答内容
        QByteArray rawResp = reply->readAll();
        // TODO：解析返回内容，自行实现界面渲染逻辑
        QJsonParseError parseError;
        // 转为JSON文档，捕获解析错误
        QJsonDocument doc = QJsonDocument::fromJson(rawResp, &parseError);

        // 1. 判断：返回内容不是合法JSON
        if (parseError.error != QJsonParseError::NoError) {
          QString errInfo = tr("Returned data is not valid JSON:\n%1")
                                .arg(parseError.errorString());
          safeCloseProgress();
          auto msg = std::make_unique<ShowMessage>(parentWnd);
          msg->showMsg(tr("Parse Failed"), errInfo, 1);
          return;
        }

        QJsonObject rootObj = doc.object();

        // 2. 判断：服务端返回业务错误（密钥无效、模型不存在、余额不足等）
        if (rootObj.contains("error")) {
          QJsonObject errObj = rootObj["error"].toObject();
          QString serverErr = errObj["message"].toString().trimmed();
          safeCloseProgress();
          auto msg = std::make_unique<ShowMessage>(parentWnd);
          msg->showMsg(tr("API Rejected"),
                       tr("Server Error:\n%1").arg(serverErr), 1);
          return;
        }

        // 3. 正常成功响应，提取AI回答
        QJsonArray choicesArr = rootObj["choices"].toArray();
        if (choicesArr.isEmpty()) {
          safeCloseProgress();
          auto msg = std::make_unique<ShowMessage>(parentWnd);
          msg->showMsg(tr("Success"), tr("AI returned empty content"), 1);
          return;
        }

        // 取第一条回复
        QJsonObject firstChoice = choicesArr.first().toObject();
        QJsonObject aiMsgObj = firstChoice["message"].toObject();
        QString aiReplyText = aiMsgObj["content"].toString().trimmed();

        // ========== 弹窗展示完整问答 ==========
        // 分段纯翻译文本，tr内部不含任何换行符
        QString part1 = tr("User Question");
        QString part2 = tr("AI Reply");

        // 外部拼接换行、占位符，tr只负责文字
        QString showBody;
        showBody += part1;
        showBody += ":\n\n%1\n\n";
        showBody += part2;
        showBody += ":\n\n%2";

        // qDebug() << aiReplyText;
        //  复制到系统剪贴板
        QClipboard* clip = QGuiApplication::clipboard();
        clip->setText(aiReplyText);

        m_Preferences->saveAIConfig();

        // 最后填充占位符
        showBody = showBody.arg(userQuestion, aiReplyText);

        safeCloseProgress();
        auto msg = std::make_unique<ShowMessage>(parentWnd);
        // msg->showMsg(tr("AI Response Completed"), showBody, 1);

        if (m_NotesList->isAINoteRename) {
          m_NotesList->isAINoteRename = false;
          m_MsgBox->ui->btnOk->setText(tr("Modify Title"));

          if (msg->showMsg(tr("AI Response Completed"), aiReplyText, 2)) {
            QTextEdit* edit =
                m_NotesList->m_RenameNotes->findChild<QTextEdit*>("renameEdit");
            if (edit) {
              edit->setText(aiReplyText);
            }
          }
        } else if (m_Reader->isAIReaderExplanation) {
          m_Reader->isAIReaderExplanation = false;
          m_MsgBox->ui->btnOk->setText(tr("Add Note"));
          if (msg->showMsg(tr("AI Response Completed"), aiReplyText, 2)) {
            m_Reader->addBookNote(aiReplyText);
          }
        } else if (m_Notes->isAIQA) {
          m_Notes->ui->editAnswer->setText(aiReplyText);
          m_Notes->ui->progQA->setRange(0, 100);
          m_Notes->isAIQA = false;
          m_Notes->ui->editQuestion->setEnabled(true);

        } else if (isAndroidAIQA) {
          isAndroidAIQA = false;
          m_Notes->appendAIResults(aiReplyText);
        } else

        {
          msg->showMsg(tr("AI Response Completed"), aiReplyText, 1);
        }
      });
}

void MainWindow::checkAiConnectivity(const AiSingleRecord& cfg,
                                     std::function<void()> onSuccess) {
  QUrl url = buildAiApiUrl(cfg.endpoint);
  if (!url.isValid()) {
    auto msg = std::make_unique<ShowMessage>(mw_one);
    msg->showMsg(tr("Error"), tr("Endpoint URL invalid"), 1);
    return;
  }
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  req.setRawHeader("Authorization",
                   QString("Bearer %1").arg(cfg.apiKey).toUtf8());

  QJsonObject body;
  body["model"] = cfg.modelId;
  body["max_tokens"] = 1;
  body["thinking_enable"] = false;
  QJsonArray messages;
  QJsonObject msgUser;
  msgUser["role"] = "user";
  msgUser["content"] = "hello";
  messages.append(msgUser);
  body["messages"] = messages;

  QByteArray postData = QJsonDocument(body).toJson(QJsonDocument::Compact);
  QNetworkReply* reply = m_ainetMgr->post(req, postData);

  connect(reply, &QNetworkReply::finished, mw_one,
          [this, reply, cfg, onSuccess]() {
            reply->deleteLater();
            if (!this->isVisible()) return;

            QString reqUrl = reply->request().url().toString();
            if (reply->error() != QNetworkReply::NoError) {
              QString errMsg = reply->errorString();
              QString content =
                  tr("Network Error") + ":\n%1\n" + tr("Request URL") + ":\n%2";
              content = content.arg(errMsg, reqUrl);
              auto msg = std::make_unique<ShowMessage>(mw_one);
              msg->showMsg(tr("Connect Failed"), content, 1);
              return;
            }
            // 连通测试弹窗提示
            // auto msg = std::make_unique<ShowMessage>(mw_one);
            // QString text = tr("Connection test passed!") + "\n" +
            //               tr("Model ID: %1").arg(cfg.modelId);
            // msg->showMsg(tr("Success"), text, 1);

            m_Preferences->saveAIConfig();

            // 连通成功，执行传入的回调函数
            if (onSuccess) onSuccess();
          });
}

void MainWindow::aiChatQuery(const QString& userQuestion) {
  // 读取界面配置
  QString ep = m_Preferences->ui->editEndpoint->text().trimmed();
  QString key = m_Preferences->ui->editAIKey->text().trimmed();
  QString mid = m_Preferences->ui->editAIModelID->text().trimmed();

  QWidget* parentWnd = this;
  if (m_Preferences->isTestBtnClicked) {
    parentWnd = m_Preferences;
  }

  if (m_NotesList->isAINoteRename) {
    parentWnd = m_NotesList->m_RenameNotes;
  }

  if (ep.isEmpty() || key.isEmpty() || mid.isEmpty()) {
    auto msg = std::make_unique<ShowMessage>(parentWnd);
    msg->showMsg(tr("Warning"),
                 tr("Endpoint / API Key / Model ID cannot be empty"), 1);
    return;
  }

  AiSingleRecord cfg;
  cfg.endpoint = ep;
  cfg.apiKey = key;
  cfg.modelId = mid;
  cfg.temperature = 0.1;
  cfg.timeoutSec = 10;
  cfg.maxTokens = 1024;

  if (!m_Notes->isAIQA && !isAndroidAIQA) {
    showProgress();
  }

  // 复用统一连通检测函数，连通成功后执行提问
  // checkAiConnectivity(cfg, [this, cfg, userQuestion]() {
  sendAiChatRequest(cfg, userQuestion);
  //});
}

QUrl MainWindow::buildAiApiUrl(const QString& rawEndpoint) {
  QUrl u(rawEndpoint);
  if (!u.isValid()) return QUrl();

  QString path = u.path();
  // 判断路径是否已经以 chat/completions 结尾
  bool fullPathReady =
      path.endsWith("chat/completions") || path.endsWith("chat/completions/");
  if (fullPathReady) {
    return u;
  }

  // 去除末尾多余斜杠再拼接标准接口路径
  while (path.endsWith("/")) path.chop(1);
  path += "/chat/completions";
  u.setPath(path);
  return u;
}

/**
 * @brief 更新向量状态指示灯
 * @param status 0=就绪, 1=正在进行, 2=错误
 */
void MainWindow::setVectorStatus(int status, int current, int total) {
  if (!isLocalAIModel) {
    mui->lblVectorStatus->hide();
    return;
  }

  mui->lblVectorStatus->show();
  mui->lblVectorStatus->setProperty("state", QString::number(status));

  // ✅ 核心：整体字号变小，圆点通过 <font> 标签单独放大
  // size="+2" 表示在基础字号上增加 2pt，可根据视觉效果微调为 +3 或 +4
  const QString dotHtml = "<font size=\"+2\">●</font>";
  const QString space = "&nbsp;";  // 使用 HTML 空格保证间距不被压缩

  if (status == 1 && total > 0) {
    double pct = 100.0 * current / total;
    mui->lblVectorStatus->setText(
        QString("%1%2(%3%)").arg(dotHtml).arg(space).arg(pct, 0, 'f', 1));
  } else {
    mui->lblVectorStatus->setText(dotHtml);
  }

  // ✅ QSS 中调小基础字号（例如从 11px 降到 9px）
  // color 属性会自动穿透到 <font> 标签内的圆点上
  const QString lightStyle = R"(
        QLabel#lblVectorStatus {
            font-size: 10px;         /* 👈 文字调小 */
            color: #cccccc;
            background: transparent;
        }
        QLabel#lblVectorStatus[state="1"] { color: #ff9800; }
        QLabel#lblVectorStatus[state="2"] { color: #f44336; }
    )";

  const QString darkStyle = R"(
        QLabel#lblVectorStatus {
            font-size: 10px;         /* 👈 文字调小 */
            color: #555555;
            background: transparent;
        }
        QLabel#lblVectorStatus[state="1"] { color: #ffb74d; }
        QLabel#lblVectorStatus[state="2"] { color: #ef5350; }
    )";

  mui->lblVectorStatus->setStyleSheet(isDark ? darkStyle : lightStyle);

  mui->lblVectorStatus->style()->unpolish(mui->lblVectorStatus);
  mui->lblVectorStatus->style()->polish(mui->lblVectorStatus);
}