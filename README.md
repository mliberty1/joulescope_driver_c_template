<!--
# Copyright 2014-2023 Jetperch LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
-->

# Joulescope Driver C Template

This project contains a template to get you started writing a custom C
program using the
[Joulescope driver](https://github.com/jetperch/joulescope_driver).


## Getting started

The general process is:

1. Install local build tools.
2. Fork this project on GitHub and clone locally.
3. Build the example.
4. Run the example.  It should display statistics from a connected JS220.
6. Modify CMakeLists.txt with your project name (executable name) and version.  
7. Modify src/main.c to become your desired program.
8. [optional] Push to a new GitHub repo or private repo.


## Install local build tools

The recommended tools are:

1. [git](https://git-scm.com/)
   (to clone the repo, but can just download to skip this step)
2. [cmake](https://cmake.org/)
3. C compiler

**Windows**: download the binaries for git and cmake using the links above.
Install them.  Then install [Visual Studio 2022 Community](https://visualstudio.microsoft.com/downloads/).
Select the "Desctop development with C++" workload which includes the latest MSVC and Windows SDK.

**macOS**: we recommend homebrew](https://brew.sh/).  Follow the installation instructions,
and then `brew install git cmake`

**Ubuntu**: `sudo apt install git cmake build-essential`


## Fork and clone

At a command line, type:

    cd <YOUR_BASE_PATH>
    git clone https://github.com/jetperch/joulescope_driver_c_template.git <YOUR_PROJECT_NAME>
    cd <YOUR_PROJECT_NAME>
    git submodule init
    git submodule update

You should also edit the project name and version in CMakeLists.txt.


## Build the example

Use cmake to configure the build system:

    cd <YOUR_BASE_PATH>/<YOUR_PROJECT_NAME>
    mkdir build
    cd build
    cmake ..
    
From then on, from the project directory, build using:

    cmake --build build

You can then run the executable:

**Windows**: .\build\src\Debug\<project_name>

**macOS** and **Ubuntu**: ./build/src/Debug/<project_name>
