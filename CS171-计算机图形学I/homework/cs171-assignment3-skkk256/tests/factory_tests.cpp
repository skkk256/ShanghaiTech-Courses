/**
 * @file factory_tests.cpp
 * @author CS171 TA Group
 * @brief
 * @version 0.1
 * @date 2023-05-01
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <gtest/gtest.h>

#include <memory>

#include "rdr/factory.h"
#include "rdr/object.h"
#include "rdr/properties.h"
#include "rdr/rdr.h"

using namespace RDR_NAMESPACE_NAME;

class Sphere : public ConfigurableObject {
public:
  Sphere() noexcept = default;
  Sphere(const Properties &props) noexcept : ConfigurableObject(props) {
    ConstructFromProperties(props);
  }
  ~Sphere() noexcept = default;
  void ConstructFromProperties(const Properties &props) noexcept {
    origin = props.getProperty<Vec3f>("origin");
    radius = props.getProperty<Float>("radius");
  }
  void print() const noexcept {
    std::cout << "origin: (" << origin.x << ", " << origin.y << ", " << origin.z
              << ")\n";
    std::cout << "radius: " << radius << "\n";
  }

private:
  Vec3f origin;
  Float radius;
};

TEST(Factory, RegisterAndCreateTest) {
  // RDRRegisterWithString<Sphere>()
  RDR_REGISTER_CLASS(Sphere);
  Factory::doRegisterAllClasses();

  Properties props;

  props.setProperty("origin", Vec3f(0, 0, 0));
  props.setProperty("radius", 1.5_F);

  // auto sphere = RDR_CREATE_CLASS_FROM_PROPERTIES(Sphere, props);
  ref<Sphere> sphere = RDR_CREATE_CLASS(Sphere, props);

  sphere->print();
}

class ClassA : public ConfigurableObject {
public:
  ClassA() noexcept = delete;
  ClassA(const Properties &props) {
    a = props.getProperty<int>("a");
    b = props.getProperty<int>("b");
    c = props.getProperty<int>("c");
  }
  ~ClassA() noexcept = default;
  int a, b, c;
};

// You cannot Register a non-derived-from-cobject class (cannot be compiled)
TEST(Factory, RegisterAndCreateANonDerivedCobjectTest) {
  RDR_REGISTER_CLASS(ClassA);
  Factory::doRegisterAllClasses();
  Properties props;

  props.setProperty("a", 1);
  props.setProperty("b", 2);
  props.setProperty("c", 3);

  ref<ClassA> A = RDR_CREATE_CLASS(ClassA, props);

  EXPECT_EQ(A->a, 1);
  EXPECT_EQ(A->b, 2);
  EXPECT_EQ(A->c, 3);
}