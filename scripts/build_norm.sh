#!/bin/bash

rm -rf CMake* src cmake*  Make*  bsat lib buddy tbsat
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j4
