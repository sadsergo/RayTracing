# SdfTaskTemplate
Template for task 1 (Signed Distance Fields). Part of Forward and Inverse Rendering (FAIR) course, CMC MSU, Spring 2025.

## Build
Clone this repo with its submodules:

    git clone https://github.com/SammaelA/SdfTaskTemplate.git
    cd SdfTaskTemplate
    git submodule update --init

Install SDF (Ubuntu 22)

    sudo apt-get install libsdl2-2.0-0

- For installations on other platforms see https://wiki.libsdl.org/

Build the executable:

    cmake -B build && cmake --build build

## Execute

    ./render

Template visualizes one layer of an SDF grid (example_grid.bin, mode of a bunny)  
use W and S keys to swich between layers.
