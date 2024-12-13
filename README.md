# MonaCPP

MonaCPP is a modern C++ library designed to complement the Standard Library (STL). It provides powerful tools for handling sockets, files, and other utilities, with support for both synchronous and asynchronous IO operations.

MonaCPP is inspired by the [Poco Libraries](https://pocoproject.org/), but with a focus on being lightweight and offering full support for asynchronous operations.

MonaCPP uses  [nlohmann/json](https://github.com/nlohmann/json) for JSON and Value handling.

Special Thanks to their authors.

## Install
```cmake -B build -G Ninja```
```cmake --build build```
```cmake --install build```


## License
MonaCPP is licensed under the MIT License.

## TODO
- Add unit tests.
- Write documentation