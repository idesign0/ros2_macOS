file(GLOB files "*.png")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/materials/textures)
