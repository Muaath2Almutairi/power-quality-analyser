cmake_minimum_required(VERSION 3.15)
project(power_quality_analyser C)

# Enforce C99 standard as required by the coursework brief
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable strict warnings to catch common mistakes
add_compile_options(-Wall -Wextra -Wpedantic)

# Collect all .c source files
add_executable(analyser
    main.c
    waveform.c
    io.c
)

# Link the math library (required for sqrt(), fabs())
target_link_libraries(analyser m)
