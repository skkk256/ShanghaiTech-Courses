/**
 * @file render.h
 * @author ShanghaiTech CS171 TAs
 * @brief It is necessary to wrap up the starting-up phase of the renderer (e.g.
 * to perform integration tests). Temporary implementation.
 * @version 0.1
 * @date 2023-06-17
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __RENDER_H__
#define __RENDER_H__

#include <utility>

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

/**
 * @brief Render class is the abstraction of the whole rendering pipeline, which
 * helps mitigate the problem of re-implementing the initialization process in
 * tests. Further, this prepares for the future integration of multiple render
 * WIP (as well as initialization process's design)
 */
class RenderInterface {
public:
  // For renderers, we don't allow for any kind of copying
  RenderInterface(RenderInterface &&)                      = delete;
  RenderInterface &operator=(RenderInterface &&)           = delete;
  RenderInterface(const RenderInterface &other)            = delete;
  RenderInterface &operator=(const RenderInterface &other) = delete;

  virtual ~RenderInterface() = default;

  /**
   * @brief Clear renderer runtime information and reset the renderer
   * state as if it is just initialized. This is useful when singleton objects
   * are used and hard to reset.
   */
  virtual void clearRuntimeInfo() {
    Factory::clearRuntimeInfo();

    // Must be executed in the last
    Memory::clearRuntimeInfo();
  }

  /**
   * @brief Initialize the renderer. This is supposed to be called only once
   * before clearRuntimeInfo() is called.
   */
  virtual void initialize() = 0;

  /**
   * @brief Preprocess the scene. All rendering processes might share this pass.
   */
  virtual void preprocess() = 0;

  /**
   * @brief Render the scene.
   */
  virtual void render() = 0;

  virtual vector<Vec3f> exportImageToArray() const           = 0;
  virtual bool exportImageToDisk(const fs::path &path) const = 0;

protected:
  RenderInterface(Properties props) : props(std::move(props)) {}

  const Properties props;
};

class NativeRender final : public RenderInterface {
public:
  NativeRender(const Properties &props) : RenderInterface(props) {}
  NativeRender(Properties &&props) : RenderInterface(std::move(props)) {}

  /// @see RenderInterface::clearRuntimeInfo
  void clearRuntimeInfo() override;

  /// @see RenderInterface::initialize
  void initialize() override;

  /// @see RenderInterface::preprocess
  void preprocess() override;

  /// @see RenderInterface::render
  void render() override;

  /// @see RenderInterface::exportImageToArray
  vector<Vec3f> exportImageToArray() const override;

  /// @see RenderInterface::exportImageToDisk
  bool exportImageToDisk(const fs::path &path) const override;

  /// @brief Generate a film using the existing pipeline when the renderer
  /// itself is properly initialized. To be refractored into member function.
  static Film prepareDebugCanvas(const Vec2i &resolution);

private:
  CrossConfigurationContext cross_context{};
  PreprocessContext preprocess_context{};
  vector<ConfigurableObject *> global_context{};
};

RDR_NAMESPACE_END

#endif
