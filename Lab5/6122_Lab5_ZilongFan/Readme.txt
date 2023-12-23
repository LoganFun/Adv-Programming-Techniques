Since there is a building error at pace-ice, I add

	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

to the CMakeList.txt, which is outside the example folder. 

Without this change, i cannot build them on pace-ice. However, I can build the file on Ubuntu without this change. 