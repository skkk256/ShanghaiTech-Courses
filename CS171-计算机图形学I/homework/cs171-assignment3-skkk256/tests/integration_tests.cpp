/**
 * @file integration_tests.cpp
 * @author CS171 TA Group
 * @brief Not only the "integration" test, but also the integration test
 * @version 0.1
 * @date 2023-06-16
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <gtest/gtest.h>

#include "config_template.h"
#include "nlohmann/json.hpp"
#include "rdr/rdr.h"
#include "rdr/render.h"

using namespace RDR_NAMESPACE_NAME;

static std::string replaceAll(
    const std::string &str_in, const std::string &from, const std::string &to) {
  // https://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
  std::string str = str_in;
  if (from.empty()) return str;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }

  return str;
}

static void renderPixelAverageWithCallback(
    const std::string &config, std::function<bool(Vec3f)> callback) {
  spdlog::set_level(spdlog::level::err);
  Factory::doRegisterAllClasses();

  const std::array<std::string, 3> profile_names = {
      "RandomWalk", "NextEventEstimation", "MultipleImportanceSampling"};
  for (const auto &profile : profile_names) {
    auto sphere_config = replaceAll(config, "{{0}}", profile);

    nlohmann::json root_json = nlohmann::json::parse(sphere_config);
    Properties root_properties(root_json);
    ref<RenderInterface> render = make_ref<NativeRender>(root_properties);

    // Render pipeline
    render->initialize();
    render->preprocess();
    render->render();

    vector<Vec3f> result = render->exportImageToArray();
    Vec3f pixel_average =
        std::reduce(result.begin(), result.end(), Vec3f(0, 0, 0),
            [](const Vec3f &a, const Vec3f &b) { return a + b; }) /
        result.size();
    bool bSucceed = callback(pixel_average);
    if (!bSucceed) {
      Exception_("Failed at {}", profile);
    }

    // cleanup the singletons
    render->clearRuntimeInfo();
  }
}

TEST(IntegrationTests, WhiteSphere) {
  // clang-format off
  renderPixelAverageWithCallback(sphere_config_template, 
  [](Vec3f pixel) -> bool {
    EXPECT_NEAR(pixel.x, 1, 1e-3);
    EXPECT_NEAR(pixel.y, 1, 1e-3);
    EXPECT_NEAR(pixel.z, 1, 1e-3);
    return !HasFailure();
  });
  // clang-format on
}

TEST(IntegrationTests, GlassSphere) {
  // clang-format off
  renderPixelAverageWithCallback(sphere_config_template, 
  [](Vec3f pixel) -> bool {
    EXPECT_NEAR(pixel.x, 1, 1e-3);
    EXPECT_NEAR(pixel.y, 1, 1e-3);
    EXPECT_NEAR(pixel.z, 1, 1e-3);
    return !HasFailure();
  });
  // clang-format on
}

TEST(IntegrationTests, TwoSpheres) {
  // clang-format off
  renderPixelAverageWithCallback(two_spheres_config_template, 
  [](Vec3f pixel) -> bool {
    EXPECT_NEAR(pixel.x, 0.27657896, 2e-3);
    EXPECT_NEAR(pixel.y, 0.27657896, 2e-3);
    EXPECT_NEAR(pixel.z, 0.27657896, 2e-3);
    return !HasFailure();
  });
  // clang-format on
}
