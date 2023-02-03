# Prometheus
A selection of tech glued together into a sort of general purpose framework. Used for prototyping or experimentation. It's not meant to be an engine.

## Prerequisites

### Linux

* cmake
* libsdl2-dev
* libvulkan-dev
* vulkan-validationlayers-dev

## To build

cd build
cmake ../src
cmake --build .

## Design Goals

Ideal world stuff... 

* Everything should be a job
* The only thing running on the main thread should be the job scheduler
* Scheduling dependencies should be defined in code and job ordering/sync points done at runtime
