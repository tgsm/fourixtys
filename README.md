# fourixtys
A work-in-progress Nintendo 64 emulator. It does not boot any games yet. It does, however, pass some tests on [n64-systemtest](https://github.com/lemmy-64/n64-systemtest).

## Building (Linux)
Install [CMake](https://cmake.org) if you haven't already. You'll also need [fmtlib](https://fmt.dev) and [SDL2](https://www.libsdl.org/).
```bash
# Arch Linux
pacman -S cmake fmt sdl2
```

Then follow these steps to build the project:
```bash
# Clone the project
git clone --recursive https://github.com/tgsm/fourixtys.git
cd fourixtys

# Create a build folder and go into it
mkdir build && cd build

# Generate project files
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the project with all threads
make -j$(nproc --all)
```

## License
The project is currently licensed under the [MIT License](LICENSE).
