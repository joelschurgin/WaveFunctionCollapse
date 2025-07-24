# Optimizations for the Wave Function Collapse algorithm.

## GOAL
Implement an algorithm. Optimize it! Time different versions to test.

### Original Algorithm 
Written in ```main_no_parallel.cpp```. For those who are unfamiliar, wave function collapse generates an image in the style of a source image. The source image is broken into small patterns and find rules for which patterns can be next to each other based on pixel values. The rules are used to generate a new (usually larger) image.


### OpenMP Parallelization
Written in ```main_omp.cpp```.
Processes neighboring cells in parallel while calculating which possibilities to remove for the current cell.

### Pattern Caching
Written in ```main_pattern_caching.cpp```.
Precomputes and saves where we can using three methods. First, the relationships of compatible patterns, for the all 8 directions of neighboring cells, are stored in a compatibility cache. Second, patterns and indices are stored in a hash map. Third, entropies are stored in a cache which is updated whenever a pattern changes.

### Pattern Weight
Written in ```main_pattern_weight.cpp```.
Builds upon pattern caching method by counting how many times each pattern occurs in the source image. These counts provide a distribution that we sample from while generating the image.

### Sequential Queue
Written in ```main_seq_Q.cpp```.
Instead of looping through the entire output image, cells are processed based on a queue. When a cell's neighbors are visited, they are added to the queue. In addition, cells are marked as queued to prevent adding repeats.

*OpenMP and Sequential Queue variants are based on https://amylh.github.io/WaveCollapseGen/
Images from https://github.com/CodingTrain/Wave-Function-Collapse/tree/main/Processing/WFC_Overlapping_Model/images

## Future Improvements
One issue with the current methodolgy for testing is that sometimes wave function collapse will be unable to fill the entire output image due to having too many constraints and/or some bad luck. When timing these approaches

## Build Instructions
```
make all
make package
```

