CXX = g++
LD_OPTION = -lboost_program_options

runtime: raytrace
	./raytrace image.png

compile-time: raytrace_constexpr
	./raytrace_constexpr image.png

raytrace: $(wildcard *.cpp *.hpp)
	$(CXX) -std=c++20 -o raytrace source.cpp $(LD_OPTION)

raytrace_parallel: $(wildcard *.cpp *.hpp)
	$(CXX) -std=c++20 -o raytrace_parallel source.cpp -DYK_ENABLE_PARALLEL -ltbb $(LD_OPTION)

raytrace_constexpr: $(wildcard *.cpp *.hpp)
	$(CXX) -std=c++20 -o raytrace_constexpr source.cpp -fconstexpr-ops-limit=2100000000 -DYK_ENABLE_CONSTEXPR $(LD_OPTION)

clean:
	rm -f *.png raytrace*

.PHONY: clean
