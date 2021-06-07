LD_OPTION = -lboost_program_options

runtime: raytrace
	./raytrace image.png

compile-time: raytrace_constexpr
	./raytrace_constexpr image.png

raytrace: $(wildcard *.cpp *.hpp)
	g++-10 -std=c++20 -o raytrace source.cpp $(LD_OPTION)

raytrace_constexpr: $(wildcard *.cpp *.hpp)
	g++-10 -std=c++20 -fconstexpr-ops-limit=2100000000 -DYK_ENABLE_CONSTEXPR -o raytrace_constexpr source.cpp $(LD_OPTION)

clean:
	rm -f *.png raytrace*

.PHONY: clean
