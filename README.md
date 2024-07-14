# Fetch

![Fetch tool Screenshot](./fetch.png)

Fast system information fetcher for Linux in C++.

**Note: Tested only on my system. Will not work on wayland**

# Install

- Make sure you have a C++ compiler.
- Compile and move the binary to your $PATH e.g
  ```sh
  g++ -std=c++17 -Ofast -lX11 fetch.cpp -o fetch
  mv fetch "$HOME"/.local/bin
  ```
  or
  ```sh
  clang++ -std=c++17 -Ofast -lX11 fetch.cpp -o fetch
  mv fetch "$HOME"/.local/bin
  ```
