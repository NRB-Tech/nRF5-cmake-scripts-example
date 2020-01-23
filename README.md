This project is an example of implementing a Nordic CMake project from scratch using nRF5-cmake-scripts.

This project depends on the following dependencies:

- A github account and SSH key configured
- [Nordic command line tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs) (`nrfjprog` and `mergehex`) by Nordic Semiconductor - Wrapper utility around JLink
    - This also includes the JLink installer – install this
- [Python 3](https://www.python.org/)
- [Nordic nrfutil](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrfutil%2FUG%2Fnrfutil%2Fnrfutil_intro.html) by Nordic Semiconductor - a utility for generating DFU packages
    - After installing Python, install with `pip install nrfutil`  
- ARM GNU Toolchain by ARM and the GCC Team - compiler toolchain for embedded ARM chips.
    - On a Mac, can be installed with homebrew:
        ```shell
        brew tap ArmMbed/homebrew-formulae
        brew install arm-none-eabi-gcc
        ```
    - On other platforms you can download from the [GNU-ARM toolchain page](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads)
    - On Windows, you need to add this to your PATH, see below
- [git](https://git-scm.com/) - A version control system
- [CMake](https://cmake.org/) - A build tool

On Mac and Linux, your PATH is probably configured correctly automatically, but if not or on Windows you will need to ensure that all binaries are in your system PATH, or you can provide some executable paths directly in your CMake script (see errors when generating). On Windows, your PATH might need to look something like this:
```
C:\Program Files (x86)\GNU Tools Arm Embedded\9 2019-q4-major\bin;C:\Program Files\Nordic Semiconductor\nrf-command-line-tools\bin;%USERPROFILE%\AppData\Local\Programs\Python\Python36\Scripts;C:\Program Files\Git\cmd;C:\Program Files (x86)\SEGGER\JLink
```

To test this project after cloning from the repo, run
```shell
cmake -Bcmake-build-download -G "Unix Makefiles" .
cmake --build cmake-build-download/ --target download
```

## Recreate this project

The steps below are also available at https://nrbtech.io/blog/2020/1/4/using-cmake-for-nordic-nrf52-projects

First, in a terminal/command window clone the [base project](https://github.com/NRB-Tech/nRF5-cmake-scripts-example-base) to set up the project structure:

```shell
git clone --recurse-submodules https://github.com/NRB-Tech/nRF5-cmake-scripts-example-base.git .
```

Run a script to clean up the project ready for your own use (on Windows, run in git bash by right clicking in directory > "Git Bash here"):
```shell
./cleanup
```

Copy the example `CMakeLists.txt` as recommended in the `nRF5-cmake-scripts` readme:
```shell
cmake -E copy nRF5-cmake-scripts/example/CMakeLists.txt .
```

_Note: You may also need to edit some of the variables in this file for your platform, such as setting `NRFJPROG`, `MERGEHEX`, `NRFUTIL` and `PATCH_EXECUTABLE` manually if they are not in your `PATH`._

Then we can use the script to download the dependencies:

```shell
cmake -Bcmake-build-download -G "Unix Makefiles" .
# on Windows, run `cmake -Bcmake-build-download -G "MinGW Makefiles" .`
cmake --build cmake-build-download/ --target download
```

Copy across some files from an SDK example project:

```shell
cmake -E copy toolchains/nRF5/nRF5_SDK_16.0.0_98a08e2/examples/ble_peripheral/ble_app_template/pca10040/s132/armgcc/ble_app_template_gcc_nrf52.ld src/gcc_nrf52.ld
cmake -E copy toolchains/nRF5/nRF5_SDK_16.0.0_98a08e2/examples/ble_peripheral/ble_app_template/pca10040/s132/config/sdk_config.h src/
```

At this point, you can open the project in CLion or your editor of choice to edit the files.

Add a file `src/main.c` and add some source code. Here we add some simple code to log a message.

```c
#include "nrf_log_default_backends.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

int main(void) {
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();

    NRF_LOG_INFO("Hello world");
    while(true) {
        // do nothing
    }
}
```

Create an `src/app_config.h` file to override some of the default configuration in `sdk_config.h`:

```c
#define NRF_LOG_BACKEND_RTT_ENABLED 1 // enable rtt
#define NRF_LOG_BACKEND_UART_ENABLED 0 // disable uart
#define NRF_LOG_DEFERRED 0 // flush logs immediately
#define NRF_LOG_ALLOW_OVERFLOW 0 // no overflow
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 2 // block until processed
```

We are going to include the DFU bootloader too, so we need to generate keys. In terminal/command prompt:

```shell
nrfutil keys generate keys/dfu_private.key
nrfutil keys display --key pk --format code keys/dfu_private.key --out_file keys/dfu_public_key.c
```

Now we need to create a file `src/CMakeLists.txt` to build our targets:

```cmake
set(NRF5_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gcc_${NRF_FAMILY})

# DFU requirements
# List the softdevice versions previously used, or use FALSE if no previous softdevices
set(PREVIOUS_SOFTDEVICES FALSE)
# Set the location to the DFU private key
set(PRIVATE_KEY ${CMAKE_CURRENT_SOURCE_DIR}/../keys/dfu_private.key)
set(PUBLIC_KEY ${CMAKE_CURRENT_SOURCE_DIR}/../keys/dfu_public_key.c)
# Set the App validation type. [NO_VALIDATION|VALIDATE_GENERATED_CRC|VALIDATE_GENERATED_SHA256|VALIDATE_ECDSA_P256_SHA256]
set(APP_VALIDATION_TYPE NO_VALIDATION)
# Set the Soft Device validation type. [NO_VALIDATION|VALIDATE_GENERATED_CRC|VALIDATE_GENERATED_SHA256|VALIDATE_ECDSA_P256_SHA256]
set(SD_VALIDATION_TYPE NO_VALIDATION)
# The bootloader version (user defined)
set(BOOTLOADER_VERSION 1)
# The DFU version string (firmware version string)
set(DFU_VERSION_STRING "${VERSION_STRING}")

# Set the target name
set(target example)

# add the required libraries for this example
nRF5_addLog()
nRF5_addSeggerRTT()
nRF5_addAppError()

# include files
list(APPEND SOURCE_FILES
        main.c
        )
list(APPEND INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )

nRF5_addExecutable(${target} "${SOURCE_FILES}" "${INCLUDE_DIRS}" "${NRF5_LINKER_SCRIPT}")

# make sdk_config.h import app_config.h
target_compile_definitions(${target} PRIVATE USE_APP_CONFIG)

# Here you can set a list of user variables to be defined in the bootloader makefile (which you have modified yourself)
set(bootloader_vars "")

# add the secure bootloader build target
nRF5_addSecureBootloader(${target} "${PUBLIC_KEY}" "${bootloader_vars}")
# add the bootloader merge target
nRF5_addBootloaderMergeTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE} ${SD_VALIDATION_TYPE} ${BOOTLOADER_VERSION})
# add the bootloader merged flash target
nRF5_addFlashTarget(bl_merge_${target} "${CMAKE_CURRENT_BINARY_DIR}/${target}_bl_merged.hex")
# Add the Bootloader + SoftDevice + App package target
nRF5_addDFU_BL_SD_APP_PkgTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE} ${SD_VALIDATION_TYPE} ${BOOTLOADER_VERSION})
# Add the App package target
nRF5_addDFU_APP_PkgTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE})

# print the size of consumed RAM and flash - does not yet work on Windows
if(NOT ${CMAKE_HOST_SYSTEM_NAME} STREQUAL "Windows")
    nRF5_print_size(${target} ${NRF5_LINKER_SCRIPT} TRUE)
endif()
```

Then we are ready to build and run our example. First, run JLink tools to get the RTT output:

```shell
cmake -Bcmake-build-debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug .
# On Windows, run `cmake -Bcmake-build-debug -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug .` 
# If you get an error that the compiler cannot be found, ensure it is present in your PATH (try running `arm-none-eabi-gcc`). Windows users, see the dependencies section.
cmake --build cmake-build-debug/ --target START_JLINK_RTT
```

Then, build the merge and flash target:
```shell
cmake --build cmake-build-debug/ --target flash_bl_merge_example
```

You should see the "Hello world" log output in the RTT console! From here you can add source code and include SDK libraries with the macros provided in `nRF5-cmake-scripts/includes/libraries.cmake`.

To debug using CLion, follow the instructions in the [CLion tutorial](https://www.nrbtech.io/blog/2020/1/4/using-clion-for-nordic-nrf52-projects).
