libvcetoy
=========

Toy project to interact with amdgpu's VCE features.

Its main purpose is to expose a small interface for calculating the motion
vectors between two images using the VCE core in AMD GPUs.

This project is a quick and dirty initial exploration to play with the
hardware. Beware of messy code inside.

Features
--------
  - [ ] Basic support for motion vector calculations
    - [x] Submit VCE commands
    - [x] Submit VCE MV command
    - [ ] Macro block size
  - [ ] Vulkan Interop Support

Building
--------

```
mkdir build/
cd build/
meson ..
ninja
```

Tests
-----

```
cd build/test
./vcetoy_test # Needs vulkan and a local X display
```
