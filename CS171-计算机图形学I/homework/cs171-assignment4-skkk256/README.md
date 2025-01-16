[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/-iwZlyFE)
# Assignment 4: Rendering Isosurfaces by Volumetric Techniques

- Student Name: 盛宽
- Student ID: 2021533152

Please read the [documentation](https://faculty.sist.shanghaitech.edu.cn/faculty/liuxp/course/cs171/assignment/assignment4/assignment4.html) carefully.

## Instructions

### Compilation

The code skeleton utilizes cmake as the project building tool. We have added OpenMP support for you in the code skeleton. In case your environment is not compatible with OpenMP, we offer a macro named `NO_OMP` to enable/disable OpenMP in the cmake generation step:

```bash
# Suppose your present working directory is Coding/build/

# Enable OpenMP
cmake -DNO_OMP=0 ..

# Disable OpenMP
cmake -DNO_OMP=1 ..
```

### Execution

Once you manage to compile the project, you can run your volume renderer via typing the command below.

```bash
# Suppose your present working directory is Coding/build/

./volrend ../configs/fw_genus2.txt
```

The "fw_genus2.txt" is a configuration file for rendering diffusion-only scenes, where you can specify the number of available threads, samples per pixel, camera/light settings, output resolutions, etc. It is a plain-text file, where each option is stored in the format `key value`. You can edit "config.hpp" to add your own options if necessary.


## Third-party Libraries

Third-party libraries are placed in the `3rdLibs/` directory. You are NOT allowed to use any third-party libraries other than:

- Eigen
- stb_image_write
- [tinycolormap](https://github.com/yuki-koyama/tinycolormap)


## Copyright

Copyright 2023 ShanghaiTech Univeristy. Code skeleton by Yuehao Wang.
