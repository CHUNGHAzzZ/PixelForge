#include "App/Canvas/Common.hpp"

#include <opencv2/imgproc.hpp>

namespace PixelForge {

QImage imageFromMat(const cv::Mat &image)
{
    if (image.empty()) {
        return {};
    }

    cv::Mat converted;
    switch (image.channels()) {
    case 1:
        cv::cvtColor(image, converted, cv::COLOR_GRAY2RGBA);
        break;
    case 3:
        cv::cvtColor(image, converted, cv::COLOR_BGR2RGBA);
        break;
    case 4:
        cv::cvtColor(image, converted, cv::COLOR_BGRA2RGBA);
        break;
    default:
        return {};
    }

    return QImage(
               converted.data,
               converted.cols,
               converted.rows,
               static_cast<qsizetype>(converted.step),
               QImage::Format_RGBA8888)
        .copy();
}

}
