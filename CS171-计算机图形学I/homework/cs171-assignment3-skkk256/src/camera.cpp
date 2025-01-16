#include "rdr/camera.h"

#include "rdr/film.h"
#include "rdr/ray.h"

RDR_NAMESPACE_BEGIN

void Camera::crossConfiguration(const CrossConfigurationContext &context) {
  film = context.film;
  lookAt(properties.getProperty<Vec3f>("look_at", Vec3f(0, 0, 0)),
      properties.getProperty<Vec3f>("ref_up", Vec3f(0, 1, 0)));
  clearProperties();
}

Ray Camera::generateRay(Float dx, Float dy) const noexcept {
  const auto resolution = film->getResolution();

  // [-1, 1]
  dx = dx / static_cast<Float>(resolution.x) * 2 - 1;
  dy = dy / static_cast<Float>(resolution.y) * 2 - 1;
  return {position, Normalize(dx * right + dy * up + forward)};
}

DifferentialRay Camera::generateDifferentialRay(
    Float x, Float y) const noexcept {
  auto ray    = generateRay(x, y);
  auto dx_ray = generateRay(x + 1, y);
  auto dy_ray = generateRay(x, y + 1);
  return {std::move(ray), dx_ray, dy_ray};
}

//===----------------------------------------------------------------------===//
// You might neglect the implementations below if you are not working on
// bidirectional path tracing
//===----------------------------------------------------------------------===//

Vec3f Camera::We(const Ray &ray, Vec2f *pixel) const {
  const Float cos_theta = Dot(ray.direction, Normalize(forward));
  if (cos_theta < 0) return {0, 0, 0};

  // Solve for dx, dy
  Vec3f dxy = Mul(inv_generation_matrix, ray.direction);
  AssertAllPositive(dxy.z);
  dxy.xy() /= dxy.z;

  Float dx = dxy.x;
  Float dy = dxy.y;
  dx       = (dx + 1) / 2 * film->getResolution().x;
  dy       = (dy + 1) / 2 * film->getResolution().y;

  const Vec2f film_position = Vec2f(dx, dy);
  if (pixel != nullptr) *pixel = film_position;

  // Terminate the calculation if out-of-bound
  if (film_position.x < 0 || film_position.x > film->getResolution().x ||
      film_position.y < 0 || film_position.y > film->getResolution().y)
    return {0, 0, 0};

  // Only for pin-hole camera
  const Float cos2_theta = cos_theta * cos_theta;
  const Float result     = 1.0F / (normalized_area * cos2_theta * cos2_theta);
  return {result, result, result};
}

Vec3f Camera::sampleWithRef(
    const Vec3f &ref, Vec2f *pixel, Float *pdf_w) const {
  // Since it is a pin-hole camera, there is nothing to be sampled
  const Ray ray{position, Normalize(ref - position)};

  // p_w = d^2 / (A cos_theta)
  if (pdf_w != nullptr)
    *pdf_w =
        SquareNorm(ref - position) / (Dot(ray.direction, Normalize(forward)));
  return We(ray, pixel);
}

void Camera::pdf(const Ray &ray, Float *pdf_a, Float *pdf_w) const {
  const Float cos_theta = Dot(ray.direction, Normalize(forward));
  if (cos_theta < 0) {
    if (pdf_a != nullptr) *pdf_a = 0;
    if (pdf_w != nullptr) *pdf_w = 0;
    return;
  }

  if (pdf_a != nullptr) *pdf_a = 1;
  if (pdf_w != nullptr)
    *pdf_w = 1.0F / (normalized_area * cos_theta * cos_theta * cos_theta);
}

void Camera::lookAt(const Vec3f &look_at, const Vec3f &ref_up) {
  forward = Normalize(look_at - position);
  right   = Normalize(Cross(forward, ref_up));
  up      = Cross(right, forward);

  AssertAllValid(forward, right, up);
  AssertAllNormalized(forward, right, up);

  // Indeed half of the length
  const Float y_len = tanf(Radians(fov / 2)) * focal_length;
  const Float x_len = y_len * film->getAspectRatio();

  // Scale the corresponding vectors
  forward = forward * focal_length;
  right   = right * x_len;
  up      = up * y_len;

  Mat3f generation_matrix(right, up, forward);
  inv_generation_matrix = Inverse(generation_matrix);

  Info_("Camera generation matrix:         {}", generation_matrix);
  Info_("Inverse Camera generation matrix: {}", inv_generation_matrix);

  // The area of the film(cancel out the focal length)
  normalized_area = 4 * x_len * y_len / (focal_length * focal_length);
  Info_("Camera Area: {}", normalized_area);

  // A simple test to avoid singularity
  const Vec2f dxy = {0.5, 0.5};
  const auto ray  = generateRay(dxy.x, dxy.y);
  AssertAllValid(ray.origin, ray.direction);
  AssertAllNormalized(ray.direction);

  Vec2f recovered_dxy;
  We(ray, &recovered_dxy);
  AssertNear(dxy, recovered_dxy, 1e-3);
}

Vec3f Camera::getPosition() const noexcept {
  return position;
}

Float Camera::getFov() const noexcept {
  return fov;
}

Float Camera::getNormalizedAreaOfImage() const noexcept {
  return normalized_area;
}

ref<Film> &Camera::getFilm() noexcept {
  return film;
}

RDR_NAMESPACE_END
