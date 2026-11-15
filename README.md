# CPUID-Explorer

A lightweight CPU information utility written in C using MSVC intrinsics and the CPUID instruction.

## Overview

CPUID-Explorer is a Windows-based CPU information utility that retrieves processor information directly through the CPUID instruction.

The project is currently implemented as a console application, with plans to introduce a graphical user interface in the future.

## Sample Output

The following screenshot shows the console output running on the development machine (Intel Core i9-7980XE):

<img width="844" height="1239" alt="image" src="https://github.com/user-attachments/assets/a575e031-fd3d-4088-b4ad-809b2da58785" />

## Current Features

- CPU Vendor Identification
- CPU Brand String
- Maximum Basic CPUID Function
- Maximum Extended CPUID Function
- CPU Family / Model / Stepping
- Processor Type
- Cache Enumeration
- Cache Size Calculation
- Cache Associativity
- Cache Inclusiveness

## Planned Features

- CPU Instruction Set Detection
- CPU Topology Information
- Extended AMD Cache Detection
- GUI
- Real-time CPU Information Dashboard

## Requirements

- Windows x86 / x86-64
- Visual Studio supports C99 features

## Build and Run

1. Clone this repository.
2. Open `CPUID-Explorer.slnx` in Visual Studio.
3. Select the desired platform.
4. Build and run the project.

## Technical Details

The project uses:

- C
- MSVC Intrinsics
- `__cpuid`
- `__cpuidex`
- `<intrin.h>`

## License

GPL-3.0 License
