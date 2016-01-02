# 0 Greetings
Welcome to the installation!

**Contents**

  1. Requirements
  2. Installation
  3. Running
  4. Debugging

# 1 Requirements
You will need the following libraries, headers and tools:

  * at least g++ 4.8 or clang 3.3
  * boost
  * [Qt4](http://www.qt.io/)
  * [cmake](http://www.cmake.org/)

# 2 Installation
Type in this directory:

```sh
mkdir build
cd build
cmake -DCOMPILER=clang -DCMAKE_BUILD_TYPE=Release ..
```

# 3 Running
You can start the tray icon this:

```sh
cd clang/src
nohup ./megatag &
```

The commandline tool `megatool` can be started like this:

```sh
cd clang/src
source ../../bash_completion
./megatool --help
```

Go to

 * Alt+F1
 * -> System Settings
 * -> Start and Quit (or something similar)

Add the program and make sure that it is executed (work directory)
from where it currently is.

# 4 Debugging
TODO

