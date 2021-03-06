#ifndef QT_HUB_FILE_H
#define QT_HUB_FILE_H

#include <QObject>

class File : public QObject {
  Q_OBJECT
 public:
  File();
  QString m_text;
  qreal m_textPos;

  Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged)
  Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
  Q_PROPERTY(qreal textPos READ textPos WRITE setTextPos NOTIFY textPosChanged)

  QString source() const;
  void setSource(const QString &source);

  QString text() const;
  void setText(const QString &text);

  qreal textPos();
  void setTextPos(qreal &textPos);

  void setStr(QString);
 signals:
  void sourceChanged();
  void textChanged();
  void textPosChanged();

 public slots:
  void readFile();

 private slots:

 private:
  QString m_source;
};

#endif  // FILE_H
