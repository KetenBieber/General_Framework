cmake_minimum_required(VERSION 3.16)

#将src工作目录的绝对路径赋给SRC_DIR
file(GLOB_RECURSE APP_SRC_DIR "tests/*.c")
file(GLOB_RECURSE UNITY_SRC_LIST "Unity/*.c")

# 项目名称
project(UnitTest)

# 头文件路径
include_directories("tests")
include_directories("Unity/src")
include_directories("Unity/extras/fixture/src")
include_directories("Unity/extras/memory/src")

#将所有源文件生成一个可执行文件
add_executable(TDD_test  ${APP_SRC_DIR} ${UNITY_SRC_LIST})
