/**
 * @file factory_tests.cpp
 * @author CS171 TA Group
 * @brief
 * @version 0.1
 * @date 2023-05-01
 *
 * @copyright Copyright (c) 2023
 */

#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

#include "rdr/properties.h"

using namespace RDR_NAMESPACE_NAME;

static auto sample_config = R"(
{
  "integrator": {
    "type": "path",
    "spp": 32,
    "max_depth": 12
  },
  "film": {
    "resolution": [
      600,
      600
    ]
  },
  "camera": {
    "position": [
      0.0,
      1.0,
      6.8
    ],
    "look_at": [
      0.0,
      1.0,
      0.0
    ],
    "ref_up": [
      0.0,
      1.0,
      0.0
    ],
    "fov": 19.5,
    "focal_length": 1.0
  },
  "materials": {
    "grey_diffuse": {
      "type": "diffuse",
      "color": [
        0.725,
        0.71,
        0.68
      ]
    },
    "green_diffuse": {
      "type": "diffuse",
      "color": [
        0.14,
        0.45,
        0.091
      ]
    },
    "red_diffuse": {
      "type": "diffuse",
      "color": [
        0.63,
        0.065,
        0.05
      ]
    }
  },
  "objects": [
    {
      "type": "mesh",
      "path": "assets/light.obj",
      "light": {
        "type": "area",
        "radiance": [
          17.0,
          12.0,
          5.0
        ]
      }
    },
    {
      "type": "mesh",
      "path": "assets/left.obj",
      "material_name": "red_diffuse"
    },
    {
      "type": "mesh",
      "path": "assets/right.obj",
      "material_name": "green_diffuse"
    },
    {
      "type": "mesh",
      "path": "assets/floor.obj",
      "material_name": "grey_diffuse"
    },
    {
      "type": "mesh",
      "path": "assets/ceiling.obj",
      "material_name": "grey_diffuse"
    },
    {
      "type": "mesh",
      "path": "assets/back.obj",
      "material_name": "grey_diffuse"
    },
    {
      "type": "mesh",
      "path": "assets/short_box.obj",
      "material_name": "grey_diffuse",
      "translate": [
        -0.7,
        0,
        0.6
      ]
    },
    {
      "type": "mesh",
      "path": "assets/tall_box.obj",
      "material_name": "grey_diffuse",
      "translate": [
        0.7,
        0,
        -0.5
      ]
    }
  ]
}
)";

TEST(Properties, ConstructFromJson) {
  using json      = Properties::json;
  json       data = json::parse(sample_config);
  Properties prop(data);

  EXPECT_EQ(prop.getProperty<Properties>("integrator")
                .getProperty<std::string>("type"),
            "path");
  EXPECT_EQ(prop.getProperty<Properties>("integrator").getProperty<int>("spp"),
            32);
  EXPECT_EQ(
      prop.getProperty<Properties>("integrator").getProperty<int>("max_depth"),
      12);

  EXPECT_EQ(
      prop.getProperty<Properties>("film").getProperty<Vec2i>("resolution"),
      Vec2i(600, 600));

  EXPECT_EQ(
      prop.getProperty<Properties>("camera").getProperty<Vec3f>("position"),
      Vec3f(0, 1, 6.8));
  EXPECT_EQ(
      prop.getProperty<Properties>("camera").getProperty<Vec3f>("look_at"),
      Vec3f(0, 1, 0));
  EXPECT_EQ(prop.getProperty<Properties>("camera").getProperty<Vec3f>("ref_up"),
            Vec3f(0, 1, 0));
  EXPECT_EQ(prop.getProperty<Properties>("camera").getProperty<Float>("fov"),
            Float(19.5));
  EXPECT_EQ(
      prop.getProperty<Properties>("camera").getProperty<Float>("focal_length"),
      Float(1));

  auto materials = prop.getProperty<Properties>("materials");
  EXPECT_EQ(materials.getProperty<Properties>("grey_diffuse")
                .getProperty<std::string>("type"),
            "diffuse");
  EXPECT_EQ(materials.getProperty<Properties>("grey_diffuse")
                .getProperty<Vec3f>("color"),
            Vec3f(0.725, 0.71, 0.68));
  EXPECT_EQ(materials.getProperty<Properties>("green_diffuse")
                .getProperty<std::string>("type"),
            "diffuse");
  EXPECT_EQ(materials.getProperty<Properties>("green_diffuse")
                .getProperty<Vec3f>("color"),
            Vec3f(0.14, 0.45, 0.091));
  EXPECT_EQ(materials.getProperty<Properties>("red_diffuse")
                .getProperty<std::string>("type"),
            "diffuse");
  EXPECT_EQ(materials.getProperty<Properties>("red_diffuse")
                .getProperty<Vec3f>("color"),
            Vec3f(0.63, 0.065, 0.05));

  auto objects = prop.getProperty<vector<Properties>>("objects");
  EXPECT_EQ(objects[0].getProperty<std::string>("type"), "mesh");
  EXPECT_EQ(objects[0].getProperty<std::string>("path"), "assets/light.obj");
  EXPECT_EQ(
      objects[0].getProperty<Properties>("light").getProperty<std::string>(
          "type"),
      "area");
  EXPECT_EQ(objects[0].getProperty<Properties>("light").getProperty<Vec3f>(
                "radiance"),
            Vec3f(17.0, 12.0, 5.0));

  EXPECT_EQ(objects[1].getProperty<std::string>("type"), "mesh");
  EXPECT_EQ(objects[1].getProperty<std::string>("path"), "assets/left.obj");
  EXPECT_EQ(objects[1].getProperty<std::string>("material_name"),
            "red_diffuse");

  EXPECT_EQ(objects[2].getProperty<std::string>("type"), "mesh");
  EXPECT_EQ(objects[2].getProperty<std::string>("path"), "assets/right.obj");
  EXPECT_EQ(objects[2].getProperty<std::string>("material_name"),
            "green_diffuse");
}
