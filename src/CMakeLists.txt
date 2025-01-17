cmake_minimum_required(VERSION 3.13.4)
project(Interpretator)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=leak,address,undefined")

if (NOT DEFINED GRAMMAR)
  set(GRAMMAR "parser.yy" CACHE STRING "file with grammar of language" FORCE)
endif()

flex_target(scanner
  lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.cc
)

bison_target(parser
  ${GRAMMAR}
  ${CMAKE_CURRENT_BINARY_DIR}/parser.cc
  COMPILE_FLAGS "--defines=${CMAKE_CURRENT_BINARY_DIR}/parser.tab.hh")

add_flex_bison_dependency(scanner parser)

add_executable(${PROJECT_NAME} 
  driver.cpp  
  ${BISON_parser_OUTPUTS}
  ${FLEX_scanner_OUTPUTS}
)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
set(INCLUDE_DIR1 ${CMAKE_SOURCE_DIR}/third_party)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR1})

set(TARGETS
  ${PROJECT_NAME}
)

foreach(TNAME ${TARGETS})
  target_compile_features(${TNAME} PRIVATE cxx_std_20)
  target_include_directories(${TNAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
  target_include_directories(${TNAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()