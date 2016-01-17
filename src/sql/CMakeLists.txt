add_library(closure SHARED closure.c)
#set_target_properties(closure
#	PROPERTIES OUTPUT_NAME closure.so)
set_target_properties(closure PROPERTIES PREFIX "")

