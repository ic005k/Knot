// enhancedcolorpicker.h
#ifndef ENHANCEDCOLORPICKER_H
#define ENHANCEDCOLORPICKER_H

#include <QColor>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QSlider>
#include <QWidget>

// 自定义色相环部件
class HueWheelWidget : public QWidget {
  Q_OBJECT
 public:
  explicit HueWheelWidget(QWidget *parent = nullptr) : QWidget(parent) {
    setFixedSize(220, 220);
    m_currentHue = 0;
  }

  int currentHue() const { return m_currentHue; }
  void setCurrentHue(int hue) {
    if (hue != m_currentHue) {
      m_currentHue = hue;
      update();
    }
  }

 signals:
  void hueChanged(int hue);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

 private:
  int m_currentHue;
  void calculateHue(const QPoint &pos);
};

// 自定义饱和度/明度选择部件
class SVSelectorWidget : public QWidget {
  Q_OBJECT
 public:
  explicit SVSelectorWidget(QWidget *parent = nullptr) : QWidget(parent) {
    setFixedWidth(220);
    setMinimumHeight(100);
    setMaximumHeight(220);

    m_hue = 0;
    m_saturation = 255;
    m_value = 255;
  }

  void setHue(int hue) {
    if (hue != m_hue) {
      m_hue = hue;
      update();
    }
  }

  void setSaturation(int saturation) {
    if (saturation != m_saturation) {
      m_saturation = saturation;
      update();
    }
  }

  void setValue(int value) {
    if (value != m_value) {
      m_value = value;
      update();
    }
  }

 signals:
  void svChanged(int saturation, int value);

 protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

 private:
  int m_hue;
  int m_saturation;
  int m_value;
  void calculateSV(const QPoint &pos);
};

class EnhancedColorPicker : public QDialog {
  Q_OBJECT
 public:
  explicit EnhancedColorPicker(QWidget *parent = nullptr,
                               const QColor &initialColor = Qt::white);
  QColor selectedColor() const { return m_currentColor; }
  static QColor getColor(QWidget *parent = nullptr,
                         const QColor &initial = Qt::white,
                         const QString &title = "Select Color");

 private slots:
  void onHueChanged(int hue);
  void onSaturationChanged(int saturation);
  void onValueChanged(int value);
  void onRedChanged(int red);
  void onGreenChanged(int green);
  void onBlueChanged(int blue);
  void onAlphaChanged(int alpha);
  void onHexInputChanged(const QString &text);
  void onSvChanged(int saturation, int value);
  void onPresetColorClicked();
  void onAccept();
  void onCancel();

 private:
  QColor m_currentColor;
  int m_currentHue;
  int m_currentSaturation;
  int m_currentValue;
  int m_currentAlpha;

  // UI组件
  QLabel *m_colorPreview;
  HueWheelWidget *m_hueWheel;
  SVSelectorWidget *m_svSelector;
  QWidget *m_presetColorsWidget;
  QSlider *m_hueSlider;
  QSlider *m_saturationSlider;
  QSlider *m_valueSlider;
  QSlider *m_redSlider;
  QSlider *m_greenSlider;
  QSlider *m_blueSlider;
  QSlider *m_alphaSlider;
  QLineEdit *m_hexEdit;

  QVector<QColor> m_presetColors;

  void updateControlsFromColor();
  void updateColorFromHSV(int hue, int saturation, int value);
  void updateColorFromRGB(int red, int green, int blue);
  void initPresetColors();

  bool isShow = false;

  bool m_skipHexUpdate = false;
};

#endif  // ENHANCEDCOLORPICKER_H
