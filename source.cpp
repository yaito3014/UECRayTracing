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
#define YK_IMAGE_WIDTH 400
#endif  // !YK_IMAGE_WIDTH

#ifndef YK_SPP
#define YK_SPP 100
#endif  // !YK_SPP

#ifndef YK_MAX_DEPTH
#define YK_MAX_DEPTH 50
#endif  // !YK_MAX_DEPTH

namespace yk {

namespace constants {

constexpr double aspect_ratio = 16.0 / 9.0;
constexpr std::uint32_t image_width = YK_IMAGE_WIDTH;
constexpr std::uint32_t image_height =
    static_cast<std::uint32_t>(image_width / aspect_ratio);
constexpr std::uint32_t samples_per_pixel = YK_SPP;
constexpr std::uint32_t max_depth = YK_MAX_DEPTH;

}  // namespace constants

using color = color3d;

using image_t =
    std::array<color3b, constants::image_width * constants::image_height>;

std::uint32_t verbose = 0;

template <concepts::arithmetic T>
constexpr color3b to_color3b(const color3<T>& from,
                             std::uint32_t samples_per_pixel) {
  auto [r, g, b] = from / samples_per_pixel;
  color3<T> color = {
      .r = math::sqrt(r),
      .g = math::sqrt(g),
      .b = math::sqrt(b),
  };
  return (color.clamped(0.0, 0.999) * 256).template to<uint8_t>();
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_in_unit_sphere(Gen& gen) {
  return vec3<T>::random(gen, -1, 1).normalize() *
         uniform_real_distribution<T>(0.01, 0.99)(gen);
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
constexpr vec3<T> random_unit_vector(Gen& gen) {
  return vec3<T>::random(gen, -1, 1).normalize();
}

template <concepts::arithmetic T, std::uniform_random_bit_generator Gen>
vec3<T> random_in_hemisphere(const vec3<T>& normal, Gen& gen) {
  vec3<T> in_unit_sphere = random_unit_vector<T>(gen);
  return dot(in_unit_sphere, normal) > 0.0 ? in_unit_sphere : -in_unit_sphere;
}

template <concepts::arithmetic T, concepts::hittable<T> H,
          std::uniform_random_bit_generator Gen>
constexpr color ray_color(const ray<T>& r, const H& world, unsigned int depth,
                          Gen& gen) {
  if (!std::is_constant_evaluated() && verbose > 2)
    std::cout << "ray { origin : (" << r.origin.x << ", " << r.origin.y << ", "
              << r.origin.z << "), direction : (" << r.direction.x << ", "
              << r.direction.y << ", " << r.direction.z << ") }" << '\n';
  if (depth == 0) return color(0, 0, 0);
  hit_record<T> rec;
  if (world.hit(r, 0.001, std::numeric_limits<T>::infinity(), rec)) {
    pos3<T> target = rec.p + random_in_hemisphere<T>(rec.normal, gen);
    return ray_color(ray<T>(rec.p, target - rec.p), world, depth - 1, gen) / 2;
  }
  auto t = (r.direction.normalized().y + 1.0) / 2;
  return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
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


template <concepts::arithmetic T = double>
constexpr image_t render() {
  const camera<T> cam;

  const auto world = hittable_list<T>{}
                         .add(sphere<T>(pos3<T>(0, 0, -1), 0.5))
                         .add(sphere<T>(pos3<T>(0, -100.5, -1), 100));

  if (!std::is_constant_evaluated()) std::cout << "rendering..." << std::endl;

  image_t image = {};

  constexpr std::string_view time = __TIME__;
  constexpr std::uint32_t constexpr_seed =
      std::accumulate(time.begin(), time.end(), std::uint32_t(0));

  for_each(
      views::cartesian_product(std::views::iota(0u, constants::image_height),
                               std::views::iota(0u, constants::image_width)),
      [&](auto hw) {
        const auto& [h, w] = hw;

        if (!std::is_constant_evaluated() && verbose)
          std::cout << "(row,col) : " << '('
                    << std::setw(
                           std::ceil(std::log10(constants::image_height)) - 1)
                    << h << ','
                    << std::setw(std::ceil(std::log10(constants::image_width)) -
                                 1)
                    << w << ')' << std::endl;

        color pixel_color = transform_reduce(
            std::views::iota(0u, constants::samples_per_pixel), color(0, 0, 0),
            std::plus{}, [&](auto s) {
              if (!std::is_constant_evaluated() && verbose > 1)
                std::cout
                    << "(row,col,sam) : " << '('
                    << std::setw(
                           std::ceil(std::log10(constants::image_height)) - 1)
                    << h << ','
                    << std::setw(std::ceil(std::log10(constants::image_width)) -
                                 1)
                    << w << ','
                    << std::setw(
                           std::ceil(std::log10(constants::samples_per_pixel)) -
                           1)
                    << s << ')' << std::endl;

              xor128 gen(std::is_constant_evaluated()
                             ? constexpr_seed +
                                   (h * constants::image_width + w) *
                                       constants::samples_per_pixel +
                                   s
                             : std::random_device{}());
              uniform_real_distribution<T> dist(0, 1);

              auto u = (w + dist(gen)) / (constants::image_width - 1.0);
              auto v = (constants::image_height - (h + dist(gen)) - 1) /
                       (constants::image_height - 1.0);
              return ray_color(cam.get_ray(u, v), world, constants::max_depth,
                               gen);
            });

        // parallel access to different element in the same vector is safe
        image[h * constants::image_width + w] =
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
