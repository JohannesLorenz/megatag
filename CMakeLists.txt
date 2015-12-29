cmake_minimum_required(VERSION 2.8)
include(cmake/init_project.txt)

# settings
project(megatag)
set(DESCRIPTION "a simple library to tag your files")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "1")

# options

# custom targets

# processing
include(cmake/process_project.txt)
print_summary_base()

