
if( NOT TARGET Cinder-KCB2 )
	get_filename_component( Cinder-KCB2_INC_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE )
	get_filename_component( Cinder-KCB2_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( Cinder-KCB2_LIB_PATH "${CMAKE_CURRENT_LIST_DIR}/../../lib" ABSOLUTE )
	set(KINECT_INCLUDE_DIR "C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409/inc")
	set(KINECT_LIB_DIR "C:/Program Files/Microsoft SDKs/Kinect/v2.0_1409/lib")

	set( Cinder-KCB2_INCLUDES
		${Cinder-KCB2_INC_PATH}/Kinect2.h
		${Cinder-KCB2_LIB_PATH}/KCBv2Lib.h
	)

	message(KCB2_LIB_PATH.................${Cinder-KCB2_LIB_PATH})

	set( Cinder-KCB2_SOURCES
		${Cinder-KCB2_SOURCE_PATH}/Kinect2.cpp
	)

	list( APPEND Cinder-KCB2_LIBRARIES
		${Cinder-KCB2_INCLUDES}
		${Cinder-KCB2_SOURCES}
	)

	source_group( "src" FILES ${Cinder-KCB2_SOURCES} )
	source_group( "include" FILES ${Cinder-KCB2_INCLUDES} )

	add_library( Cinder-KCB2 ${Cinder-KCB2_LIBRARIES} )
	target_include_directories( Cinder-KCB2 PUBLIC "${CINDER_PATH}/include" ${KINECT_INCLUDE_DIR} ${Cinder-KCB2_LIB_PATH} ${Cinder-KCB2_INC_PATH} )
	#target_include_directories( ${PROJECT_NAME} PUBLIC ${KINECT_INCLUDE_DIR} ${Cinder-KCB2_LIB_PATH} ${Cinder-KCB2_INC_PATH} )
	target_link_libraries(Cinder-KCB2 PUBLIC
		${Cinder-KCB2_LIB_PATH}/x64/KCBv2.lib
		${KINECT_LIB_DIR}/x64/kinect20.lib
		${KINECT_LIB_DIR}/x64/Kinect20.Face.lib
	)
endif()
