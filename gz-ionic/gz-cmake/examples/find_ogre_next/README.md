A quick demonstration of searching for OGRE-Next via gz-cmake.

To test with the pkg-config and the specified version of Ogre installed:

## OGRE-Next

```bash
cd ogre-next
mkdir build
cd build
cmake ..
```

### Expected output

```bash
-- Finding OGRE-Next
-- Found PkgConfig: /usr/bin/pkg-config (found version "2.3.0")
-- Looking for OGRE-Next using the name: OGRE-Next
--   + component HlmsPbs: found
--   + component HlmsUnlit: found
--   + component Overlay: found
-- Looking for GzOGRENext - found

-- OGRE_NEXT_FOUND: TRUE
-- OGRE_NEXT_LIBRARIES: /usr/lib/libOgreNextMain.so/usr/lib/libpthread.aGzOGRENext-HlmsPbs::GzOGRENext-HlmsPbsGzOGRENext-HlmsUnlit::GzOGRENext-HlmsUnlitGzOGRENext-Overlay::GzOGRENext-Overlay
-- OGRE_NEXT_INCLUDE_DIRS: /usr/include/usr/include/OGRE-Next/usr/include/usr/include/OGRE-Next/RenderSystems/GL3Plus
-- OGRE_NEXT_VERSION: 2.3.3
-- OGRE_NEXT_VERSION_MAJOR: 2
-- OGRE_NEXT_VERSION_MINOR: 3
-- OGRE_NEXT_VERSION_PATCH: 3
-- OGRE_NEXT_RESOURCE_PATH: /usr/lib/OGRE-Next
-- GzOGRENext_VERSION_EXACT: FALSE
-- GzOGRENext_VERSION_COMPATIBLE: TRUE
-- Imported targets: OGRE-Next::OGRE-NextGzOGRENext-HlmsPbs::GzOGRENext-HlmsPbsGzOGRENext-HlmsUnlit::GzOGRENext-HlmsUnlitGzOGRENext-Overlay::GzOGRENext-OverlayGzOGRENext::GzOGRENext
```

## OGRE-Next (version 3.0)

Use `PKG_CONFIG_PATH` environment variable to specify the path where to look for `.pc` files of OGRE-Next package.

```bash
cd ogre-next-v3.0
mkdir build
cd build
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig/ cmake ..
```

### Expected output

```bash
-- Finding OGRE-Next of version 3.0
-- Found PkgConfig: /usr/bin/pkg-config (found version "2.3.0")
-- Looking for OGRE-Next using the name: OGRE-Next
--   + component HlmsPbs: found
--   + component HlmsUnlit: found
--   + component Overlay: found
-- Looking for GzOGRENext - found

-- OGRE_NEXT_FOUND: TRUE
-- OGRE_NEXT_LIBRARIES: /usr/local/lib/libOgreNextMain_d.soGzOGRENext-HlmsPbs::GzOGRENext-HlmsPbsGzOGRENext-HlmsUnlit::GzOGRENext-HlmsUnlitGzOGRENext-Overlay::GzOGRENext-Overlay
-- OGRE_NEXT_INCLUDE_DIRS: /usr/local/include/usr/local/include/OGRE-Next/usr/local/include/usr/local/include/OGRE-Next/RenderSystems/GL3Plus
-- OGRE_NEXT_VERSION: 3.0.0
-- OGRE_NEXT_VERSION_MAJOR: 3
-- OGRE_NEXT_VERSION_MINOR: 0
-- OGRE_NEXT_VERSION_PATCH: 0
-- OGRE_NEXT_RESOURCE_PATH: /usr/local/lib/OGRE-Next
-- GzOGRENext_VERSION_EXACT: TRUE
-- GzOGRENext_VERSION_COMPATIBLE: TRUE
-- Imported targets: OGRE-Next::OGRE-NextGzOGRENext-HlmsPbs::GzOGRENext-HlmsPbsGzOGRENext-HlmsUnlit::GzOGRENext-HlmsUnlitGzOGRENext-Overlay::GzOGRENext-OverlayGzOGRENext::GzOGRENext
```
