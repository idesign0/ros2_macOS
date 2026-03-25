file(GLOB files "*ttf" "*png" "*fontdef")

install(FILES ${files} DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/fonts)
install(DIRECTORY liberation-sans DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/fonts)
install(DIRECTORY roboto DESTINATION ${GZ_RENDERING_RELATIVE_RESOURCE_PATH}/ogre/media/fonts)
