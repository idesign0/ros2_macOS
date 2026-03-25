file(GLOB files "*.material" "*.compositor")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/materials/scripts)

