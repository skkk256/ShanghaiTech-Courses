#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include "rdr/rdr.h"

RDR_NAMESPACE_BEGIN

class TexCoordinateGenerator : public ConfigurableObject {
public:
  // ++ Required by ConfigurableObject
  TexCoordinateGenerator(const Properties &props) : ConfigurableObject(props) {}
  // --

  virtual ~TexCoordinateGenerator() = default;
  virtual Vec2f Map(const SurfaceInteraction &interaction, Vec2f &dstdx,
      Vec2f &dstdy) const           = 0;
};

class UVMapping2D final : public TexCoordinateGenerator {
public:
  // ++ Required by ConfigurableObject
  UVMapping2D(const Properties &props)
      : scale(props.getProperty<Vec2f>("scale", Vec2f(1, 1))),
        delta(props.getProperty<Vec2f>("delta", Vec2f(0, 0))),
        TexCoordinateGenerator(props) {}
  // --
  Vec2f Map(const SurfaceInteraction &interaction, Vec2f &dstdx,
      Vec2f &dstdy) const override;

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "UVMapping2D [\n"
        "  scale  = {}\n"
        "  delta  = {}\n"
        "]",
        scale, delta);
  }
  // --

private:
  const Vec2f scale, delta;
};

class Texture : public ConfigurableObject {
public:
  // ++ Required by ConfigurableObject
  Texture(const Properties &props) : ConfigurableObject(props) {}
  // --

  virtual ~Texture() = default;

  /// Evaluate the texture at the given interaction
  virtual Vec3f evaluate(const SurfaceInteraction &interaction) const = 0;
};

class ConstantTexture final : public Texture {
public:
  // ++ Required by ConfigurableObject
  ConstantTexture(const Properties &props)
      : color(props.getProperty<Vec3f>("color")), Texture(props) {}
  // --

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "ConstantTexture [\n"
        "  color  = {}\n"
        "]",
        color);
  }
  // --

  /// @see Texture::evaluate
  Vec3f evaluate(const SurfaceInteraction &interaction) const override {
    return color;
  }

private:
  const Vec3f color;
};

class CheckerBoardTexture final : public Texture {
public:
  // ++ Required by ConfigurableObject
  CheckerBoardTexture(const Properties &props)
      : color0(props.getProperty<Vec3f>("color0", Vec3f(.4f))),
        color1(props.getProperty<Vec3f>("color1", Vec3f(.2f))),
        texmap(RDR_CREATE_CLASS(TexCoordinateGenerator,
            props.getProperty<Properties>("tex_coordinate_generator"))),
        Texture(props) {}
  // --

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "CheckerBoardTexture [\n"
        "  color0  = {}\n"
        "  color1  = {}\n"
        "]",
        color0, color1);
  }
  // --

  /// @see Texture::evaluate
  Vec3f evaluate(const SurfaceInteraction &interaction) const override {
    Vec2f dstdx, dstdy;
    const auto &st = texmap->Map(interaction, dstdx, dstdy);

    int x = 2 * Mod(static_cast<int>(st.x * 2), 2) - 1,
        y = 2 * Mod(static_cast<int>(st.y * 2), 2) - 1;

    if (x * y == 1)
      return color0;
    else
      return color1;
  }

private:
  const Vec3f color0;
  const Vec3f color1;
  ref<TexCoordinateGenerator> texmap;
};

/**
 * @brief Texture class. This class is designed only to load and evaluate
 * texture at a given uv position, which is different from our film
 * implementation. The format of the loaded texture is:
 * - 32-bit float
 * - 4 channels (RGBA)
 * - (0, 0) corresponds to the upper-left corner
 * - (1, 0) corresponds to the right of the upper-left corner, etc.
 * - data is stored in row-major order, i.e. elements in the same row are
 *  continuous in memory
 */
class ImageTexture final : public Texture {
public:
  // ++ Required by ConfigurableObject
  ImageTexture(const Properties &props);
  // --

  // ++ Required by Object
  std::string toString() const override {
    return format(
        "ImageTexture [\n"
        "  width  = {},\n"
        "  height = {},\n"
        "  path   = {}\n"
        "]",
        width, height, properties.getProperty<std::string>("path"));
  }
  // --

  virtual ~ImageTexture() = default;
  const Float *getData() const { return data.data(); }

  int getWidth() const { return width; }
  int getHeight() const { return height; }

  // ++ Required by Texture
  Vec3f evaluate(const SurfaceInteraction &interaction) const override;
  // --

protected:
  int width, height;

  vector<Float> data;
  ref<MIPMap> mipmap;
  ref<TexCoordinateGenerator> texmap;
};

RDR_REGISTER_CLASS(UVMapping2D)

RDR_REGISTER_FACTORY(TexCoordinateGenerator,
    [](const Properties &props) -> TexCoordinateGenerator * {
      auto type = props.getProperty<std::string>("type");
      if (type == "uvmapping2d") {
        return Memory::alloc<UVMapping2D>(props);
      } else {
        Exception_("TexCoordinateGenerator type {} not supported", type);
      }

      return nullptr;
    })

RDR_REGISTER_CLASS(ImageTexture)
RDR_REGISTER_CLASS(ConstantTexture)
RDR_REGISTER_CLASS(CheckerBoardTexture)

RDR_REGISTER_FACTORY(Texture, [](const Properties &props) -> Texture * {
  auto type = props.getProperty<std::string>("type");
  if (type == "image") {
    return Memory::alloc<ImageTexture>(props);
  } else if (type == "checkerboard") {
    return Memory::alloc<CheckerBoardTexture>(props);
  } else if (type == "constant") {
    return Memory::alloc<ConstantTexture>(props);
  } else {
    Exception_("Texture type {} not supported", type);
  }

  return nullptr;
})

RDR_NAMESPACE_END

#endif
