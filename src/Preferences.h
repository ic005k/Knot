#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QDialog>
#include <QFontDatabase>
#include <QStyleHints>
#include <QToolButton>

#include "src/Comm/TextEditToolbar.h"

namespace Ui {
class Preferences;
}

class Preferences : public QDialog {
  Q_OBJECT

 public:
  explicit Preferences(QWidget *parent = nullptr);
  ~Preferences();
  Ui::Preferences *ui;

  TextEditToolbar *textToolbarPreferences;
  bool devMode = false;
  QString chkStyle;
  bool isFontChange = false;
  void saveOptions();

  void initOptions();

  void setBakStatus(bool status);
  bool getBakStatus();
  void setLatestAction(QString action);

  void appendBakFile(QString action, QString bakfile);

  QStringList getBakFilesList();

  void initCheckStatus();

  bool isOverUIFont();
  bool isOverReaderFont();
  void setDefaultFont(QString fontFamily);
  QString getDefaultFont();

  QString setFontDemoUI(QString customFontPath, QToolButton *btn, int fontSize);

  void setEncSyncStatusTip();

 protected:
  void keyReleaseEvent(QKeyEvent *event) override;

  bool eventFilter(QObject *watch, QEvent *evn) override;
  void closeEvent(QCloseEvent *event) override;
 public slots:

 private slots:
  void on_btnBack_clicked();

  void on_sliderFontSize_sliderMoved(int position);

  void on_btnCustomFont_clicked();

  void on_chkUIFont_clicked();

  void on_sliderFontSize_valueChanged(int value);

  void on_btnReStart_clicked();

  void on_chkDark_clicked(bool checked);

  void on_chkZip_clicked();

  void on_editPassword_textChanged(const QString &arg1);

  void on_editValidate_textChanged(const QString &arg1);

  void on_btnShowPassword_pressed();

  void on_btnShowPassword_released();

  void on_btnShowValidate_pressed();

  void on_btnShowValidate_released();

  void on_chkDark_clicked();

 private:
  QString iniBakFiles = "BakFiles.ini";
  void getCheckStatusChange();
  QString orgCustomFontText;
  QList<int> listCheckStatus;
  int readerFontID = 0;
  int uiFontID = 0;

  QFont::Weight uiFontWeight;

  QByteArray aes_key0 = "MySuperSecretKey1234567890";  // 长度不足32会自动处理
  QByteArray aes_iv0 = "InitializationVe";
};

#endif  // PREFERENCES_H
