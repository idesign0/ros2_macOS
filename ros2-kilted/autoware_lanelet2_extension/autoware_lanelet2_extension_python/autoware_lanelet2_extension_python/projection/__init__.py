import lanelet2  # noqa: F401 # isort: skip
import autoware_lanelet2_extension_python._autoware_lanelet2_extension_python_boost_python_projection as _projection_cpp

MGRSProjector = _projection_cpp.MGRSProjector
MGRSProjector.__doc__ = """
    MGRSProjector(origin: lanelet2.io.Origin)
    Example:
        origin = lanelet2.io.Origin(lat, lon, alt)
        projector = MGRSProjector(origin)
    """
TransverseMercatorProjector = _projection_cpp.TransverseMercatorProjector
TransverseMercatorProjector.__doc__ = """
    TransverseMercatorProjector(origin: lanelet2.io.Origin)
    Example:
        origin = lanelet2.io.Origin(lat, lon, alt)
        projector = TransverseMercatorProjector(origin)
    """
