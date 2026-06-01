# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/components/components/bootloader/subproject"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix/tmp"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix/src/bootloader-stamp"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix/src"
  "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/idf_audio_camera/esp-idf/examples/get-started/audio_camera_Ax_300/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
