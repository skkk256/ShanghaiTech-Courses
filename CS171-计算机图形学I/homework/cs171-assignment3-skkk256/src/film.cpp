#include "rdr/film.h"

#include "rdr/platform.h"

/// Do not change the order of these includes
// clang-format off
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <tinyexr.h>
// clang-format on

RDR_NAMESPACE_BEGIN
/* ===================================================================== *
 *
 * TinyEXR related
 *
 * ===================================================================== */

// Example code from https://github.com/syoyo/tinyexr
static bool SaveEXR(
    const float *rgb, int width, int height, const char *outfilename) {
  EXRHeader header;
  InitEXRHeader(&header);

  EXRImage image;
  InitEXRImage(&image);

  image.num_channels = 3;

  vector<float> images[3];
  images[0].resize(width * height);
  images[1].resize(width * height);
  images[2].resize(width * height);

  // Split RGBRGBRGB... into R, G and B layer
  for (int i = 0; i < width * height; i++) {
    images[0][i] = rgb[3 * i + 0];
    images[1][i] = rgb[3 * i + 1];
    images[2][i] = rgb[3 * i + 2];
  }

  float *image_ptr[3];
  image_ptr[0] = &(images[2].at(0)); // B
  image_ptr[1] = &(images[1].at(0)); // G
  image_ptr[2] = &(images[0].at(0)); // R

  image.images = (unsigned char **)image_ptr;
  image.width  = width;
  image.height = height;

  header.num_channels = 3;
  header.channels     =
      (EXRChannelInfo *)malloc(sizeof(EXRChannelInfo) * header.num_channels);
  // Must be (A)BGR order, since most of EXR viewers expect this channel order.
  strncpy(header.channels[0].name, "B", 255);
  header.channels[0].name[strlen("B")] = '\0';
  strncpy(header.channels[1].name, "G", 255);
  header.channels[1].name[strlen("G")] = '\0';
  strncpy(header.channels[2].name, "R", 255);
  header.channels[2].name[strlen("R")] = '\0';

  header.pixel_types = (int *)malloc(sizeof(int) * header.num_channels);
  header.requested_pixel_types =
      (int *)malloc(sizeof(int) * header.num_channels);
  for (int i = 0; i < header.num_channels; i++) {
    header.pixel_types[i] =
        TINYEXR_PIXELTYPE_FLOAT; // pixel type of input image
    header.requested_pixel_types[i] =
        TINYEXR_PIXELTYPE_HALF; // pixel type of output image to be stored in
    // .EXR
  }

  const char *err = nullptr; // or nullptr in C++11 or later.
  int ret         = SaveEXRImageToFile(&image, &header, outfilename, &err);
  if (ret != TINYEXR_SUCCESS) {
    Exception_("Error ocurred when saving EXR result: {}", err);
    FreeEXRErrorMessage(err); // free's buffer for an error message
    return ret;
  }

  Info_("EXR result saved to [ {} ]", outfilename);

  free(header.channels);
  free(header.pixel_types);
  free(header.requested_pixel_types);
  return true;
}

/* ===================================================================== *
 *
 *  Film Implementations
 *
 * ===================================================================== */

Film::Film(const Properties &props)
  : resolution(props.getProperty<Vec2i>("resolution", Vec2i(600, 600))),
    block_side_length(props.getProperty<int>("block_side_length", 16)),
    data(resolution.x * resolution.y),
    weight(resolution.x * resolution.y),
    light_data(resolution.x * resolution.y) {
  if (block_side_length <= 0) {
    Exception_("block side length should be greater equal than 1");
  }
}

void Film::crossConfiguration(const CrossConfigurationContext &context) {
  filter = context.filter;
}

void Film::preprocess(const PreprocessContext &context) {
  // build the FilmBlockView table
  block_resolution.x = std::ceil(
      static_cast<Float>(resolution.x) / static_cast<Float>(block_side_length));
  block_resolution.y = std::ceil(
      static_cast<Float>(resolution.y) / static_cast<Float>(block_side_length));
  block_views.reserve(block_resolution.x * block_resolution.y);

  // Notice the traverse order
  for (uint32_t y = 0; y < resolution.y; y += block_side_length) {
    for (uint32_t x = 0; x < resolution.x; x += block_side_length) {
      const uint32_t block_width  = Min(block_side_length, resolution.x - x);
      const uint32_t block_height = Min(block_side_length, resolution.y - y);

      FilmBlockView block_view(*this, {x, y}, {block_width, block_height});
      block_views.push_back(block_view);
    }
  }
}

Float Film::getAspectRatio() const {
  return static_cast<Float>(resolution.x) / static_cast<Float>(resolution.y);
}

Vec2i Film::getResolution() const {
  return resolution;
}

Vec3f &Film::getPixel(int x, int y) {
  return data[x + resolution.x * y];
}

const Vec3f &Film::getPixel(int x, int y) const {
  return data[x + resolution.x * y];
}

void Film::clear() {
  std::fill(data.begin(), data.end(), Vec3f(0.0));
  std::fill(weight.begin(), weight.end(), 0.0);
  std::fill(light_data.begin(), light_data.end(), Vec3f(0.0));
}

void Film::exportImageToArray(vector<Vec3f> &result) const {
  result.resize(resolution.x * resolution.y);
  for (int i  = 0; i < data.size(); i++)
    result[i] = Cast<Float>(data[i] / weight[i]) + light_data[i];
}

void Film::exportImageToFile(const fs::path &path_name) const {
  const std::string file_name = path_name.string();
  const auto ext              = path_name.extension();
  const size_t hprod          = static_cast<const size_t>(
    resolution.x * resolution.y * 3);

  if (ext == ".png") {
    Info_("Exporting PNG file [ {} ]...", path_name.string());
    Warn_(
        "PNG result is not recommended for debugging correctness. Consider "
        "using EXR instead.");
    vector<uint8_t> rgb_data(hprod);
    for (int i = 0; i < data.size(); i++) {
      auto const &filter_weight = weight[i];

      Float col_x = NAN;
      Float col_y = NAN;
      Float col_z = NAN;

      if (filter_weight == 0.0) {
        col_x = light_data[i].x;
        col_y = light_data[i].y;
        col_z = light_data[i].z;
      } else {
        col_x = data[i].x / filter_weight + light_data[i].x;
        col_y = data[i].y / filter_weight + light_data[i].y;
        col_z = data[i].z / filter_weight + light_data[i].z;
      }

      rgb_data[3 * i]     = GammaCorrection(col_x);
      rgb_data[3 * i + 1] = GammaCorrection(col_y);
      rgb_data[3 * i + 2] = GammaCorrection(col_z);
    }

    stbi_flip_vertically_on_write(1);
    stbi_write_png(
        file_name.c_str(), resolution.x, resolution.y, 3, rgb_data.data(), 0);
  } else if (ext == ".exr") {
    Info_("Exporting EXR file [ {} ]...", path_name.string());
    vector<float> rgb_data(hprod);

    for (int y = 0; y < resolution.y; y++) {
      for (int x = 0; x < resolution.x; x++) {
        int flipped_y     = resolution.y - 1 - y;
        int index         = x + y * resolution.x;
        int flipped_index = x + flipped_y * resolution.x;

        const Float &filter_weight = weight[index];
        if (filter_weight == 0) {
          rgb_data[3 * flipped_index]     = light_data[index].x;
          rgb_data[3 * flipped_index + 1] = light_data[index].y;
          rgb_data[3 * flipped_index + 2] = light_data[index].z;
        } else {
          rgb_data[3 * flipped_index] =
              data[index].x / filter_weight + light_data[index].x;
          rgb_data[3 * flipped_index + 1] =
              data[index].y / filter_weight + light_data[index].y;
          rgb_data[3 * flipped_index + 2] =
              data[index].z / filter_weight + light_data[index].z;
        } // end of if
      }   // end of x
    }     // end of y

    // Execute the SaveEXR to save the result
    SaveEXR(rgb_data.data(), resolution.x, resolution.y, file_name.c_str());
  } else {
    Exception_("Image extension [ {} ] is not supported.", ext.string());
  }
}

void Film::commitSample(const Vec2f &sample_pos, const Vec3f &measurement) {
  if (!isInside(sample_pos)) return;
  blockVisitor(
      sample_pos, [sample_pos, measurement](FilmBlockView &block_view) -> void {
        block_view.commitSample(sample_pos, measurement);
      });
}

void Film::commitLightImageSplat(
    const Vec2f &sample_pos, const Vec3f &measurement) {
  if (!isInside(sample_pos)) return;
  const Vec2i block_index_2d(std::floor(sample_pos.x / block_side_length),
      std::floor(sample_pos.y / block_side_length));
  const int &block_index =
      block_index_2d.x + block_index_2d.y * block_resolution.x;
  block_views[block_index].commitLightImageSplat(sample_pos, measurement);
}

// =======================================================================
// FilmBlockView Implementation
// =======================================================================

void FilmBlockView::commitSample(
    const Vec2f &sample_pos, const Vec3f &measurement) {
  if (!isEffectiveSample(sample_pos)) return;
  // lock the local block when committing the sample
  std::scoped_lock<std::mutex> lock(*local_lock);

  const auto &filter         = film.filter;
  const Float &filter_radius = filter->getRadius();
  const int &discrete_radius = std::ceil(filter_radius - 0.5);

  // Find the corresponding pixel
  AssertAllNonNegative(sample_pos.x, sample_pos.y);
  const Vec2i pixel_index(std::floor(sample_pos.x), std::floor(sample_pos.y));

  // traverse the pixels in the filter window
  for (int x = -discrete_radius; x <= discrete_radius; ++x) {
    for (int y = -discrete_radius; y <= discrete_radius; ++y) {
      const Vec2i &current_pixel_index = pixel_index + Vec2i(x, y);
      if (!isInside(current_pixel_index)) continue;
      const Vec2f &relative_pos = sample_pos -
                                  Cast<Float>(current_pixel_index) -
                                  static_cast<Float>(0.5);
      const Float &weight = filter->evaluate(relative_pos);
      film.getWeight(current_pixel_index.x, current_pixel_index.y) += weight;
      film.getPixel(current_pixel_index.x, current_pixel_index.y) +=
          measurement * weight;
    }
  }
}

void FilmBlockView::commitLightImageSplat(
    const Vec2f &sample_pos, const Vec3f &measurement) {
  if (!isEffectiveSample(sample_pos)) return;

  std::scoped_lock<std::mutex> lock(*local_lock);
  const Vec2i pixel_index(std::floor(sample_pos.x), std::floor(sample_pos.y));
  film.getLightPixel(pixel_index.x, pixel_index.y) += measurement;
  // unlocked
}

RDR_NAMESPACE_END
