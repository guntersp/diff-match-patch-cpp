
target_sources(${PROJECT_NAME}
  PUBLIC
    diff_match_patch_base.h
    diff_match_patch.h
  )

add_subdirectory(algorithms)
add_subdirectory(traits)
add_subdirectory(types)
add_subdirectory(utils)

