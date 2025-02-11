# ParaCL

## Introduction

This project is an implementation of the frontend and interpreter of the ParaCL language.

## Build and Run

Cloning repository:
```
git clone https://github.com/ask0later/ParaCL
```

If you want to build the project, write this in the project directory:
```
cmake -S . -B build
cmake --build build
```

After that, you can run main target program:

```
./build/src/Interpretator [file program]
```

## Tests
### End to end

If you want to run end-to-end tests, type it:
```
python3 tests/end-to-end/check_tests.py
```