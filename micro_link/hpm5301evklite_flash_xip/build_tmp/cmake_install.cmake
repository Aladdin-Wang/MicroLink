# Install script for directory: E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm_sdk_localized_for_hpm5301evklite

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "flash_xip")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "E:/software/HPM5300/sdk_env/toolchains/rv32imac_zicsr_zifencei_multilib_b_ext-win/bin/riscv32-unknown-elf-objdump.exe")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/arch/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/boards/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/soc/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/drivers/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/utils/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/components/cmake_install.cmake")
  include("E:/software/HPM5300/sdk_env/hpm_sdk/samples/cherryusb/device/composite/cdc_acm_hid_msc_freertos/hpm5301evklite_flash_xip/build_tmp/middleware/cmake_install.cmake")

endif()

