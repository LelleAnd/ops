  ## Define constants when using fixed length strings internally and in OPS interface
  ## Max name length for: DomainId, ParticipantId, TopicName, PublisherName, SubscriberName, ...
  ## If you use ops::utilities::nnn() the length need to be able to handle Domain::TopicName
  if ($ENV{OPS_FIXED_OBJECT_NAME_SIZE})
    set(FIXED_OBJECT_NAME_SIZE $ENV{OPS_FIXED_OBJECT_NAME_SIZE})
  else()
    set(FIXED_OBJECT_NAME_SIZE 50)
  endif()

  ## Max length of key set by user on publisher/message and subscriber filter
  if ($ENV{OPS_FIXED_MESSAGE_KEY_SIZE})
    set(FIXED_MESSAGE_KEY_SIZE $ENV{OPS_FIXED_MESSAGE_KEY_SIZE})
  else()
    set(FIXED_MESSAGE_KEY_SIZE 60)
  endif()

  ## Max TypeString length and depends on type names and inheritance depth
  if ($ENV{OPS_FIXED_TYPE_ID_SIZE})
    set(FIXED_TYPE_ID_SIZE $ENV{OPS_FIXED_TYPE_ID_SIZE})
  else()
    set(FIXED_TYPE_ID_SIZE 256)
  endif()

  ## Max name length for: ChannelId
  if ($ENV{OPS_FIXED_CHANNEL_ID_SIZE})
    set(FIXED_CHANNEL_ID_SIZE $ENV{OPS_FIXED_CHANNEL_ID_SIZE})
  else()
    set(FIXED_CHANNEL_ID_SIZE 20)
  endif()

  ## Max length for filename including path when giving ops configuration files
  if ($ENV{OPS_FIXED_FILENAME_SIZE})
    set(FIXED_FILENAME_SIZE $ENV{OPS_FIXED_FILENAME_SIZE})
  else()
    set(FIXED_FILENAME_SIZE 1024)
  endif()

  ## Configure C++ include file with the constant values
  configure_file(OPSStringLengths.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/OPSStringLengths.h)

# Make the headers visible to everything (below this location)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/../Common/idl/Generated/Cpp
)

# We are going to need the boost headers
include_directories(${Boost_INCLUDE_DIRS})

# with msvc and mingw we need to set a platform...
if(WIN32)
  add_definitions(-D_WIN32_WINNT=0x0501)

  # and get rid of some bogus warnings with visual studio
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  endif()
endif()

################################################################################
# Build the source

# add the source
add_subdirectory(source)

################################################################################
# Documentation

# Do documentation targets
if(BUILD_DOCUMENTATION AND DOXYGEN)
  # This processes our *doxyfile.in template
  configure_file(${PROJECT_SOURCE_DIR}/Cpp/ops.doxyfile.in
    ${PROJECT_BINARY_DIR}/doc/ops/ops.doxyfile
    )

  # This creates a new target to build documentation.
  add_custom_target("doc-ops" ${DOXYGEN}
    ${PROJECT_BINARY_DIR}/doc/ops/ops.doxyfile
    )

  # install html documentation
  install(DIRECTORY ${PROJECT_BINARY_DIR}/doc/ops/html
    DESTINATION doc/ops
    COMPONENT ops_doc
  )
endif()
