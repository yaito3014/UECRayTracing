#include <array>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <ranges>
#include <string>
#include <tuple>

#if YK_ENABLE_PARALLEL
#include <execution>
#include <mutex>
#endif  // YK_ENABLE_PARALLEL

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/cxxopts.hpp"
#include "thirdparty/stb_image_write.h"
#include "yk/camera.hpp"
#include "yk/color.hpp"
#include "yk/config.hpp"
#include "yk/hittable.hpp"
#include "yk/hittable_list.hpp"
#include "yk/math.hpp"
#include "yk/random.hpp"
#include "yk/ray.hpp"
#include "yk/sphere.hpp"
#include "yk/vec3.hpp"

#ifndef YK_IMAGE_WIDTH
#define YK_IMAGE_WIDTH 400
#endif  // !YK_IMAGE_WIDTH

#ifndef YK_SPP
#define YK_SPP 100
#endif  // !YK_SPP

namespace yk {

namespace constants {

constexpr double aspect_ratio = 16.0 / 9.0;
constexpr size_t image_width = YK_IMAGE_WIDTH;
constexpr size_t image_height = static_cast<size_t>(image_width / aspect_ratio);
constexpr size_t samples_per_pixel = YK_SPP;
constexpr unsigned int max_depth = 50;

}  // namespace constants

using image_t =
    std::array<color3b, constants::image_width * constants::image_height>;

template <std::floating_point T>
constexpr color3b to_color3b(const color3<T>& from, size_t samples_per_pixel) {
  auto [r, g, b] = from / samples_per_pixel;
  color3d color = {
      .r = sqrt(r),
      .g = sqrt(g),
      .b = sqrt(b),
  };
  return (color.clamped(0.0, 0.999) * 256).template to<uint8_t>();
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_vec_in_unit_sphere(Gen& gen) {
  return vec3<double>::random(gen, -1, 1).normalize() *
         uniform_real_distribution<T>(0.1, 0.99)(gen);
}

template <std::floating_point T, concepts::hittable<T> H,
          std::uniform_random_bit_generator Gen>
constexpr color3d ray_color(const ray<T>& r, const H& world, unsigned int depth,
                            Gen& gen) {
  if (depth == 0) return color3d(0, 0, 0);
  hit_record<T> rec;
  if (world.hit(r, 0, std::numeric_limits<T>::infinity(), rec)) {
    pos3<T> target = rec.p + rec.normal + random_vec_in_unit_sphere<T>(gen);
    return ray_color(ray<T>(rec.p, target - rec.p), world, depth - 1, gen) / 2;
  }
  auto t = (r.direction.normalized().y + 1.0) / 2;
  return (1.0 - t) * color3d(1.0, 1.0, 1.0) + t * color3d(0.5, 0.7, 1.0);
}

template <class R, class F>
constexpr auto for_each(R r, F&& f) {
  auto common = r | std::views::common;
  return std::for_each(
#if YK_ENABLE_PARALLEL
      std::execution::par,
#endif  // YK_ENABLE_PARALLEL
      std::ranges::begin(common), std::ranges::end(common), std::forward<F>(f));
}

size_t verbose = 0;

template <concepts::arithmetic T = double>
constexpr image_t render() {
  camera<T> cam;

  auto world = hittable_list<T>{}
                   .add(sphere<T>(pos3<T>(0, 0, -1), 0.5))
                   .add(sphere<T>(pos3<T>(0, -100.5, -1), 100));

  if (!std::is_constant_evaluated()) std::cout << "rendering..." << std::endl;

  image_t image = {};

  constexpr std::string_view time = __TIME__;
  constexpr uint32_t seed =
      std::accumulate(time.begin(), time.end(), size_t(0));

#if YK_ENABLE_PARALLEL
  thread_safe_random_generator<xor128>
#else
  xor128
#endif  // YK_ENABLE_PARALLEL
      gen(std::is_constant_evaluated() ? seed : std::random_device{}());
  uniform_real_distribution<T> dist(0.0, 1.0);

  for (size_t h : std::views::iota(0u, constants::image_height)) {
    if (!std::is_constant_evaluated() && verbose)
      std::cout << "rendering row : " << (h + 1) << " / "
                << constants::image_height << std::endl;
    for_each(std::views::iota(0u, constants::image_width), [&, h](size_t w) {
      color3<T> pixel_color(0, 0, 0);

#if YK_ENABLE_PARALLEL
      std::mutex mtx;
#endif  // YK_ENABLE_PARALLEL

      for_each(std::views::iota(0u, constants::samples_per_pixel),
               [&, h, w](size_t s) {
                 if (!std::is_constant_evaluated() && verbose > 1)
                   std::cout << "[pixel (" << h << ", " << w << ")] "
                             << "sample : " << s << " / "
                             << constants::samples_per_pixel << std::endl;
                 auto u = (w + dist(gen)) / (constants::image_width - 1.0);
                 auto v = (constants::image_height - h - 1 + dist(gen)) /
                          (constants::image_height - 1.0);
                 yk::color3d color = ray_color(cam.get_ray(u, v), world,
                                               constants::max_depth, gen);
#if YK_ENABLE_PARALLEL
                 std::lock_guard<std::mutex>{mtx},
#endif  // YK_ENABLE_PARALLEL
                     pixel_color += color;
               });
      // parallel access to different element in the same vector is safe
      image[h * yk::constants::image_width + w] =
          to_color3b(pixel_color, constants::samples_per_pixel);
    });
  }

  if (!std::is_constant_evaluated())
    std::cout << "rendering finished" << std::endl;

  return image;
}

void print_ppm(const image_t& image) {
  std::cout << "P3" << '\n'
            << constants::image_width << ' ' << constants::image_height << '\n'
            << 255 << '\n';
  for (const auto& [r, g, b] : image)
    std::cout << +r << ' ' << +g << ' ' << +b << '\n';
}

}  // namespace yk

int main(int argc, char* argv[]) {
  // handle command line args

  std::string filename;
  cxxopts::Options options("raytrace", "raytracing program");
  options.add_options()("h,help", "print usage")(
      "o,output", "filename of output",
      cxxopts::value<
          std::string>())("v,verbose",
                          "verbose output")("verbose-level",
                                            "set verbose level",
                                            cxxopts::value<
                                                std::vector<size_t>>());
  options.parse_positional({"output", "positional"});
  auto parsed = options.parse(argc, argv);
  if (parsed.count("help") || !parsed.count("output")) {
    std::cout << options.help() << std::endl;
    exit(EXIT_SUCCESS);
  }
  if (parsed.count("verbose") && !yk::verbose) ++yk::verbose;
  if (parsed.count("verbose-level"))
    yk::verbose = parsed["verbose-level"].as<std::vector<size_t>>().back();

  filename = parsed["output"].as<std::string>();

  // rendering
  YK_CONSTEXPR yk::image_t image = yk::render();

  std::cout << "write to file : " << filename << std::endl;
  if (!stbi_write_png(filename.c_str(), yk::constants::image_width,
                      yk::constants::image_height, 3, image.data(),
                      3 * yk::constants::image_width)) {
    std::cout << "error" << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::cout << "success" << std::endl;
}
