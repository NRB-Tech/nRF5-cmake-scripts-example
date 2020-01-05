This project is an example of implementing a Nordic CMake project from scratch using cmake-nRF5x.

This project depends on the following dependencies:

- [JLink](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack) by Segger - interface software for the JLink familiy of programmers
- [Nordic command line tools](https://www.nordicsemi.com/Software-and-tools/Development-Tools/nRF-Command-Line-Tools/Download#infotabs) (`nrfjprog` and `mergehex`) by Nordic Semiconductor - Wrapper utility around JLink
- [Nordic nrfutil](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_nrfutil%2FUG%2Fnrfutil%2Fnrfutil_intro.html) by Nordic Semiconductor - a utility for generating DFU packages. Currently requires installing with `pip install nrfutil --pre` to install the prerelease 6.0.0 version.  
- [ARM GNU Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) by ARM and the GCC Team - compiler toolchain for embedded (= bare metal) ARM chips. On a Mac, can be installed with homebrew:
    ```shell
    brew tap ArmMbed/homebrew-formulae
    brew install arm-none-eabi-gcc
    ```

To test this project from the repo, run
```shell
cmake -Bcmake-build-download -G "Unix Makefiles"
cmake --build cmake-build-download/ --target download
```

## Recreate this project

The steps below are also available at https://nrbtech.io/blog/2020/1/4/using-cmake-for-nordic-nrf52-projects

First, lets create a directory, and initialise a git repo:

```shell
# create the directory
mkdir "example"
cd example
# init git
git init
# fetch boilerplate .gitignore
curl https://www.gitignore.io/api/linux,macos,cmake,clion,python,windows -o .gitignore
# append project-specific gitignore additions
echo "

toolchains
cmake-build-*
keys
" >> .gitignore
```

Next, we need to add the cmake-nRF5x project:

```shell
git submodule add https://github.com/nrbrook/cmake-nRF5x
```

Then, copy the example `CMakeLists.txt` as recommended in the readme:
```shell
cp cmake-nRF5x/example/CMakeLists.txt .
mkdir src
cp cmake-nRF5x/example/src/CMakeLists.txt src/
```

We need to edit the example `CMakeLists.txt` to update the path to the script, near the bottom:

```cmake
include("${CMAKE_CURRENT_LIST_DIR}/cmake-nRF5x/CMake_nRF5x.cmake")
```

Then we can use the script to download the dependencies:

```shell
cmake -Bcmake-build-download -G "Unix Makefiles"
cmake --build cmake-build-download/ --target download
```

Then, copy across some files from an SDK example project

```shell
cp toolchains/nRF5/nRF5_SDK_16.0.0_98a08e2/examples/ble_peripheral/ble_app_blinky/pca10040/s132/armgcc/ble_app_blinky_gcc_nrf52.ld src/gcc_nrf52.ld
cp toolchains/nRF5/nRF5_SDK_16.0.0_98a08e2/examples/ble_peripheral/ble_app_blinky/pca10040/s132/config/sdk_config.h src/
```

At this point, you can open the project in CLion or your editor of choice to edit the files. See the [CLion tutorial](https://www.nrbtech.io/blog/2020/1/4/using-clion-for-nordic-nrf52-projects) for steps setting it up.

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
}
```

Create an `src/app_config.h` file to just modify the parts of `sdk_config.h` we want:

```c
#define NRF_LOG_BACKEND_RTT_ENABLED 1 // enable rtt
#define NRF_LOG_BACKEND_UART_ENABLED 0 // disable uart
#define NRF_LOG_DEFERRED 0 // flush logs immediately
#define NRF_LOG_ALLOW_OVERFLOW 0 // no overflow
#define SEGGER_RTT_CONFIG_DEFAULT_MODE 2 // block until processed
```

We are going to include the DFU bootloader too, so we need to generate keys:

```shell
mkdir keys
nrfutil keys generate keys/dfu_private.key
nrfutil keys display --key pk --format code keys/dfu_private.key --out_file keys/dfu_public_key.c
```

After these changes we need to update our `src/CMakeLists.txt` file from the example version:

```cmake
string(SUBSTRING ${PLATFORM} 0 5 NRF_FAMILY)
set(NRF5_LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/gcc_${NRF_FAMILY})

# DFU requirements
# List the softdevice versions previously used
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
nRF5x_addLog()
nRF5x_addSeggerRTT()
nRF5x_addAppError()

# usually you would include files in this directory here, like so:
list(APPEND SOURCE_FILES
        main.c
        )
list(APPEND INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )

nRF5x_addExecutable(${target} "${SOURCE_FILES}" "${INCLUDE_DIRS}" "${NRF5_LINKER_SCRIPT}")

target_compile_definitions(${target} PRIVATE USE_APP_CONFIG)

# Here you would set a list of user variables to be defined for the bootloader makefile (which you have modified yourself)
set(bootloader_vars "")

# add the secure bootloader build target
nRF5x_addSecureBootloader(${target} "${PUBLIC_KEY}" "${bootloader_vars}")
# add the bootloader merge target
nRF5x_addBootloaderMergeTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE} ${SD_VALIDATION_TYPE} ${BOOTLOADER_VERSION})
# add the bootloader merged flash target
nRF5x_addFlashTarget(bl_merge_${target} "${CMAKE_CURRENT_BINARY_DIR}/${target}_bl_merged.hex")
# Add the Bootloader + SoftDevice + App package target
nRF5x_addDFU_BL_SD_APP_PkgTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE} ${SD_VALIDATION_TYPE} ${BOOTLOADER_VERSION})
# Add the App package target
nRF5x_addDFU_APP_PkgTarget(${target} ${DFU_VERSION_STRING} ${PRIVATE_KEY} ${PREVIOUS_SOFTDEVICES} ${APP_VALIDATION_TYPE})

# print the size of consumed RAM and flash
nRF5x_print_size(${target} ${NRF5_LINKER_SCRIPT} TRUE)
```

Then we are ready to build and run our example. First, run JLink tools to get the RTT output:

```shell
cmake -Bcmake-build-debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug/ --target START_JLINK
```

Then, build the merge and flash target:
```shell
cmake --build cmake-build-debug/ --target flash_bl_merge_example
```

You should see the "Hello world" log output in the RTT console! From here you can add source code and include SDK libraries with the macros provided in `cmake-nRF5x/includes/libraries.cmake`.

