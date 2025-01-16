[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/BOMGYPMn)
# Assignment 5: Cloth Simulation with the Mass Spring System

Name: 盛宽

Student ID:  2021533152

**Please write your name and your student ID in this README.**

## Introduction

This is a GUI program using OpenGL for real-time cloth simulation. Please read the following instructions carefully before working on this project.

## Shader path

Before compiling and running the program, you can change the shader path in `test/main.cpp`, about line 60.

If you keep it unchanged, you should set your working directory to the `Coding` folder, or the program will fail finding the shader files.

## Compilation

To compile the program, set your working directory to the `Coding` folder, then

````
mkdir build
cd build
cmake ..
cmake --build . -j 4
````

After running this you only need to execute the last command to compile again.

If you have any trouble compiling, please contact TAs.

You should be able to see a window with some contents in it if you run the compiled program.

## Controls & Settings

To simulate, **hold P key**. Running the program does not start the simulation. Press esc key to quit the program.

A first person camera is provided.

- W, A, S, D, Left Control, Space: Moving
- Arrows: Look around

The simulation parameters can be tuned in `test/main.cpp`, about line 60.

## Extra

If you are not satisfied with the code skeleton, you are encouraged to modify it to achieve more interesting effects. 

For example, you may modify the fragment shader for better lighting setup, or modify the definition of `RectClothSimulator` for extra functionalities.

## Statement

Refer to [course page](https://faculty.sist.shanghaitech.edu.cn/faculty/liuxp/course/cs171/).
