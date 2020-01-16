# Another Game Engine
A game engine that serves as my graphics programming playground.

### Building
For Windows, I prefer MSYS2 for this, since it's almost exactlylike linux. So you are going to need these packages:
```shell
SDL2
```

Simply get the dependencies:
```shell
pacman -S mingw-w64-x86_64-SDL2
```
(replace x86_64 with i686 if you're running a 32bit OS (unlikely))

#### Now just build it
```shell
mkdir -p build
cd build
cmake -G "MSYS Makefiles" .. 
make -j4
```