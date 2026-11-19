#pragma once

#include <QImage>
#include <limits>
#include <vector>

namespace render {
class FrameBuffer {
public:
  void resize(int w, int h) {
    if (w == width() && h == height())
      return;
    image_ = QImage(w, h, QImage::Format_RGB32);
    depth_.assign(static_cast<size_t>(w) * h, kFar);
  }
  int width() const { return image_.width(); }
  int height() const { return image_.height(); }
  QImage &image() { return image_; }
  QRgb *colorRow(int y) { return reinterpret_cast<QRgb *>(image_.scanLine(y)); }
  float *depthRow(int y) {
    return depth_.data() + static_cast<size_t>(y) * width();
  }

  static constexpr float kFar = -std::numeric_limits<float>::infinity();

private:
  QImage image_;
  std::vector<float> depth_;
};

} // namespace render