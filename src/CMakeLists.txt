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
nRF5_addLog()
nRF5_addSeggerRTT()
nRF5_addAppError()

# usually you would include files in this directory here, like so:
list(APPEND SOURCE_FILES
        main.c
        )
list(APPEND INCLUDE_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}"
        )

nRF5_addExecutable(${target} "${SOURCE_FILES}" "${INCLUDE_DIRS}" "${NRF5_LINKER_SCRIPT}")

target_compile_definitions(${target} PRIVATE USE_APP_CONFIG)

# Here you would set a list of user variables to be defined for the bootloader makefile (which you have modified yourself)
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
if(NOT WIN32)
    nRF5_print_size(${target} ${NRF5_LINKER_SCRIPT} TRUE)
endif()