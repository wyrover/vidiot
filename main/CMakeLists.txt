FILE(GLOB installfiles ${PROJECT_SOURCE_DIR}/install/*.txt)
SOURCE_GROUP("install" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/install/*.*)

SOURCE_GROUP("exe" REGULAR_EXPRESSION ${PROJECT_SOURCE_DIR}/main/src/.* FILES CMakeLists.txt ../icons/icons.rc)

# icons.rc is required to embed the icon file in the executable
ADD_EXECUTABLE(main WIN32 src/main.cpp ${installfiles} ../icons/icons.rc)

TARGET_LINK_LIBRARIES(main vidiot ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${FFMPEG_LIBRARIES} ${PORTAUDIO_LIBRARIES} ${SOUNDTOUCH_LIBRARIES})

COPY_MSVC_PROJECT_USER_FILE(main)

INSTALL(TARGETS main COMPONENT Vidiot DESTINATION bin)

# Copy html files to output folder
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/Debug/html)
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/html ${CMAKE_CURRENT_BINARY_DIR}/Release/html)
# Copy icons to output folder
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Debug/icons)
add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/icons ${CMAKE_CURRENT_BINARY_DIR}/Release/icons)
