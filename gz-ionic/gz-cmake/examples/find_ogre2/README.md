A quick demonstration of searching for Ogre 2.1 vs 2.2 via gz-cmake.

To test with both versions installed:

## Ogre 2.1

```bash
cd ogre-2.1
mkdir build
cd build
cmake ..
```

### Expected output

```bash
-- Looking for GzOGRE2 - found

-- OGRE2_FOUND: TRUE
-- OGRE2_LIBRARIES: /usr/lib/x86_64-linux-gnu/OGRE-2.1/libOgreMain.s GzOGRE2-HlmsPbs: GzOGRE2-HlmsPb GzOGRE2-HlmsUnlit: GzOGRE2-HlmsUnli GzOGRE2-Overlay: GzOGRE2-Overlay
-- OGRE2_INCLUDE_DIRS: /usr/include/OGRE-2.1/usr/include/OGRE-2.1/RenderSystems/GL3Plus
-- OGRE2_VERSION: 2.1.0
-- OGRE2_VERSION_MAJOR: 2
-- OGRE2_VERSION_MINOR: 1
-- OGRE2_VERSION_PATCH: 0
-- OGRE2_RESOURCE_PATH: /usr/lib/x86_64-linux-gnu/OGRE-2.1/OGRE
```

## Ogre 2.2

```bash
cd ogre-2.2
mkdir build
cd build
cmake ..
```

### Expected output


```bash
-- Looking for GzOGRE2 - found

-- OGRE2_FOUND: TRUE
-- OGRE2_LIBRARIES: /usr/lib/x86_64-linux-gnu/OGRE-2.2/libOgreMain.s GzOGRE2-HlmsPbs: GzOGRE2-HlmsPb GzOGRE2-HlmsUnlit: GzOGRE2-HlmsUnli GzOGRE2-Overlay: GzOGRE2-Overlay
-- OGRE2_INCLUDE_DIRS: /usr/include/OGRE-2.2/usr/include/OGRE-2.2/RenderSystems/GL3Plus
-- OGRE2_VERSION: 2.2.6
-- OGRE2_VERSION_MAJOR: 2
-- OGRE2_VERSION_MINOR: 2
-- OGRE2_VERSION_PATCH: 6
-- OGRE2_RESOURCE_PATH: /usr/lib/x86_64-linux-gnu/OGRE-2.2/OGRE
```
