#include <array>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numbers>
#include <numeric>
#include <random>
#include <ranges>
#include <string>
#include <tuple>

#if YK_ENABLE_PARALLEL
#include <execution>
#endif  // YK_ENABLE_PARALLEL

#include "thirdparty/cxxopts.hpp"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"
#include "yk/camera.hpp"
#include "yk/cartesian_product.hpp"
#include "yk/color.hpp"
#include "yk/config.hpp"
#include "yk/hittable.hpp"
#include "yk/hittable_list.hpp"
#include "yk/material.hpp"
#include "yk/math.hpp"
#include "yk/random.hpp"
#include "yk/ray.hpp"
#include "yk/sphere.hpp"
#include "yk/vec3.hpp"

#if YK_ENABLE_PARALLEL
#define YK_EXEC_PAR std::execution::par,
#else
#define YK_EXEC_PAR
#endif  // YK_ENABLE_PARALLEL

#ifndef YK_IMAGE_WIDTH
#define YK_IMAGE_WIDTH 1200
#endif  // !YK_IMAGE_WIDTH

#ifndef YK_SPP
#define YK_SPP 500
#endif  // !YK_SPP

#ifndef YK_MAX_DEPTH
#define YK_MAX_DEPTH 50
#endif  // !YK_MAX_DEPTH

namespace yk {

namespace constants {

constexpr double aspect_ratio = 3.0 / 2.0;
constexpr std::uint32_t image_width = YK_IMAGE_WIDTH;
constexpr std::uint32_t image_height =
    static_cast<std::uint32_t>(image_width / aspect_ratio);
constexpr std::uint32_t samples_per_pixel = YK_SPP;
constexpr std::uint32_t max_depth = YK_MAX_DEPTH;

}  // namespace constants

using color = color3d;

using image_t =
    std::array<color3b, constants::image_width * constants::image_height>;

template <concepts::arithmetic T>
constexpr color3b to_color3b(const color3<T>& from,
                             std::uint32_t samples_per_pixel) noexcept {
  auto [r, g, b] = from / samples_per_pixel;
  color3<T> color = {
      .r = math::sqrt(r),
      .g = math::sqrt(g),
      .b = math::sqrt(b),
  };
  return (color.clamped(0.0, 0.999) * 256).template to<uint8_t>();
}

template <std::ranges::input_range R, std::copy_constructible F>
constexpr auto for_each(R&& range, F func) {
  return std::for_each(YK_EXEC_PAR std::ranges::begin(range),
                       std::ranges::end(range), func);
}

template <std::ranges::input_range R, class T, std::copy_constructible BinOp,
          std::invocable<std::ranges::range_value_t<R>> UnaryOp>
constexpr T transform_reduce(R&& r, T init, BinOp bin_op, UnaryOp unary_op) {
  return std::transform_reduce(YK_EXEC_PAR std::ranges::begin(r),
                               std::ranges::end(r), init, bin_op, unary_op);
}

template <concepts::arithmetic T>
constexpr auto random_scene() noexcept {
  auto ground_material = lambertian(color(0.5, 0.5, 0.5));
  auto material1 = dielectric<color::value_type>(1.5);
  auto material2 = lambertian(color(0.4, 0.2, 0.1));
  auto material3 = metal(color(0.7, 0.6, 0.5), 0.0);

  return hittable_list<T>{} |
         sphere(pos3<T, world_tag>(0, -1000, 0), 1000., ground_material) |
         sphere(pos3<T, world_tag>(0, 1, 0), 1.0, material1) |
         sphere(pos3<T, world_tag>(-4, 1, 0), 1.0, material2) |
         sphere(pos3<T, world_tag>(4, 1, 0), 1.0, material3);
}

template <concepts::arithmetic T = double>
constexpr image_t render() {
  const auto world = random_scene<T>();

  pos3<T, world_tag> lookfrom(13, 2, 3);
  pos3<T, world_tag> lookat(0, 0, 0);
  vec3<T> vup(0, 1, 0);
  auto dist_to_focus = 10.0;
  auto aperture = 0.1;

  const camera<T> cam(lookfrom, lookat, vup, 20, constants::aspect_ratio,
                      aperture, dist_to_focus);
  const raytracer<T, double> tracer = {};

  if (!std::is_constant_evaluated()) std::cout << "rendering..." << std::endl;

  image_t image = {};

  for_each(
      views::cartesian_product(std::views::iota(0u, constants::image_height),
                               std::views::iota(0u, constants::image_width)),
      [&](auto yx) {
        const auto& [y, x] = yx;

        if (!std::is_constant_evaluated() && verbose)
          std::cout << "(row,col) : " << '('
                    << std::setw(
                           std::ceil(std::log10(constants::image_height)) - 1)
                    << y << ','
                    << std::setw(std::ceil(std::log10(constants::image_width)) -
                                 1)
                    << x << ')' << std::endl;

        color pixel_color = transform_reduce(
            std::views::iota(0u, constants::samples_per_pixel), color(0, 0, 0),
            std::plus{}, [&](auto s) {
              if (!std::is_constant_evaluated() && verbose > 1)
                std::cout
                    << "(row,col,sam) : " << '('
                    << std::setw(
                           std::ceil(std::log10(constants::image_height)) - 1)
                    << y << ','
                    << std::setw(std::ceil(std::log10(constants::image_width)) -
                                 1)
                    << x << ','
                    << std::setw(
                           std::ceil(std::log10(constants::samples_per_pixel)) -
                           1)
                    << s << ')' << std::endl;

              constexpr std::string_view time = __TIME__;
              constexpr std::uint32_t constexpr_seed =
                  std::accumulate(time.begin(), time.end(), std::uint32_t(0));

              mt19937 gen(std::is_constant_evaluated()
                              ? constexpr_seed +
                                    (y * constants::image_width + x) *
                                        constants::samples_per_pixel +
                                    s
                              : std::random_device{}());
              uniform_real_distribution<T> dist(0, 1);

              auto u = (x + dist(gen)) / constants::image_width;
              auto v = (constants::image_height - y - 1 + dist(gen)) /
                       constants::image_height;
              return tracer.ray_color(cam.get_ray(u, v, gen), world,
                                      constants::max_depth, gen);
            });

        // parallel access to different element in the same vector is safe
        image[y * constants::image_width + x] =
            to_color3b(pixel_color, constants::samples_per_pixel);
      });

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
  std::ios::sync_with_stdio(false);
  std::cout.tie(nullptr);

  std::string filename;

  // handle command line args
  cxxopts::Options options("raytrace", "raytracing program");

  // clang-format off
  options.add_options()
      ("h,help"         , "print usage")
      ("v,verbose"      , "verbose output")
      ("o,output"       , "filename of output", cxxopts::value<std::string>())
      ("l,verbose-level", "set verbose level" , cxxopts::value<std::vector<std::uint32_t>>());
  // clang-format on

  options.parse_positional({"output", "positional"});
  auto parsed = options.parse(argc, argv);
  if (parsed.count("help") || !parsed.count("output")) {
    std::cout << options.help() << std::endl;
    exit(EXIT_SUCCESS);
  }
  if (parsed.count("verbose") && !yk::verbose) ++yk::verbose;
  if (parsed.count("verbose-level"))
    yk::verbose =
        parsed["verbose-level"].as<std::vector<std::uint32_t>>().back();

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
