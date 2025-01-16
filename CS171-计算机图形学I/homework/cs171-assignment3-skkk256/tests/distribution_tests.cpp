
#include <gtest/gtest.h>

#include "rdr/math_utils.h"

using namespace RDR_NAMESPACE_NAME;

TEST(Distribution, Distribution1D) {
  constexpr int N = 1000000;
  constexpr int M = 100;
  Sampler       sampler;
  Float         sum = 0;

  std::array<float, M> arr;
  std::array<int, M>   pool;
  for (int i = 0; i < M; ++i) {
    arr[i]  = sampler.get1D();
    pool[i] = 0;
    sum += arr[i];
  }

  Distribution1D dist(arr.data(), M);
  for (int sample_id = 0; sample_id < N; sample_id++) {
    Float        pdf;
    const float &u = sampler.get1D();
    const int   &i = dist.sampleDiscrete(u, &pdf);
    EXPECT_NEAR(pdf, arr[i] / sum, 1e-3);
    EXPECT_TRUE(0 <= i && i < M);
    pool[i]++;
  }

  for (int i = 0; i < M; ++i)
    EXPECT_NEAR(pool[i] / (float)N, arr[i] / sum, 1e-3);
}