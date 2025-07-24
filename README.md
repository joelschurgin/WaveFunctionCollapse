# Optimizations for the Overlapping Wave Function Collapse algorithm.

## GOAL
Implement an algorithm. Optimize it! Time the different versions to find the fastest.

### Original Algorithm
Written in ```main_pattern_caching.cpp```.
Wave function collapse generates an image in the style of a source image. The source image is broken into small patterns and find rules for which patterns can be next to each other based on pixel values. The rules are used to generate a new (usually larger) image.

### OpenMP Parallelization
Written in ```main_omp.cpp```.
Processes neighboring cells in parallel while calculating which possibilities to remove for the current cell.

### Pattern Weight
Written in ```main_pattern_weight.cpp```.
Counts how many times each pattern occurs in the source image. These counts provide a random distribution that we sample from while generating the image.

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
cd packages
```

## Run
For each executable:
```
main_pattern_caching.exe <input_image> --size=<number>
main_omp.exe <input_image> --size=<number>
main_pattern_weight.exe <input_image> --size=<number>
main_seq_Q.exe <input_image> --size=<number>
```

For the measure-performance powershell script:
```
.\measure-performance.ps1
```
or
```
.\measure-performance.ps1 <input_image> --size=<number>
```
No arguments will default to ```Images\Flowers.png``` and ```--size=16```.

