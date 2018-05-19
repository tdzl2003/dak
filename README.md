# Dak

High performance publish-subscribe database which supports huge amount of topics.

See: [wiki](https://github.com/tdzl2003/dak/wiki)

## Status

* Version 0.0.1 under deverlopment.

### MileStones:

#### 0.0.1

* [x] Message subscription/send for local center.
* [ ] Message subscription/send for remote connection.
* [ ] Unit test for message subscription/send.
* [ ] Server executable.

#### 0.1.0

* [ ] Meta subscription/set for local center.
* [ ] Meta subscription/set for remote center.
* [ ] Unit test for meta subscription/set.
* [ ] Configuration support for server executable.

#### 0.2.0

* [ ] Provide docker image & CI build.
* [ ] Subscription over callback with error codes.
* [ ] Connection recovery/retrying.
* [ ] Heart beat.

#### 0.3.0

* [ ] Benchmark for local&remote center.
* [ ] Performance optimize.

#### 1.0.0

* [ ] Support sharding.
* [ ] Bugfixes.
* [ ] Advanced performance optimize.
* [ ] More guides & documents.

#### 1.1.0

* [ ] Support sharding with scaling.

## How to build this project

You should install following softwares:

* CMake (3.11 or above)
* C++ Compiler which support C++11 (Visual Studio 2015+/Xcode/clang/gcc)
* [vcpkg](https://github.com/Microsoft/vcpkg)

Set enviroment variables

```bash
# Linux/MacOS:
export VCPKG_ROOT=*Where you install vcpkg(without trailing slash)*

# Windows:
SET VCPKG_ROOT=*Where you install vcpkg(without trailing slash)*
```

Install boost via vcpkg:

```bash
# Platform default:
vcpkg install boost

# Windows with x64 support:
vcpkg install boost:x64-windows
```

Generate project via `CMake`:

```bash
# Linux/MacOS:
cmake -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake
cd build && make

# Windows:
cmake -H. -Bbuild -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake
# Then open project in build directory.
```
