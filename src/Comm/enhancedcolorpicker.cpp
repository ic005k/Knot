// enhancedcolorpicker.cpp
#include "enhancedcolorpicker.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QValidator>
#include <QtMath>

#include "ui_MainWindow.h"

extern Ui::MainWindow *mui;
extern bool isAndroid;

QString simpleLargeSliderStyle = R"(
        QSlider::groove:horizontal {
          border: 0px solid #bbb;
        }

        QSlider::sub-page:horizontal {
          background: #1ABC9C;
          border-radius: 0px;
          margin-top: 15px;
          margin-bottom: 15px;
        }

        QSlider::add-page:horizontal {
          background: rgb(100,100,100);
          border: 0px solid #777;
          border-radius: 0px;
          margin-top: 15px;
          margin-bottom: 15px;
        }

        QSlider::handle:horizontal {
          background: rgb(255,255,255);
          border: 1px solid rgba(10,10,10,255);
        border-image: url(":/res/qslider_btn.png");
          width: 25px;
          height: 15px;
          border-radius: 0px;
          margin-top: 5px;
          margin-bottom: 5px;
        }

        QSlider::handle:horizontal:hover {
          background: rgb(0,191,255);
          border: 1px solid rgba(25,25,25,255);
          border-radius: 0px;
        }

        QSlider::sub-page:horizontal:disabled {
          background: #bbb;
          border-color: #999;
        }

        QSlider::add-page:horizontal:disabled {
          background: #eee;
          border-color: #999;
        }

        QSlider::handle:horizontal:disabled {
          background: #eee;
          border: 1px solid #aaa;
          border-radius: 4px;
        }
    )";

// HueWheelWidget 实现
void HueWheelWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect rect = this->rect();
  QPoint center = rect.center();
  int radius = qMin(rect.width(), rect.height()) / 2 - 5;

  // 绘制色相环
  for (int i = 0; i < 360; ++i) {
    painter.save();
    painter.translate(center);
    painter.rotate(i);

    QColor color;
    color.setHsv(i, 255, 255);
    painter.setPen(QPen(color, 5));
    painter.drawLine(0, 0, radius, 0);

    painter.restore();
  }

  // 绘制当前色相指示器
  painter.save();
  painter.translate(center);
  painter.rotate(m_currentHue);

  painter.setPen(QPen(Qt::black, 2));
  painter.setBrush(Qt::white);
  painter.drawEllipse(QPoint(radius, 0), 6, 6);

  painter.restore();
}

void HueWheelWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    calculateHue(event->pos());
  }
}

void HueWheelWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    calculateHue(event->pos());
  }
}

void HueWheelWidget::calculateHue(const QPoint &pos) {
  QPoint center = rect().center();
  QPoint relativePos = pos - center;

  if (relativePos.isNull()) return;

  qreal angle = qAtan2(relativePos.y(), relativePos.x()) * 180 / M_PI;
  angle = fmod(angle + 360, 360);  // 转换为0-360度
  m_currentHue = static_cast<int>(angle);
  emit hueChanged(m_currentHue);
  update();
}

// SVSelectorWidget 实现
void SVSelectorWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  QRect rect = this->rect();

  // 创建饱和度-明度渐变
  QImage image(rect.size(), QImage::Format_ARGB32);
  for (int y = 0; y < rect.height(); ++y) {
    int value = 255 - (y * 255 / rect.height());
    for (int x = 0; x < rect.width(); ++x) {
      int saturation = x * 255 / rect.width();
      QColor color = QColor::fromHsv(m_hue, saturation, value);
      image.setPixel(x, y, color.rgba());
    }
  }

  painter.drawImage(rect.topLeft(), image);

  // 绘制当前选择点
  int x = m_saturation * rect.width() / 255;
  int y = (255 - m_value) * rect.height() / 255;

  painter.setPen(QPen(Qt::black, 2));
  painter.setBrush(Qt::white);
  painter.drawEllipse(QPoint(x, y), 5, 5);
}

void SVSelectorWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    calculateSV(event->pos());
  }
}

void SVSelectorWidget::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons() & Qt::LeftButton) {
    calculateSV(event->pos());
  }
}

void SVSelectorWidget::calculateSV(const QPoint &pos) {
  QRect rect = this->rect();
  int s = qBound(0, pos.x() * 255 / rect.width(), 255);
  int v = 255 - qBound(0, pos.y() * 255 / rect.height(), 255);

  m_saturation = s;
  m_value = v;
  emit svChanged(s, v);
  update();
}

// EnhancedColorPicker 实现
EnhancedColorPicker::EnhancedColorPicker(QWidget *parent,
                                         const QColor &initialColor)
    : QDialog(parent), m_currentColor(initialColor) {
  setWindowTitle(tr("选择颜色"));
  resize(500, 600);

  // 初始化颜色值
  m_currentHue = m_currentColor.hue();
  if (m_currentHue < 0) m_currentHue = 0;  // 处理黑白灰的情况
  m_currentSaturation = m_currentColor.saturation();
  m_currentValue = m_currentColor.value();
  m_currentAlpha = m_currentColor.alpha();

  // 初始化预设颜色
  initPresetColors();

  // 创建主布局
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(10);

  // 颜色预览
  m_colorPreview = new QLabel();
  m_colorPreview->setMinimumHeight(10);
  m_colorPreview->setMaximumHeight(220);
  m_colorPreview->setFixedWidth(220);
  mainLayout->addWidget(m_colorPreview, 0, Qt::AlignHCenter);

  // 颜色选择区域
  QVBoxLayout *colorSelectorLayout = new QVBoxLayout();
  mainLayout->addLayout(colorSelectorLayout, Qt::AlignHCenter);

  // 色相环
  m_hueWheel = new HueWheelWidget();
  colorSelectorLayout->addWidget(m_hueWheel, 0, Qt::AlignHCenter);

  // 饱和度和明度选择区域
  m_svSelector = new SVSelectorWidget();
  colorSelectorLayout->addWidget(m_svSelector, 0, Qt::AlignHCenter);

  // 预设颜色
  QLabel *presetLabel = new QLabel(tr("常用颜色:"));
  mainLayout->addWidget(presetLabel);
  presetLabel->hide();

  m_presetColorsWidget = new QWidget();
  QGridLayout *presetLayout = new QGridLayout(m_presetColorsWidget);
  presetLayout->setSpacing(5);

  for (int i = 0; i < m_presetColors.size(); ++i) {
    QPushButton *colorBtn = new QPushButton();
    colorBtn->setFixedSize(30, 30);
    colorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid #aaaaaa;")
            .arg(m_presetColors[i].name()));
    colorBtn->setProperty("color", m_presetColors[i].name());

    connect(colorBtn, &QPushButton::clicked, this,
            &EnhancedColorPicker::onPresetColorClicked);
    presetLayout->addWidget(colorBtn, i / 6, i % 6);
  }

  mainLayout->addWidget(m_presetColorsWidget);
  m_presetColorsWidget->hide();

  // 颜色控制滑块
  QGridLayout *controlsLayout = new QGridLayout();
  mainLayout->addLayout(controlsLayout);

  // HSV控制
  if (isShow) controlsLayout->addWidget(new QLabel(tr("色相:")), 0, 0);
  m_hueSlider = new QSlider(Qt::Horizontal);
  m_hueSlider->setRange(0, 359);
  controlsLayout->addWidget(m_hueSlider, 0, 1);
  m_hueSlider->hide();

  if (isShow) controlsLayout->addWidget(new QLabel(tr("饱和度:")), 1, 0);
  m_saturationSlider = new QSlider(Qt::Horizontal);
  m_saturationSlider->setRange(0, 255);
  controlsLayout->addWidget(m_saturationSlider, 1, 1);
  m_saturationSlider->hide();

  if (isShow) controlsLayout->addWidget(new QLabel(tr("明度:")), 2, 0);
  m_valueSlider = new QSlider(Qt::Horizontal);
  m_valueSlider->setRange(0, 255);
  controlsLayout->addWidget(m_valueSlider, 2, 1);
  m_valueSlider->hide();

  // RGB控制
  controlsLayout->addWidget(new QLabel(tr("Red:")), 3, 0);
  m_redSlider = new QSlider(Qt::Horizontal);
  m_redSlider->setRange(0, 255);
  controlsLayout->addWidget(m_redSlider, 3, 1);
  if (isAndroid) {
    m_redSlider->setMinimumHeight(35);
    m_redSlider->setStyleSheet(simpleLargeSliderStyle);
  }

  controlsLayout->addWidget(new QLabel(tr("Green:")), 4, 0);
  m_greenSlider = new QSlider(Qt::Horizontal);
  m_greenSlider->setRange(0, 255);
  controlsLayout->addWidget(m_greenSlider, 4, 1);
  if (isAndroid) {
    m_greenSlider->setMinimumHeight(35);
    m_greenSlider->setStyleSheet(simpleLargeSliderStyle);
  }

  controlsLayout->addWidget(new QLabel(tr("Blue:")), 5, 0);
  m_blueSlider = new QSlider(Qt::Horizontal);
  m_blueSlider->setRange(0, 255);
  controlsLayout->addWidget(m_blueSlider, 5, 1);
  if (isAndroid) {
    m_blueSlider->setMinimumHeight(35);
    m_blueSlider->setStyleSheet(simpleLargeSliderStyle);
  }

  // 透明度控制
  if (isShow) controlsLayout->addWidget(new QLabel(tr("透明度:")), 6, 0);
  m_alphaSlider = new QSlider(Qt::Horizontal);
  m_alphaSlider->setRange(0, 255);
  controlsLayout->addWidget(m_alphaSlider, 6, 1);
  m_alphaSlider->hide();

  // 十六进制输入
  controlsLayout->addWidget(new QLabel(tr("HEX:")), 7, 0);
  m_hexEdit = new QLineEdit();
  m_hexEdit->setValidator(new QRegularExpressionValidator(
      QRegularExpression("^#?[0-9A-Fa-f]{6,8}$"), this));
  controlsLayout->addWidget(m_hexEdit, 7, 1);

  // 按钮
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  mainLayout->addLayout(buttonLayout);

  QPushButton *okBtn = new QPushButton(tr("Ok"));
  QPushButton *cancelBtn = new QPushButton(tr("Cancel"));

  buttonLayout->addWidget(okBtn);
  buttonLayout->addWidget(cancelBtn);

  // 连接信号槽
  connect(m_hueSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onHueChanged);
  connect(m_saturationSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onSaturationChanged);
  connect(m_valueSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onValueChanged);
  connect(m_redSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onRedChanged);
  connect(m_greenSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onGreenChanged);
  connect(m_blueSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onBlueChanged);
  connect(m_alphaSlider, &QSlider::valueChanged, this,
          &EnhancedColorPicker::onAlphaChanged);
  connect(m_hexEdit, &QLineEdit::textChanged, this,
          &EnhancedColorPicker::onHexInputChanged);

  connect(m_hueWheel, &HueWheelWidget::hueChanged, m_hueSlider,
          &QSlider::setValue);

  connect(m_svSelector, &SVSelectorWidget::svChanged, this,
          &EnhancedColorPicker::onSvChanged);

  connect(okBtn, &QPushButton::clicked, this, &EnhancedColorPicker::onAccept);
  connect(cancelBtn, &QPushButton::clicked, this,
          &EnhancedColorPicker::onCancel);

  // 初始化控件
  updateControlsFromColor();
}

void EnhancedColorPicker::initPresetColors() {
  m_presetColors = {
      Qt::red,        Qt::green,       Qt::blue,       Qt::yellow,
      Qt::cyan,       Qt::magenta,     Qt::black,      Qt::white,
      Qt::gray,       Qt::darkRed,     Qt::darkGreen,  Qt::darkBlue,
      Qt::lightGray,  Qt::darkGray,    Qt::darkCyan,   Qt::darkMagenta,
      Qt::darkYellow, Qt::transparent, Qt::darkBlue,   Qt::lightGray,
      Qt::darkRed,    Qt::darkGreen,   Qt::darkYellow, Qt::darkCyan};
}

QColor EnhancedColorPicker::getColor(QWidget *parent, const QColor &initial,
                                     const QString &title) {
  EnhancedColorPicker dialog(parent, initial);
  dialog.setWindowTitle(title);
  if (dialog.exec() == QDialog::Accepted) {
    return dialog.selectedColor();
  }
  return initial;
}

void EnhancedColorPicker::onHueChanged(int hue) {
  updateColorFromHSV(hue, m_currentSaturation, m_currentValue);
  m_hueWheel->setCurrentHue(hue);
  m_svSelector->setHue(hue);
}

void EnhancedColorPicker::onSaturationChanged(int saturation) {
  updateColorFromHSV(m_currentHue, saturation, m_currentValue);
  m_svSelector->setSaturation(saturation);
}

void EnhancedColorPicker::onValueChanged(int value) {
  updateColorFromHSV(m_currentHue, m_currentSaturation, value);
  m_svSelector->setValue(value);
}

void EnhancedColorPicker::onRedChanged(int red) {
  updateColorFromRGB(red, m_currentColor.green(), m_currentColor.blue());
}

void EnhancedColorPicker::onGreenChanged(int green) {
  updateColorFromRGB(m_currentColor.red(), green, m_currentColor.blue());
}

void EnhancedColorPicker::onBlueChanged(int blue) {
  updateColorFromRGB(m_currentColor.red(), m_currentColor.green(), blue);
}

void EnhancedColorPicker::onAlphaChanged(int alpha) {
  m_currentAlpha = alpha;
  m_currentColor.setAlpha(alpha);
  m_colorPreview->setStyleSheet(
      QString("background-color: %1; border: 1px solid #cccccc;")
          .arg(m_currentColor.name(QColor::HexArgb)));
  m_hexEdit->setText(m_currentColor.name(QColor::HexArgb));
}

void EnhancedColorPicker::onHexInputChanged(const QString &text) {
  if (text.isEmpty()) return;

  QColor color(text);
  if (color.isValid()) {
    m_currentColor = color;
    m_currentHue = color.hue();
    if (m_currentHue < 0) m_currentHue = 0;
    m_currentSaturation = color.saturation();
    m_currentValue = color.value();
    m_currentAlpha = color.alpha();

    m_skipHexUpdate = true;  // 用户正在输入，跳过更新
    updateControlsFromColor();
    m_skipHexUpdate = false;  // 输入完成，恢复更新
  }
}

void EnhancedColorPicker::onSvChanged(int saturation, int value) {
  m_saturationSlider->setValue(saturation);
  m_valueSlider->setValue(value);
  updateColorFromHSV(m_currentHue, saturation, value);
}

void EnhancedColorPicker::onPresetColorClicked() {
  QPushButton *btn = qobject_cast<QPushButton *>(sender());
  if (btn) {
    QColor color(btn->property("color").toString());
    m_currentColor = color;
    m_currentHue = color.hue();
    if (m_currentHue < 0) m_currentHue = 0;
    m_currentSaturation = color.saturation();
    m_currentValue = color.value();
    m_currentAlpha = color.alpha();

    updateControlsFromColor();
  }
}

void EnhancedColorPicker::onAccept() { accept(); }

void EnhancedColorPicker::onCancel() { reject(); }

void EnhancedColorPicker::updateColorFromHSV(int hue, int saturation,
                                             int value) {
  m_currentHue = hue;
  m_currentSaturation = saturation;
  m_currentValue = value;

  m_currentColor.setHsv(hue, saturation, value, m_currentAlpha);
  updateControlsFromColor();
}

void EnhancedColorPicker::updateColorFromRGB(int red, int green, int blue) {
  m_currentColor.setRgb(red, green, blue, m_currentAlpha);

  m_currentHue = m_currentColor.hue();
  if (m_currentHue < 0) m_currentHue = 0;
  m_currentSaturation = m_currentColor.saturation();
  m_currentValue = m_currentColor.value();

  updateControlsFromColor();
}

void EnhancedColorPicker::updateControlsFromColor() {
  // 阻塞信号以避免循环更新
  bool blocked = m_hueSlider->blockSignals(true);

  // 更新滑块
  m_hueSlider->setValue(m_currentHue);
  m_saturationSlider->setValue(m_currentSaturation);
  m_valueSlider->setValue(m_currentValue);
  m_redSlider->setValue(m_currentColor.red());
  m_greenSlider->setValue(m_currentColor.green());
  m_blueSlider->setValue(m_currentColor.blue());
  m_alphaSlider->setValue(m_currentAlpha);

  // 更新自定义部件
  m_hueWheel->setCurrentHue(m_currentHue);
  m_svSelector->setHue(m_currentHue);
  m_svSelector->setSaturation(m_currentSaturation);
  m_svSelector->setValue(m_currentValue);

  // 更新十六进制输入
  if (!m_skipHexUpdate) {
    m_hexEdit->setText(m_currentColor.name(QColor::HexArgb));
  }

  // 更新预览
  m_colorPreview->setStyleSheet(
      QString("background-color: %1; border: 1px solid #cccccc;")
          .arg(m_currentColor.name(QColor::HexArgb)));

  // 恢复信号
  m_hueSlider->blockSignals(blocked);
}
