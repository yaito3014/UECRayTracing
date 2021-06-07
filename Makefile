CXX = g++
CXXFLAGS += -std=c++20
LDFLAGS += -lboost_program_options
COMMON = $(CXX) $(CXXFLAGS) source.cpp $(LDFLAGS)

runtime: raytrace
	./raytrace image.png

compile-time: raytrace_constexpr
	./raytrace_constexpr image.png

raytrace: $(wildcard *.cpp *.hpp)
	$(COMMON) -o raytrace

raytrace_parallel: $(wildcard *.cpp *.hpp)
	$(COMMON) -o raytrace_parallel -DYK_ENABLE_PARALLEL -ltbb

raytrace_constexpr: $(wildcard *.cpp *.hpp)
	$(COMMON) -o raytrace_constexpr -fconstexpr-ops-limit=2100000000 -DYK_ENABLE_CONSTEXPR

clean:
	rm -f *.png raytrace*

.PHONY: clean
