License: See LICENSE file in base directory.

This software has two main purposes.
1) The user can interactively select two patches to compare.
2) The user can see the list of "top patches" to the selected target patch. This section of the program
also allows the top patches to be clustered.

Required Dependencies
---------------------
Eigen 3
VTK 6
ITK 4

Installation Notes
------------------
ITK MUST be configured and then bulit with 

ccmake ~/src/ITK -DCMAKE_CXX_FLAGS=-std=gnu++0x

the VERY first time. I.e. you cannot configure once, then set the CMAKE_CXX_FLAGS, then configure again.