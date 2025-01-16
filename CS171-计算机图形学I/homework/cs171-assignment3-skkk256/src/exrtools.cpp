/**
 * @file exrtools.cpp
 * @author ShanghaiTech CS171 TAs
 * @brief A series of tools to help you ensure the correctness of your renderer
 * by comparing the result with the reference.
 * @version 0.1
 * @date 2023-07-16
 *
 * @copyright Copyright (c) 2023
 */
#include <tinyexr.h>

#include "rdr/rdr.h"
using namespace RDR_NAMESPACE_NAME;

static void printDebug(int argc, char **argv) {
  print(
      "Usage: {} [OPTIONS] <FILE1.exr> [<FILE2.exr>]\n"
      "  -a   <FILE1.exr>             Calculate the pixel-wise average\n"
      "  -mse <FILE1.exr> <FILE2.exr> Calculate the MSE difference\n",
      argv[0]);
}

static std::vector<Vec3f> loadExr(const fs::path &path) {
  int width       = 0;
  int height      = 0;
  float *out      = nullptr;
  const char *err = nullptr;

  int ret = LoadEXR(&out, &width, &height, path.string().c_str(), &err);
  if (ret != TINYEXR_SUCCESS) {
    const std::string &err_str = err;
    FreeEXRErrorMessage(err);
    throw std::runtime_error(
        format("Failed to load EXR {}: {}", path.string().c_str(), err_str));
  }

  std::vector<Vec3f> result;
  result.reserve(width * height);
  for (int pixelId = 0; pixelId < width * height; pixelId++) {
    int idx = pixelId * 4;
    result.emplace_back(out[idx + 0], out[idx + 1], out[idx + 2]);
  }

  free(out);
  return result;
}

int main(int argc, char **argv) {
  std::string_view option;
  auto get_pixel_average = [](const auto &result) {
    return std::reduce(result.begin(), result.end(), Vec3f(0, 0, 0),
               [](const Vec3f &a, const Vec3f &b) { return a + b; }) /
           result.size();
  };

  InitLogger(true);

  // Load the exr file
  if (argc == 1) goto print_debug;
  option = argv[1];

  try {
    if (option == "-a") {
      // Calculate the pixel-wise average
      if (argc != 3) goto print_debug;
      auto result        = loadExr(argv[2]);
      auto pixel_average = get_pixel_average(result);
      print("{:.5f} {:.5f} {:.5f}\n", pixel_average.x, pixel_average.y,
          pixel_average.z);
      goto succeed;
    } else if (option == "-mse") {
      // Calculate the MSE difference
      if (argc != 4) goto print_debug;
      auto v1 = loadExr(argv[2]);
      auto v2 = loadExr(argv[3]);
      if (v1.size() != v2.size())
        throw std::runtime_error(
            format("The linearized size of {} and {} are different", argv[2],
                argv[3]));
      // clang-format off
      auto mse = std::inner_product(
          v1.begin(), v1.end(), v2.begin(), 0.0f, std::plus<>(),
          [](const Vec3f &a, const Vec3f &b) { return SquareNorm(a - b); }) /
          v1.size() / 3;
      // clang-format on
      print("{:.5f}\n", mse);
      goto succeed;
    } else
      goto print_debug;
  } catch (std::exception &ex) {
    Exception_("{}", ex.what());
    return 1;
  }

print_debug:
  printDebug(argc, argv);
  return 0;

succeed:
  return 0;
}
