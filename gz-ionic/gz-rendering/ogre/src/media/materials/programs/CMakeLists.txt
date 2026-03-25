file(GLOB files "*.glsl")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/materials/programs)

