
if( NOT TARGET Cinder-Link )
	get_filename_component( Link_PATH "${CMAKE_CURRENT_LIST_DIR}/../../deps/link" ABSOLUTE )
	get_filename_component( Cinder-Link_INC_PATH "${CMAKE_CURRENT_LIST_DIR}/../../include" ABSOLUTE )
	get_filename_component( Cinder-Link_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	
	# set( NanoVG_SOURCES
	# 	${NanoVG_SOURCE_PATH}/fontstash.h
	# 	${NanoVG_SOURCE_PATH}/nanovg_gl_utils.h
	# 	${NanoVG_SOURCE_PATH}/nanovg_gl.h
	# 	${NanoVG_SOURCE_PATH}/nanovg.h
	# 	${NanoVG_SOURCE_PATH}/nanovg.c
	# 	${NanoVG_SOURCE_PATH}/stb_image.h
	# 	${NanoVG_SOURCE_PATH}/stb_truetype.h
	# )

	# source_group(NanoVG FILES ${NanoVG_SOURCES})

	set (Link_SOURCES
		${Link_PATH}/examples/linkaudio/AudioEngine.hpp
		${Link_PATH}/examples/linkaudio/AudioEngine.cpp
		${Link_PATH}/examples/linkaudio/AudioPlatform_Dummy.hpp
		${Link_PATH}/modules/asio-standalone/asio/include/asio.hpp
	)

	set(Link_INCLUDE_DIRS
		${Link_PATH}/include
		${Link_PATH}/examples/linkaudio
		#${APP_PATH}/third_party/asio/asio/include
		${Link_PATH}/modules/asio-standalone/asio/include
	)
	source_group(Link FILES ${Link_SOURCES})

	set( Cinder-Link_INCLUDES
		${Cinder-Link_INC_PATH}/LinkWrapper.h
	)

	set( Cinder-Link_SOURCES
		${Cinder-Link_SOURCE_PATH}/LinkWrapper.cpp
	)

	list( APPEND Cinder-Link_LIBRARIES
		${NanoVG_SOURCES}
		${Cinder-Link_INCLUDES}
		${Cinder-Link_SOURCES}
	)

	source_group( "deps\\link\\src" FILES ${NanoVG_SOURCES} )
	source_group( "src" FILES ${Cinder-Link_SOURCES} )
	source_group( "include" FILES ${Cinder-Link_INCLUDES} )

	add_library( Cinder-Link ${Cinder-Link_LIBRARIES} )
	target_include_directories(Cinder-Link PUBLIC "${CINDER_PATH}/include" ${Link_SOURCE_PATH} ${Link_INCLUDE_DIRS} ${Cinder-Link_INC_PATH} )
	target_compile_definitions(Cinder-Link PUBLIC LINK_PLATFORM_WINDOWS=1)

endif()
