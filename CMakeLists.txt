cmake_minimum_required(VERSION 2.8.3)
project(video_stream_opencv)
add_definitions(
    -std=c++11
)

#add_definitions(-DBOOST_LOG_DYN_LINK=1)

#set(CMAKE_CXX_FLAGS "-lboost_system")


find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  cv_bridge
  image_transport
  camera_info_manager
  sensor_msgs
  message_generation
  std_msgs
)

find_package(Boost REQUIRED COMPONENTS
  system

)

find_package(OpenCV)

add_service_files(
  FILES
  actvid.srv
  split.srv
)

generate_messages(
  DEPENDENCIES
  std_msgs

)

catkin_package(
 CATKIN_DEPENDS std_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  $(Boost_INCLUDE_DIRS)
)

add_executable(video_stream src/video_stream.cpp)
target_link_libraries(video_stream ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(video_stream_multiple_files src/video_stream_multiple_files.cpp)
# tells catkin to compile messages before the program that uses them. Thanks to https://answers.ros.org/question/73048/first-compile-with-messages-messages-not-found/
add_dependencies(video_stream_multiple_files video_stream_opencv_generate_messages_cpp)
target_link_libraries(video_stream_multiple_files ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(vss src/vsmf_srv.cpp)
add_dependencies(vss video_stream_opencv_generate_messages_cpp)
target_link_libraries(vss ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_executable(readpath src/readpath.cpp)
add_dependencies(readpath ${catkin_EXPORTED_TARGETS} video_stream_opencv_generate_messages_cpp)
target_link_libraries(readpath ${catkin_LIBRARIES} ${Boost_LIBRARIES})

add_executable(readpathp src/readpath_pub.cpp)
add_dependencies(readpathp ${catkin_EXPORTED_TARGETS} video_stream_opencv_generate_messages_cpp)
target_link_libraries(readpathp ${catkin_LIBRARIES} ${Boost_LIBRARIES})

add_executable(readpathv src/readpathv.cpp)
add_dependencies(readpathv ${catkin_EXPORTED_TARGETS} video_stream_opencv_generate_messages_cpp)
target_link_libraries(readpathv ${catkin_LIBRARIES} ${Boost_LIBRARIES})

add_executable(readpathsrv src/readpathv_service.cpp)
add_dependencies(readpathsrv ${catkin_EXPORTED_TARGETS} video_stream_opencv_generate_messages_cpp)
target_link_libraries(readpathsrv ${catkin_LIBRARIES} ${Boost_LIBRARIES})

install(TARGETS video_stream video_stream_multiple_files readpath
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

#install(TARGETS video_stream_multiple_files readpath
#        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

install(PROGRAMS
  scripts/test_video_resource.py

  DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY launch
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
