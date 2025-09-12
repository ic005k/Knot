#ifndef SPLASHTIMER_H
#define SPLASHTIMER_H

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSplashScreen>

class SplashTimer : public QSplashScreen {
  Q_OBJECT

 public:
  explicit SplashTimer(bool isAndroid = false, int width = 600,
                       int height = 300,
                       Qt::WindowFlags flags = Qt::WindowFlags())
      : QSplashScreen(QPixmap(), flags),
        m_isAndroid(isAndroid),
        m_width(width),
        m_height(height) {
    init();
  }

  ~SplashTimer() override {}

 private:
  bool m_isAndroid;      // 是否为安卓平台
  int m_width;           // 非安卓平台默认宽度
  int m_height;          // 非安卓平台默认高度
  qreal m_dpr;           // 设备像素比
  QSize m_targetSize;    // 目标尺寸（考虑DPI和平台）
  QFont m_font;          // 显示字体
  QColor m_bgColor;      // 背景色（同时作为文字内部颜色）
  QColor m_strokeColor;  // 描边色（空心字边缘，区别于背景）
  QPixmap m_basePixmap;  // 基础背景

  void init() {
    initSizeAndDpi();
    initStyle();
    drawBaseBackground();

    setFixedSize(m_targetSize / m_dpr);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    if (m_isAndroid) {
      setWindowState(windowState() | Qt::WindowFullScreen);
    }

    updateDisplay();
  }

  void initSizeAndDpi() {
    m_dpr = qApp->devicePixelRatio();

    if (m_isAndroid) {
      QScreen* screen = QGuiApplication::primaryScreen();
      if (screen) {
        QRect screenGeometry = screen->geometry();
        m_targetSize = screenGeometry.size() * m_dpr;
      } else {
        m_targetSize = QSize(800 * m_dpr, 1280 * m_dpr);
      }
    } else {
      m_targetSize = QSize(m_width * m_dpr, m_height * m_dpr);
    }
  }

  void initStyle() {
    // 背景色：浅灰色
    m_bgColor = QColor(200, 200, 200);
    // 描边色：深灰色（与背景对比明显）
    m_strokeColor = QColor(60, 60, 60);

    // 关键修改：设置跨平台通用无衬线字体
    m_font.setFamily(
        "Arial, Helvetica, sans-serif");  // 字体优先级：Arial > Helvetica >
                                          // 系统默认无衬线
    m_font.setBold(true);
    m_font.setStyleStrategy(
        QFont::PreferAntialias);  // 优先抗锯齿，使轮廓更平滑

    // 字体大小调整（根据字体特性微调）
    if (m_isAndroid) {
      m_font.setPointSizeF(110 * m_dpr);  // 适当减小尺寸，避免轮廓重叠
    } else {
      m_font.setPointSizeF(80 * m_dpr);
    }
  }

  void drawBaseBackground() {
    m_basePixmap = QPixmap(m_targetSize);
    m_basePixmap.fill(m_bgColor);
  }

  void updateDisplay() {
    QPixmap displayPix = m_basePixmap.copy();
    QPainter painter(&displayPix);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setFont(m_font);

    if (m_isAndroid) {
      drawVerticalKNOT(painter, displayPix);
    } else {
      drawHorizontalKNOT(painter, displayPix);
    }

    painter.end();

    displayPix.setDevicePixelRatio(m_dpr);
    setPixmap(displayPix);

    repaint();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
  }

  // 水平绘制空心"KNOT"
  void drawHorizontalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString text = "KNOT";
    QFontMetricsF fm(m_font);
    QRectF textRect = fm.boundingRect(QRectF(), Qt::AlignCenter, text);

    QPointF center = displayPix.rect().center();
    QPointF textPos(center.x() - textRect.width() / 2,
                    center.y() + textRect.height() / 2 - fm.descent());

    QPainterPath path;
    path.addText(textPos, m_font, text);

    int penWidth = 2 * m_dpr;
    painter.strokePath(path, QPen(m_strokeColor, penWidth, Qt::SolidLine,
                                  Qt::RoundCap, Qt::RoundJoin));
  }

  // 垂直绘制空心"KNOT"（优化间距避免重叠）
  void drawVerticalKNOT(QPainter& painter, const QPixmap& displayPix) {
    const QString letters = "KNOT";
    QFontMetricsF fm(m_font);
    int letterHeight = fm.height();
    int spacing = letterHeight * 0.3;  // 增加间距，避免垂直方向轮廓重叠
    int totalHeight =
        letterHeight * letters.length() + spacing * (letters.length() - 1);
    qreal startY = (displayPix.height() - totalHeight) / 2 + fm.ascent();

    for (int i = 0; i < letters.length(); ++i) {
      QString letter = letters.at(i);
      qreal letterWidth = fm.horizontalAdvance(letter);
      qreal xPos = (displayPix.width() - letterWidth) / 2;
      qreal yPos = startY + i * (letterHeight + spacing);

      QPainterPath path;
      path.addText(xPos, yPos, m_font, letter);

      int penWidth = 3 * m_dpr;  // 适当减细描边，减少重叠视觉效果
      painter.strokePath(path, QPen(m_strokeColor, penWidth, Qt::SolidLine,
                                    Qt::RoundCap, Qt::RoundJoin));
    }
  }
};

#endif  // SPLASHTIMER_H
