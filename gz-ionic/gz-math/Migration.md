# Note on deprecations
A tick-tock release cycle allows easy migration to new software versions.
Obsolete code is marked as deprecated for one major release.
Deprecated code produces compile-time warnings. These warning serve as
notification to users that their code should be upgraded. The next major
release will remove the deprecated code.

## Gazebo Math 7.X to 8.X

### Breaking Changes

1. All references to `ignition` have been removed. This includes the `ignition/`
   header files, the `ignition::` namespaces, and `IGN_*` and `IGNITION_*`
   macros.

1. Behavior change in `math::Color` clamping functionality. Previously when
   any of the color r, g, or b values are larger than 1, they are assumed to
   be in the range of [0, 255] and the values are divided by 255
   by the `math::Color::Clamp` function. This is no longer the case. Users are
   expected to specify floating point r, g, b values in the range of [0, 1].
   Any values larger than 1 will be clamped to 1.

1. `math::SphericalCoordinates::LOCAL` now gives ENU coordinates instead of WNU.
Both `math::SphericalCoordinates::LOCAL2` and
`math::SphericalCoordinates::LOCAL` have the same behaviour now with
`math::SphericalCoordinates::LOCAL2` being deprecated.

1. Deprecated functionality from version 7 has been removed.

### Deprecations

1. **Stopwatch.hh**
    + The `math::clock` type alias is deprecated. Please use the underlying type
      (`std::chrono::steady_clock`) directly.

1. **graph/Vertex.hh**
    + The `Vertex::NullVertex` static member is deprecated. Please use
      `Vertex::NullVertex()` instead.
      E.g.: https://github.com/gazebosim/gz-math/pull/606/files#diff-0c0220a7e72be70337975433eeddc3f5e072ade5cd80dfb1ac03da233c39c983L153-R153

1. **graph/Edge.hh**
    + The `Edge::NullEdge` static member is deprecated. Please use
      `Vertex::NullEdge()` instead.
      E.g.: https://github.com/gazebosim/gz-math/pull/606/files#diff-0c0220a7e72be70337975433eeddc3f5e072ade5cd80dfb1ac03da233c39c983L222-R222

1. **SphericalCoordinates.hh**
    + Deprecated: `gz::math::Vector3d SphericalFromLocalPosition(
        const gz::math::Vector3d &_xyz) const`
    + Replacement: `std::optional<math::CoordinateVector3> SphericalFromLocalPosition(
        const gz::math::CoordinateVector3 &) const`
    + Deprecated: `gz::math::Vector3d GlobalFromLocalVelocity(
        const gz::math::Vector3d &_xyz) const`
    + Replacement: `std::optional<math::CoordinateVector3> GlobalFromLocalVelocity(
        const gz::math::CoordinateVector3 &) const`
    + Deprecated: `gz::math::Vector3d LocalFromSphericalPosition(
        const gz::math::Vector3d &) const`
    + Replacement: `std::optional<math::CoordinateVector3> LocalFromSphericalPosition(
        const gz::math::CoordinateVector3 &) const`
    + Deprecated: `gz::math::Vector3d LocalFromGlobalVelocity(
        const gz::math::Vector3d &_xyz) const`
    + Replacement: `std::optional<math::CoordinateVector3> LocalFromGlobalVelocity(
        const gz::math::CoordinateVector3 &_xyz) const`
    + Deprecated: `gz::math::Vector3d PositionTransform(const gz::math::Vector3d &,
        const CoordinateType &, const CoordinateType &) const`
    + Replacement: `std::optional<gz::math::CoordinateVector3>
        PositionTransform(const gz::math::CoordinateVector3 &,
        const CoordinateType &, const CoordinateType &) const`
    + Deprecated: `gz::math::Vector3d VelocityTransform(
        const gz::math::Vector3d &,
        const CoordinateType &, const CoordinateType &) const`
    + Replacement: `std::optional<gz::math::CoordinateVector3> VelocityTransform(
        const gz::math::CoordinateVector3 &,
        const CoordinateType &, const CoordinateType &) const`
    + `math::SphericalCoordinates::LOCAL2` enum is deprecated. Please use
      `math::SphericalCoordinates::LOCAL` with conversion functions that take a
      `math::CoordinateVector3` instead.

## Gazebo Math 6.X to 7.X

### Breaking Changes

  1. Removed the Quaternion integer template `Quaternioni`.

  1. The project name has been changed to use the `gz-` prefix, you **must** use the `gz` prefix!

    * This also means that any generated code that use the project name (e.g. CMake variables, in-source macros) would have to be migrated.
    * Some non-exhaustive examples of this include:
      * `GZ_<PROJECT>_<VISIBLE/HIDDEN>`
      * CMake `-config` files
      * Paths that depend on the project name

  1. Python library imports such `import ignition.math` and `from ignition import math` should be replaced with `import gz.math7` and `from gz import math7`. Note the change from `ignition` to `gz` and the addition of the major version number as a suffix to the package name.

### Deprecations

1. **Angle.hh**
    + All mutator functions that lacked a `Set` prefix have been deprecated
    and replaced by version with a `Set` prefix.

1. **SphericalCoordinates.hh**
    + ***Deprecation:*** public: static double Distance(const Angle&, const Angle&, const Angle&, const Angle&)
    + ***Replacement:*** public: static double DistanceWGS84(const Angle&, const Angle&, const Angle&, const Angle&)

1. **Matrix3.hh**
    + ***Deprecation:*** public: void Axes(const Vector3<T> &, const Vector3<T> &, const Vector3<T> &)
    + ***Replacement:*** public: void SetAxes(const Vector3<T> &, const Vector3<T> &, const Vector3<T> &)

    + ***Deprecation:*** public: void Axis(const Vector3<T> &, T)
    + ***Replacement:*** public: void SetFromAxisAngle(const Vector3<T> &, T)
    + ***Deprecation:*** public: void From2Axes(const Vector3<T> &, const Vector3<T> &)
    + ***Replacement:*** public: void SetFrom2Axes(const Vector3<T> &, const Vector3<T> &)
    + ***Deprecation:*** public: void Col(unsigned int, const Vector3<T> &)
    + ***Replacement:*** public: void SetCol(unsigned int, const Vector3<T> &)

1. **Pose3.hh**
    + The addition operators `+` and `+=` are deprecated in favor of multiplication
      operators `*` and `*=`, though the order of operands is reversed
      (A + B = B * A).
    + The unitary negation operator `-` is deprecated in favor of the
      `Inverse()` method.
    + The subtraction operators `-` and `-=` are deprecated in favor of multiplication
      by an inverse, though the order of operands is reversed
      (A - B = B.Inverse() * A).

1. **Quaternion.hh**
    + ***Deprecation:*** public: void Axis(T, T, T, T)
    + ***Replacement:*** public: void SetFromAxisAngle(T, T, T, T)
    + ***Deprecation:*** public: void Axis(const Vector3<T>&, T)
    + ***Replacement:*** public: void SetFromAxisAngle(const Vector3<T>&, T)
    + ***Deprecation:*** public: void Euler(const Vector3<T> &)
    + ***Replacement:*** public: void SetFromEuler(const Vector3<T> &)
    + ***Deprecation:*** public: void Euler(T, T, T)
    + ***Replacement:*** public: void SetFromEuler(T, T, T)
    + ***Deprecation:*** public: void ToAxis(Vector3<T> &, T &) const
    + ***Replacement:*** public: void void AxisAngle(Vector3<T> &, T &) const
    + ***Deprecation:*** public: void Matrix(const Matrix3<T> &)
    + ***Replacement:*** public: void SetFromMatrix(const Matrix3<T> &)
    + ***Deprecation:*** public: void From2Axes(const Vector3<T> &, const Vector3<T> &)
    + ***Replacement:*** public: void SetFrom2Axes(const Vector3<T> &, const Vector3<T> &)
    + ***Deprecation:*** public: void X(T)
    + ***Replacement:*** public: void SetX(T)
    + ***Deprecation:*** public: void Y(T)
    + ***Replacement:*** public: void SetY(T)
    + ***Deprecation:*** public: void Z(T)
    + ***Replacement:*** public: void SetZ(T)
    + ***Deprecation:*** public: void W(T)
    + ***Replacement:*** public: void SetW(T)

1. The `ignition` namespace is deprecated and will be removed in future versions.  Use `gz` instead.
1. Header files under `ignition/...` are deprecated and will be removed in future versions.
   Use `gz/...` instead.
1. The following `IGN_` prefixed macros are deprecated and will be removed in future versions.
   Additionally, they will only be available when including the corresponding `ignition/...` header.
   Use the `GZ_` prefix instead.
   1. `IGN_RTOD`, `IGN_DTOR`
   1. `IGN_NORMALIZE`
   1. `IGN_PI`, `IGN_PI_2`, `IGN_PI_4`
   1. `IGN_SQRT2`
   1. `IGN_FP_VOLATILE`
   1. `IGN_SPHERE_VOLUME`, `IGN_CYLINDER_VOLUME`, `IGN_BOX_VOLUME`, `IGN_BOX_VOLUME_V`
   1. `IGN_MASSMATRIX3_DEFAULT_TOLERANCE`
1. All `IGN_*_SIZE_T` variables are deprecated and will be removed in future versions.
    Please use `GZ_*_SIZE_T` instead.


### Modifications

1. The out stream operator is guaranteed to return always plain 0 and not to
   return -0, 0.0 or other instances of zero value.

## Gazebo Math 6.9.2 to 6.10.0

1. **Color::HSV()**: A bug related to the hue output of this function was fixed.

## Gazebo Math 6.8 to 6.9

1. **SphericalCoordinates**: A bug related to the LOCAL frame was fixed. To
   preserve behaviour, the `LOCAL` frame was left with the bug, and a new
   `LOCAL2` frame was introduced, which can be used to get the correct
   calculations.

## Gazebo Math 4.X to 5.X

### Additions

1. **MassMatrix.hh**
    + Epsilon(const T), Epsilon(const Vector3<T>, const T)
      return relative tolerance proportional to machine
      precision and largest possible moment of inertia.
    * IsNearPositive(const T) is similar to IsPositive(const T)
      but it checks for positive semidefinite inertia
      using >= instead of >.

1. **Plane.hh**
    + Added copy constructor.

1. **Pose3.hh**
    + Added `*=` operator.

### Breaking Changes

1. The `Box` class has been changed to a templatized class that is not
   axis-aligned. The previous `Box` functionality is now in the
   `AxisAlignedBox` class.

1. The behavior of `Pose3` multiplication operator `*` has been changed to
   match behavior of Matrix and Quaternion multiplication.

### Modifications

1. **Inertial.hh**
    + SetMassMatrix now accepts a relative tolerance parameter.
1. **MassMatrix.hh**
    + IsPositive, IsValid, ValidMoments now accept a relative tolerance
      parameter based on Epsilon function.
    + IsValid now uses IsNearPositive instead of IsPositive
    + ValidMoments now uses >= in comparisons instead of >

### Deprecations

1. **MassMatrix3.hh**
    + All mutator functions that lacked a `Set` prefix have been deprecated
    and replaced by version with a `Set` prefix.
    + The MOI functions have been renamed to Moi.
1. **Inertial.hh**
    + The MOI functions have been renamed to Moi.

## Gazebo Math 3.X to 4.X

### Added dependencies

1. **gz-cmake**
    + gz-math now has a build dependency on gz-cmake, which
      allows cmake scripts to be shared across all the Gazebo packages.

### Modifications

1. **Box.hh**
    + Boxes generated with the default constructor do not intersect any other
    boxes or contain any points (previously they contained the origin).

1. **Helpers.hh**
    + parseInt and parseFloat functions now use std::stoi and std::stod,
      so parsing an alphanumeric string that starts with numbers
      no longer returns a NaN, but instead the beginning of the string
      is parsed (e.g. ("23ab67" -> 23) now, but ("ab23ab67" -> NaN) still).

1. **SemanticVersion.hh**
    + The SemanticVersion(const std::string &) constructor is now explicit.

1. **All Headers**
    + All headers now have an inline versioned namespace. Code should be
    unchanged except all forward declarations of math types must be replaced
    with an include of the header for that type.

### Deprecations

1. **Matrix4.hh**
    + ***Deprecation:*** public: void Translate(const Vector3<T> &_t)
    + ***Replacement:*** public: void SetTranslation(const Vector3<T> &_t)

    + ***Deprecation:*** public: void Translate(T _x, T _y, T _z)
    + ***Replacement:*** public: void SetTranslation(T _x, T _y, T _z)

## Gazebo Math 2.X to 3.X

### Modifications

1. **RotationSpline.hh**
    + The `UpdatePoint` function now returns a boolean value.

1. **Spline.hh**
    + The `UpdatePoint` function now returns a boolean value.

### Deprecations

1. **Matrix4.hh**
    + ***Deprecation:*** public: Vector3<T> TransformAffine(const Vector3<T>
        &_v) const
    + ***Replacement:*** public: bool TransformAffine(const Vector3<T>
        &_v,Vector3<T> &_result) const

1. **Helpers.hh**
    + ***Deprecation:*** IGN_DBL_MAX
    + ***Replacement:*** gz::math::MAX_D

    + ***Deprecation:*** IGN_DBL_MIN
    + ***Replacement:*** gz::math::MIN_D

    + ***Deprecation:*** IGN_DBL_LOW
    + ***Replacement:*** gz::math::LOW_D

    + ***Deprecation:*** IGN_DBL_INF
    + ***Replacement:*** gz::math::INF_D

    + ***Deprecation:*** IGN_FLT_MAX
    + ***Replacement:*** gz::math::MAX_F

    + ***Deprecation:*** IGN_FLT_MIN
    + ***Replacement:*** gz::math::MIN_F

    + ***Deprecation:*** IGN_FLT_LOW
    + ***Replacement:*** gz::math::LOW_F

    + ***Deprecation:*** IGN_FLT_INF
    + ***Replacement:*** gz::math::INF_F

    + ***Deprecation:*** IGN_UI16_MAX
    + ***Replacement:*** gz::math::MAX_UI16

    + ***Deprecation:*** IGN_UI16_MIN
    + ***Replacement:*** gz::math::MIN_UI16

    + ***Deprecation:*** IGN_UI16_LOW
    + ***Replacement:*** gz::math::LOW_UI16

    + ***Deprecation:*** IGN_UI16_INF
    + ***Replacement:*** gz::math::INF_UI16

    + ***Deprecation:*** IGN_I16_MAX
    + ***Replacement:*** gz::math::MAX_I16

    + ***Deprecation:*** IGN_I16_MIN
    + ***Replacement:*** gz::math::MIN_I16

    + ***Deprecation:*** IGN_I16_LOW
    + ***Replacement:*** gz::math::LOW_I16

    + ***Deprecation:*** IGN_I16_INF
    + ***Replacement:*** gz::math::INF_I16

    + ***Deprecation:*** IGN_UI32_MAX
    + ***Replacement:*** gz::math::MAX_UI32

    + ***Deprecation:*** IGN_UI32_MIN
    + ***Replacement:*** gz::math::MIN_UI32

    + ***Deprecation:*** IGN_UI32_LOW
    + ***Replacement:*** gz::math::LOW_UI32

    + ***Deprecation:*** IGN_UI32_INF
    + ***Replacement:*** gz::math::INF_UI32

    + ***Deprecation:*** IGN_I32_MAX
    + ***Replacement:*** gz::math::MAX_I32

    + ***Deprecation:*** IGN_I32_MIN
    + ***Replacement:*** gz::math::MIN_I32

    + ***Deprecation:*** IGN_I32_LOW
    + ***Replacement:*** gz::math::LOW_I32

    + ***Deprecation:*** IGN_I32_INF
    + ***Replacement:*** gz::math::INF_I32

    + ***Deprecation:*** IGN_UI64_MAX
    + ***Replacement:*** gz::math::MAX_UI64

    + ***Deprecation:*** IGN_UI64_MIN
    + ***Replacement:*** gz::math::MIN_UI64

    + ***Deprecation:*** IGN_UI64_LOW
    + ***Replacement:*** gz::math::LOW_UI64

    + ***Deprecation:*** IGN_UI64_INF
    + ***Replacement:*** gz::math::INF_UI64

    + ***Deprecation:*** IGN_I64_MAX
    + ***Replacement:*** gz::math::MAX_I64

    + ***Deprecation:*** IGN_I64_MIN
    + ***Replacement:*** gz::math::MIN_I64

    + ***Deprecation:*** IGN_I64_LOW
    + ***Replacement:*** gz::math::LOW_I64

    + ***Deprecation:*** IGN_I64_INF
    + ***Replacement:*** gz::math::INF_I64
