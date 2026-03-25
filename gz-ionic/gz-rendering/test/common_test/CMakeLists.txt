set(TEST_TYPE "UNIT")

set(tests
  ArrowVisual_TEST
  AxisVisual_TEST
  BoundingBox_TEST
  BoundingBoxCamera_TEST
  Camera_TEST
  Capsule_TEST
  COMVisual_TEST
  FrustumVisual_TEST
  GaussianNoisePass_TEST
  GizmoVisual_TEST
  GlobalIllumination_TEST
  Grid_TEST
  Heightmap_TEST
  InertiaVisual_TEST
  LensFlarePass_TEST
  LidarVisual_TEST
  Light_TEST
  LightVisual_TEST
  Marker_TEST
  Material_TEST
  Mesh_TEST
  MeshDescriptor_TEST
  MoveToHelper_TEST
  Node_TEST
  OrbitViewController_TEST
  OrthoViewController_TEST
  ParticleEmitter_TEST
  Projector_TEST
  RayQuery_TEST
  RenderEngine_TEST
  RenderEngineManager_TEST
  RenderingIface_TEST
  RenderPassSystem_TEST
  RenderTarget_TEST
  Scene_TEST
  SegmentationCamera_TEST
  Text_TEST
  ThermalCamera_TEST
  TransformController_TEST
  Utils_TEST
  Visual_TEST
  WireBox_TEST
)

foreach(test ${tests})
  gz_rendering_test(
    TYPE ${TEST_TYPE}
    SOURCE ${test}
    LIB_DEPS
      gz-plugin${GZ_PLUGIN_VER}::loader
      gz-common${GZ_COMMON_VER}::gz-common${GZ_COMMON_VER}
      ${PROJECT_LIBRARY_TARGET_NAME}
  )
endforeach()
