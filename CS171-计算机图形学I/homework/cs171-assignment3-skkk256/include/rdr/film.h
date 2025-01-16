#ifndef __FILM_H__
#define __FILM_H__

#include <string>
#include <vector>

#include "accel.h"
#include "rdr/rdr.h"
#include "rdr/rfilter.h"

RDR_NAMESPACE_BEGIN

class Film : public ConfigurableObject {
public:
  friend class FilmBlockView;

  // ++ Required by ConfigurableObject
  Film(const Properties &props);
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  // ++ Required by Object
  void preprocess(const PreprocessContext &context) override;
  // --

  Float getAspectRatio() const;
  Vec2i getResolution() const;

  // ++ Required by Object
  std::string toString() const override {
    return format("Film [resolution = {}]", resolution);
  }
  // --

  /// Clear all of the committed samples and pretend nothing happened
  void clear();

  /// Write the image to file or array with the given path name after performing
  /// normalization with filter weight
  void exportImageToArray(vector<Vec3f> &result) const;
  void exportImageToFile(const fs::path &path_name) const;

  /// Commit the sample to the film, where sample is represented by their
  /// absolute position on image
  void commitSample(const Vec2f &sample_pos, const Vec3f &measurement);
  void commitLightImageSplat(
      const Vec2f &sample_pos, const Vec3f &measurement);

  Vec3f &getPixel(int x, int y);
  const Vec3f &getPixel(int x, int y) const;
  const vector<Vec3f> &getRawData() const { return data; }
  Vec3f& getLightPixel(int x, int y) { return light_data[x + resolution.x * y]; }

private:
  ref<ReconstructionFilter> filter{nullptr};
  Vec2i resolution;
  vector<Vec3f> data, light_data;
  vector<Double> weight;

  // blockview-related
  uint32_t block_side_length;
  Vec2i block_resolution;
  vector<FilmBlockView> block_views;

  template <typename T>
  RDR_FORCEINLINE bool isInside(const Vec<T, 2> &pos) const {
    return pos.x >= 0 && pos.x < resolution.x && pos.y >= 0 &&
           pos.y < resolution.y;
  }

  RDR_FORCEINLINE Double &getWeight(int x, int y) {
    assert(x < resolution.x);
    assert(y < resolution.y);
    return weight[y * resolution.x + x];
  }

  template <typename T>
  void blockVisitor(const Vec2f &sample_pos, T visitor);
};

/// A subspan of Film. Can be used to commit samples or add lock
class FilmBlockView {
  using BoundType = TAABB<Vec2f>;

public:
  FilmBlockView(Film &film, Vec2u offset, Vec2u block_size)
      : film(film),
        offset(offset),
        block_size(block_size),
        local_lock(make_ref<std::mutex>()) {
    // Perform check
    const Vec2u discrete_bound = offset + block_size;
    assert(discrete_bound.x <= film.getResolution().x);
    assert(discrete_bound.y <= film.getResolution().y);

    // Compute the real bound
    const Vec2f real_offset = Cast<Float>(offset) + 0.5_F;
    bound.low_bnd           = real_offset - film.filter->getRadius();
    bound.upper_bnd =
        real_offset + Cast<Float>(block_size) + film.filter->getRadius();

    // Bound should be unioned with the film bound
    bound.unionWith(BoundType(Vec2f(0, 0), Cast<Float>(film.getResolution())));
  }

  RDR_FORCEINLINE const Vec3f &getPixel(int x, int y) const {
    return getPixel(x, y);
  }

  RDR_FORCEINLINE Vec3f &getPixel(int x, int y) {
    assert(x < block_size.x);
    assert(y < block_size.y);
    return film.getPixel(offset.x + x, offset.y + y);
  }

  RDR_FORCEINLINE Vec2u getOffset() const { return offset; }
  RDR_FORCEINLINE Vec2u getBlockSize() const { return block_size; }

  // The most important function of this class. Commit the sample to the film,
  // where sample is represented by their world position and the corresponding
  // measurement. Sample is not guaranteed to be in the block.
  void commitSample(const Vec2f &sample_pos, const Vec3f &measurement);
  void commitLightImageSplat(
      const Vec2f &sample_pos, const Vec3f &measurement);

protected:
  Film &film;
  Vec2u offset, block_size;
  ref<std::mutex> local_lock{nullptr};

  // The possible range that can contribute to this block. Might be outside of
  // the block itself
  BoundType bound;

  template <typename T>
  RDR_FORCEINLINE bool isInside(const Vec<T, 2> &pos) const {
    return pos.x >= offset.x && pos.x < offset.x + block_size.x &&
           pos.y >= offset.y && pos.y < offset.y + block_size.y;
  }

  template <typename T>
  RDR_FORCEINLINE bool isEffectiveSample(const Vec<T, 2> &pos) const {
    return pos.x >= bound.low_bnd.x && pos.x < bound.upper_bnd.x &&
           pos.y >= bound.low_bnd.y && pos.y < bound.upper_bnd.y;
  }
};

template <typename T>
void Film::blockVisitor(const Vec2f &sample_pos, T visitor) {
  if (!isInside(sample_pos)) return;
  const Vec2i block_index(std::floor(sample_pos.x / block_side_length),
      std::floor(sample_pos.y / block_side_length));
  const Float &filter_radius = filter->getRadius();
  const int &discrete_block_radius =
      std::ceil((filter_radius - 0.5) / block_side_length);
  for (int x = -discrete_block_radius; x <= discrete_block_radius; ++x) {
    for (int y = -discrete_block_radius; y <= discrete_block_radius; ++y) {
      const Vec2i &current_block_index = block_index + Vec2i(x, y);
      if (current_block_index.x < 0 ||
          current_block_index.x >= block_resolution.x ||
          current_block_index.y < 0 ||
          current_block_index.y >= block_resolution.y)
        continue;
      const int &block_index =
          current_block_index.x + current_block_index.y * block_resolution.x;
      visitor(block_views[block_index]);
    }
  }
}

RDR_REGISTER_CLASS(Film)

RDR_NAMESPACE_END

#endif
