# Optimizations for the Wave Function Collapse algorithm.
For those who are unfamiliar, wave function collapse generates an image in the style of a source image. The source image is broken into small patterns and find rules for which patterns can be next to each other based on pixel values. The rules are used to generate a new (usually larger) image.

### Original Algorithm 
Written in ```main_no_parallel.cpp```.

### OpenMP Parallelization
Written in ```main_omp.cpp```.
Processes neighboring cells in parallel while calculating which possibilities to remove for the current cell.

### Pattern Caching
Written in ```main_pattern_caching.cpp```.
Precomputes and saves where we can using three methods. First, we find which patterns are compatible and stores the relationships (for all 8 directions) in a compatibility cache. Second, we store patterns and indices in a hash map. Third, we store entropies in a cache which is updated whenever a pattern changes.

### Pattern Weight
Written in ```main_pattern_weight.cpp```.
Builds upon pattern caching method by counting how many times each pattern occurs in the source image. These counts provide a distribution that we sample from while generating the image.

*OpenMP and Sequential Queue variants are based on https://amylh.github.io/WaveCollapseGen/

## Future Improvements
One issue with the current methodolgy for testing is that sometimes wave function collapse will be unable to fill the entire output image due to having too many constraints and/or some bad luck. When timing these approaches

## Build Instructions
```
mkdir build
cd build
cmake ..
```
