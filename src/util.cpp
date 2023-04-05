#include "util.hpp"

#include <opencv2/opencv.hpp>

void wait_for_key(const char key, const int delay ) {
  while (true) {
    const char c_code = cv::waitKey(delay);
    if (c_code == key)return;
  }
}
