#ifndef SPLASHTIMER_H
#define SPLASHTIMER_H

#include <QApplication>
#include <QColor>
#include <QFont>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QPolygon>
#include <QScreen>
#include <QSplashScreen>
#include <random>

/**
 * @brief Splash screen component
 * Function: Displays loading animation and prompt text with platform adaptation
 * Features: Cross-platform compatible, automatic DPI adaptation, Android
 * fullscreen support
 */
class SplashTimer : public QSplashScreen {
  Q_OBJECT

 public:
  /**
   * @brief Constructor
   * @param isAndroid - Whether running on Android platform
   * @param width - Default width for non-Android platforms
   * @param height - Default height for non-Android platforms
   * @param loadingText - Loading prompt text
   * @param flags - Window flags
   */
  explicit SplashTimer(
      bool isAndroid = false, int width = 600, int height = 300,
      const QString& loadingText = tr("Loading, please wait..."),
      Qt::WindowFlags flags = Qt::WindowFlags())
      : QSplashScreen(QPixmap(), flags),
        m_isAndroid(isAndroid),
        m_width(width),
        m_height(height),
        m_loadingText(loadingText) {
    init();
  }

  ~SplashTimer() override {}

 private:
  // Shape type definition (internal use)
  enum ShapeType { Triangle, Rectangle, Square, Circle, Ellipse };
  static constexpr int SHAPE_TYPE_COUNT = 5;

  // Member variables
  bool m_isAndroid;       // Whether running on Android
  int m_width;            // Default width for non-Android
  int m_height;           // Default height for non-Android
  QString m_loadingText;  // Loading prompt text
  qreal m_dpr;            // Device pixel ratio
  QSize m_targetSize;     // Target size (considering DPI and platform)
  QFont m_font;           // Display font
  QColor m_bgColor;       // Background color
  QColor m_textColor;     // Text color
  QPixmap m_basePixmap;   // Base background (caches shapes to avoid redrawing)

  // Initialize the component
  void init() {
    // Initialize device adaptation and size based on platform
    initSizeAndDpi();

    // Initialize styles
    initStyle();

    // Pre-draw base background (shapes are drawn only once)
    drawBaseBackground();

    // Initialize window properties
    setFixedSize(m_targetSize / m_dpr);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    // For Android, ensure fullscreen display
    if (m_isAndroid) {
      setWindowState(windowState() | Qt::WindowFullScreen);
    }

    // Initial display
    updateDisplay();
  }

  // Initialize size and DPI based on platform
  void initSizeAndDpi() {
    m_dpr = qApp->devicePixelRatio();

    // Get screen size and create fullscreen canvas for Android
    if (m_isAndroid) {
      QScreen* screen = QGuiApplication::primaryScreen();
      if (screen) {
        QRect screenGeometry = screen->geometry();
        m_targetSize = screenGeometry.size() * m_dpr;
      } else {
        // Fallback size if screen info isn't available
        m_targetSize = QSize(800 * m_dpr, 1280 * m_dpr);
      }
    } else {
      m_targetSize = QSize(m_width * m_dpr, m_height * m_dpr);
    }
  }

  // Initialize styles (colors, fonts, etc.)
  void initStyle() {
    // Color configuration
    m_bgColor = QColor(100, 100, 100);
    m_textColor = QColor(135, 206, 250);

    // Font configuration with platform adaptation
    m_font = font();
    if (m_isAndroid) {
      // Larger font for Android devices
      m_font.setPointSizeF(16 * m_dpr);
    } else {
      m_font.setPointSizeF(12 * m_dpr);
    }
    m_font.setBold(true);
  }

  // Draw base background (including all decorative shapes)
  void drawBaseBackground() {
    m_basePixmap = QPixmap(m_targetSize);
    m_basePixmap.fill(m_bgColor);

    QPainter painter(&m_basePixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    drawDecorativeShapes(painter);
  }

  // Draw decorative shapes with platform adaptation
  void drawDecorativeShapes(QPainter& painter) {
    // More shapes for Android larger screens
    const int shapeCount = m_isAndroid ? 200 : 100;
    const int minSize = static_cast<int>((m_isAndroid ? 6 : 4) * m_dpr);
    const int maxSize = static_cast<int>((m_isAndroid ? 40 : 30) * m_dpr);

    // Shape color list
    const QVector<QColor> colors = {
        QColor(220, 220, 220, 70), QColor(180, 180, 200, 60),
        QColor(200, 200, 220, 80), QColor(170, 190, 210, 50)};

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> sizeDist(minSize, maxSize);
    std::uniform_int_distribution<int> typeDist(0, SHAPE_TYPE_COUNT - 1);
    std::uniform_int_distribution<int> colorDist(0, colors.size() - 1);
    std::uniform_int_distribution<int> xDist(0, m_targetSize.width());
    std::uniform_int_distribution<int> yDist(0, m_targetSize.height());

    // Draw random shapes
    for (int i = 0; i < shapeCount; ++i) {
      const int x = xDist(gen);
      const int y = yDist(gen);
      const int s1 = sizeDist(gen);
      const int s2 = sizeDist(gen);
      const QColor color = colors[colorDist(gen)];

      painter.setBrush(QBrush(color));
      painter.setPen(Qt::NoPen);

      // Draw shapes based on random type
      switch (static_cast<ShapeType>(typeDist(gen))) {
        case Triangle:
          drawTriangle(painter, x, y, s1, s2);
          break;
        case Rectangle:
          painter.drawRect(x, y, s1, s2);
          break;
        case Square:
          painter.drawRect(x, y, s1, s1);
          break;
        case Circle:
          painter.drawEllipse(x, y, s1, s1);
          break;
        case Ellipse:
          painter.drawEllipse(x, y, s1, s2);
          break;
      }
    }
  }

  // Draw a triangle
  void drawTriangle(QPainter& painter, int x, int y, int s1, int s2) {
    QPolygon triangle;
    triangle << QPoint(x, y) << QPoint(x + s1, y - s2 / 2)
             << QPoint(x + s1 / 2, y - s1);
    painter.drawPolygon(triangle);
  }

  // Update display content (only show loading text)
  void updateDisplay() {
    // Copy base background (avoid redrawing shapes)
    QPixmap displayPix = m_basePixmap.copy();
    QPainter painter(&displayPix);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // Draw text
    painter.setFont(m_font);
    painter.setPen(m_textColor);

    // Display loading prompt text
    const QString text = m_loadingText;

    // Calculate text position (centered)
    QFontMetrics fm(m_font);
    QRect textRect = fm.boundingRect(displayPix.rect(), Qt::AlignCenter, text);
    textRect.moveCenter(displayPix.rect().center());

    // Clear previous text area (avoid overlap)
    // painter.fillRect(textRect, m_bgColor);
    // 用透明色清除文字区域（不显示底色）
    painter.fillRect(textRect, Qt::transparent);
    // Draw text
    painter.drawText(textRect, Qt::AlignCenter, text);

    painter.end();

    // Apply DPI and update display
    displayPix.setDevicePixelRatio(m_dpr);
    setPixmap(displayPix);

    // Force refresh
    repaint();
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents, 100);
  }
};

#endif  // SPLASHTIMER_H
