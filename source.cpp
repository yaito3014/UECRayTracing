#include <array>
#include <boost/program_options.hpp>
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

}  // namespace constants

using image_t =
    std::array<color3b, constants::image_width * constants::image_height>;

template <std::floating_point T>
constexpr color3b to_color3b(const color3<T>& from, size_t samples_per_pixel) {
  return ((from / samples_per_pixel).clamped(0.0, 0.999) * 256)
      .template to<uint8_t>();
}

template <std::floating_point T, concepts::hittable<T> H>
constexpr color3d ray_color(const ray<T>& r, const H& world) {
  hit_record<T> rec;
  if (world.hit(r, 0, std::numeric_limits<T>::infinity(), rec))
    return (color3d{.r = rec.normal.x, .g = rec.normal.y, .b = rec.normal.z} +
            color3d(1, 1, 1)) /
           2;
  auto t = (r.direction.normalized().y + 1.0) / 2;
  return (1.0 - t) * color3d(1.0, 1.0, 1.0) + t * color3d(0.5, 0.7, 1.0);
}

template <class R, class F>
YK_CONSTEXPR auto for_each(R r, F&& f) {
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
      std::is_constant_evaluated()
          ? std::accumulate(time.begin(), time.end(), size_t(0))
          : std::random_device{}();

  xor128 gen(seed);
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
#if YK_ENABLE_PARALLEL
                 mtx.lock();
#endif  // YK_ENABLE_PARALLEL
                 auto u = (w + dist(gen)) / (constants::image_width - 1.0);
                 auto v = (constants::image_height - h - 1 + dist(gen)) /
                          (constants::image_height - 1.0);
#if YK_ENABLE_PARALLEL
                 mtx.unlock();
#endif  // YK_ENABLE_PARALLEL
                 yk::color3d color = ray_color(cam.get_ray(u, v), world);
#if YK_ENABLE_PARALLEL
                 std::lock_guard<std::mutex> lg(mtx);
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
  namespace po = boost::program_options;

  po::positional_options_description p;
  p.add("output-file", -1);

  po::options_description desc("Available options");
  desc.add_options()("help,h", " : help message")(
      "output-file,o", po::value<std::string>(),
      " : specify output filename")("verbose,v", " : verbose");

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.contains("help")) {
    desc.print(std::cout);
    return 0;
  }

  yk::verbose = vm.count("verbose");

  std::string filename;
  if (vm.contains("output-file"))
    filename = vm["output-file"].as<std::string>();
  else {
    std::cout << "filename required" << std::endl;
    desc.print(std::cout);
    std::exit(EXIT_FAILURE);
  }

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
