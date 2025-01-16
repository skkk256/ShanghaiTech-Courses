#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * Our camera model is characterized by a position, a forward direction, an up
 * direction, a right direction(all directionals are scaled with focal length),
 * a focal length and fov. The camera model is a pinhole camera model, with all
 * parameters in standard unit. Fov is y-axis fov.
 */
class Camera : public ConfigurableObject {
public:
  friend class Vertex;
  friend class BidirectionalPathIntegrator;

  // ++ Required by ConfigurableObject
  Camera(const Properties &props)
      : ConfigurableObject(props),
        position(props.getProperty<Vec3f>("position", Vec3f(0, 0, 1))),
        fov(props.getProperty<Float>("fov", 45)),
        focal_length(props.getProperty<Float>("focal_length", 1)) {}
  void crossConfiguration(const CrossConfigurationContext &context) override;
  // --

  /// The main interface of camera
  Ray generateRay(Float x, Float y) const noexcept;
  DifferentialRay generateDifferentialRay(Float x, Float y) const noexcept;

  /// Sampling-based interface
  Vec3f We(const Ray &ray, Vec2f *pixel) const;  // NOLINT
  Vec3f sampleWithRef(const Vec3f &ref, Vec2f *pixel, Float *pdf_w) const;
  void pdf(const Ray &ray, Float *pdf_a, Float *pdf_w) const;

  /// Getters
  Vec3f getPosition() const noexcept;
  Float getFov() const noexcept;
  Float getNormalizedAreaOfImage() const noexcept;
  ref<Film> &getFilm() noexcept;

  std::string toString() const override {
    return format(
        "Camera [\n"
        "  position     = {},\n"
        "  forward      = {},\n"
        "  up           = {},\n"
        "  right        = {},\n"
        "  focal_length = {},\n"
        "  fov          = {}\n"
        "]",
        position, forward, up, right, focal_length, fov);
  }

private:
  Vec3f position, forward, up, right;
  Float focal_length, fov, normalized_area;
  Mat3f inv_generation_matrix;

  ref<Film> film;

  // Configure with look_at and ref_up
  void lookAt(const Vec3f &look_at, const Vec3f &ref_up = {0, 1, 0});
};

RDR_REGISTER_CLASS(Camera)

RDR_NAMESPACE_END

#endif
