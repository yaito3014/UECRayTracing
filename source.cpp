#include <array>
#include <boost/program_options.hpp>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <ranges>
#include <string>
#include <tuple>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "thirdparty/stb_image_write.h"
#include "yk/color.hpp"
#include "yk/config.hpp"
#include "yk/hittable.hpp"
#include "yk/hittable_list.hpp"
#include "yk/math.hpp"
#include "yk/ray.hpp"
#include "yk/sphere.hpp"
#include "yk/vec3.hpp"

namespace yk {

namespace constants {

constexpr double aspect_ratio = 16.0 / 9.0;
constexpr size_t image_width = 400;
constexpr size_t image_height = static_cast<size_t>(image_width / aspect_ratio);

}  // namespace constants

using image_t =
    std::array<color3b, constants::image_width * constants::image_height>;

template <std::floating_point T>
constexpr color3b to_color3b(const color3<T>& from) {
  return (from * 255.999).template to<uint8_t>();
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

constexpr auto viewport_height = 2.0;
constexpr auto viewport_width = viewport_height * constants::aspect_ratio;
constexpr auto focal_length = 1.0;

constexpr auto origin = pos3d(0, 0, 0);
constexpr auto horizontal = vec3d(viewport_width, 0, 0);
constexpr auto vertical = vec3d(0, viewport_height, 0);
constexpr auto lower_left_corner =
    origin - horizontal / 2 - vertical / 2 - vec3d(0, 0, focal_length);

bool verbose = false;

template <concepts::arithmetic T = double>
constexpr image_t render() {
  auto world = hittable_list<T>{}
                   .add(sphere<T>(pos3<T>(0, 0, -1), 0.5))
                   .add(sphere<T>(pos3<T>(0, -100.5, -1), 100));

  if (!std::is_constant_evaluated()) std::cout << "rendering..." << std::endl;

  image_t image = {};
  for (size_t h : std::views::iota(0u, constants::image_height)) {
    if (!std::is_constant_evaluated() && verbose)
      std::cout << "rendering row : " << (h + 1) << " / "
                << constants::image_height << std::endl;
    for (size_t w : std::views::iota(0u, constants::image_width)) {
      auto u = w / (constants::image_width - 1.0);
      auto v =
          (constants::image_height - h - 1) / (constants::image_height - 1.0);
      image[h * yk::constants::image_width + w] =
          to_color3b(ray_color(ray(origin, lower_left_corner + u * horizontal +
                                               v * vertical - origin),
                               world));
    }
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
      "output-file,o", po::value<std::string>(), " : specify output filename")("verbose,v"," : verbose");

  po::variables_map vm;
  po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
  po::notify(vm);

  if (vm.contains("help")) {
    desc.print(std::cout);
    return 0;
  }

  yk::verbose = vm.contains("verbose");

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
