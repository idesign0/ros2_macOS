from pathlib import Path
import unittest

from ament_index_python import get_package_share_directory
from autoware_lanelet2_extension_python.projection import MGRSProjector
import lanelet2
import yaml


class MGRSProjectorInitTest(unittest.TestCase):
    def test_mgrs_initialization(self) -> None:
        m = MGRSProjector(lanelet2.io.Origin(0, 0))
        self.assertFalse(m.isMGRSCodeSet())

    def test_mgrs_set(self) -> None:
        m = MGRSProjector()
        m.setMGRSCode("53SQA")
        # now set
        self.assertTrue(m.isMGRSCodeSet())


class MGRSProjectorProjectionCalculationTest(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        self.map_path = (
            Path(get_package_share_directory("autoware_lanelet2_extension_python"))
            / "test_map/mgrs/lanelet2_map.osm"
        )
        projector_path = (
            Path(get_package_share_directory("autoware_lanelet2_extension_python"))
            / "test_map/mgrs/map_projector_info.yaml"
        )
        with open(projector_path, encoding="utf-8") as f:
            map_projector_info = yaml.safe_load(f)
            self.mgrs_grid = map_projector_info["mgrs_grid"]

        self.projector = MGRSProjector()
        self.lanelet_map = lanelet2.io.load(str(self.map_path), self.projector)

    def test_projected_mgrs(self) -> None:
        for point in self.lanelet_map.pointLayer:
            self.projector.reverse(lanelet2.core.BasicPoint3d(point.x, point.y, point.z))
            break

        self.assertTrue(self.projector.getProjectedMGRSGrid() == self.mgrs_grid)

        self.projector.setMGRSCode(self.projector.getProjectedMGRSGrid())
        self.lanelet_map = lanelet2.io.load(str(self.map_path), self.projector)

        point_1 = self.lanelet_map.pointLayer.get(1)
        """x, y

        99.99997007392813 != 100.0 within # places (2.9926071874797344e-05 difference)
        100.00005354359746 != 100.0 within # places (5.354359745979309e-05 difference)

        """
        self.assertAlmostEqual(point_1.x, 100.0, delta=1e-4)
        self.assertAlmostEqual(point_1.y, 100.0, delta=1e-4)
        self.assertAlmostEqual(point_1.z, 100.0, delta=1e-4)


if __name__ == "__main__":
    unittest.main()
