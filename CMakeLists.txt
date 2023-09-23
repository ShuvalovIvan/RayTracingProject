cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(raytracing_project)
set(PROJECT_LIB raytracing_project_lib)
add_library(${PROJECT_LIB} STATIC "")
message(STATUS "Project name ${CMAKE_PROJECT_NAME}, lib ${PROJECT_LIB}")
set( CMAKE_VERBOSE_MAKEFILE on )

# Set the C/C++ specified in the projects as requirements
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if(MSVC)
  # Enable parallel builds by default on MSVC
  string(APPEND CMAKE_C_FLAGS " /MP")
  #string(APPEND CMAKE_C_FLAGS " /fsanitize=address")
  string(APPEND CMAKE_CXX_FLAGS " /MP")
  #string(APPEND CMAKE_CXX_FLAGS " /fsanitize=address")
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:\"mfc110d\"")
  #string(APPEND CMAKE_EXE_LINKER_FLAGS " /fsanitize=address")
endif()

message(STATUS "Source dir = ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR = ${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT OUTPUT_PATH)
  set(OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin CACHE PATH "Directory where outputs will be stored")
endif()

add_definitions(-DPROJECT_NAME="${CMAKE_PROJECT_NAME}")
add_definitions(-DPROJECT_RELDIRECTORY="${TO_CURRENT_SOURCE_DIR}/")

if(WIN32)
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:CONSOLE")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(${exename} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
endif(WIN32)

if(UNIX)
  set(OS "linux")
  add_definitions(-DLINUX)
endif(UNIX)

#####################################################################################
# VulkanSDK package
#
macro(_add_package_VulkanSDK)
  set(ENV{Vulkan_LIBRARY} "$ENV{VK_SDK_PATH}/Lib")
  set(ENV{Vulkan_INCLUDE_DIR} "$ENV{VK_SDK_PATH}/Include")
  message(STATUS "Vulkan lib $ENV{Vulkan_LIBRARY} $ENV{Vulkan_INCLUDE_DIR}")
  find_package(Vulkan REQUIRED
    COMPONENTS glslc glslangValidator shaderc_combined)

  if(Vulkan_FOUND)
      Message(STATUS "--> using package VulkanSDK (linking with ${Vulkan_LIBRARY})")
      get_directory_property(hasParent PARENT_DIRECTORY)
      if(hasParent)
        set( USING_VULKANSDK "YES" PARENT_SCOPE) # PARENT_SCOPE important to have this variable passed to parent. Here we want to notify that something used the Vulkan package
      endif()
      set( USING_VULKANSDK "YES")
      option(VK_ENABLE_BETA_EXTENSIONS "Enable beta extensions provided by the Vulkan SDK" ON)
      add_definitions(-DNVP_SUPPORTS_VULKANSDK)
      if(VK_ENABLE_BETA_EXTENSIONS)
        add_definitions(-DVK_ENABLE_BETA_EXTENSIONS)
      endif()
      add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)

      set(VULKAN_HEADERS_OVERRIDE_INCLUDE_DIR CACHE PATH "Override for Vulkan headers, leave empty to use SDK")

      if (VULKAN_HEADERS_OVERRIDE_INCLUDE_DIR)
        set(vulkanHeaderDir ${VULKAN_HEADERS_OVERRIDE_INCLUDE_DIR})
      else()
        set(vulkanHeaderDir ${Vulkan_INCLUDE_DIR})
      endif()

      Message(STATUS "--> using Vulkan Headers from: ${vulkanHeaderDir}")
      include_directories(${vulkanHeaderDir})
      set( vulkanHeaderFiles
        "${vulkanHeaderDir}/vulkan/vulkan_core.h")
      LIST(APPEND PACKAGE_SOURCE_FILES ${vulkanHeaderFiles})
      source_group(Vulkan FILES ${vulkanHeaderFiles})

      LIST(APPEND LIBRARIES_OPTIMIZED ${Vulkan_LIBRARY})
      LIST(APPEND LIBRARIES_DEBUG ${Vulkan_LIBRARY})

      # CMake 3.24+ finds glslangValidator and glslc for us.
      # On < 3.24, find it manually:
      if(${CMAKE_VERSION} VERSION_LESS "3.24.0")
        get_filename_component(_VULKAN_LIB_DIR ${Vulkan_LIBRARY} DIRECTORY)
        find_file(Vulkan_GLSLANG_VALIDATOR_EXECUTABLE
          NAMES glslangValidator.exe glslangValidator
          PATHS ${_VULKAN_LIB_DIR}/../Bin)
        find_file(Vulkan_GLSLC_EXECUTABLE
          NAMES glslc.exe glslc
          PATHS ${_VULKAN_LIB_DIR}/../Bin)
      endif()
 else()
     Message(STATUS "--> NOT using package VulkanSDK")
 endif()
endmacro()

if(NOT VULKAN_FOUND)
    _add_package_VulkanSDK()
endif()

if (USING_VULKANSDK)
    LIST(APPEND PLATFORM_LIBRARIES ${Vulkan_LIBRARIES})
endif()

if(MSVC)
    add_definitions(/wd26812)  # 'enum class' over 'enum'
    add_definitions(/wd26451)  # Arithmetic overflow, casting 4 byte value to 8 byte value
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
add_subdirectory(assets)
add_subdirectory(src)
add_subdirectory(dependencies/tinyobjloader)
add_subdirectory(dependencies/GLM)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)
target_compile_features(${PROJECT_LIB} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_LIB} PRIVATE ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/dependencies/GLM/glm)

list(APPEND CMAKE_PREFIX_PATH "dependencies/GLFW/lib/cmake")
find_package(glfw3 REQUIRED)
target_link_libraries(${PROJECT_LIB} glfw)

# We check if this is the main file
# you don't usually want users of your library to
# execute tests as part of their build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    add_subdirectory(tests)
endif ()

enable_testing()
