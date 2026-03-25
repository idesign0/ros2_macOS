from autoware_lanelet2_extension_python.projection import MGRSProjector
import autoware_lanelet2_extension_python.utility.query as query
import autoware_lanelet2_extension_python.utility.utilities as utilities
from geometry_msgs.msg import Point
from geometry_msgs.msg import Pose
import lanelet2
import lanelet2.geometry
import matplotlib.pyplot as plt
import numpy as np


def test_projection():
    return MGRSProjector(lanelet2.io.Origin(0.0, 0.0))


def test_io(map_path, projection):
    return lanelet2.io.load(map_path, projection)


def plot_ll2_id(ll2, ax, text):
    xs, ys = np.array([pt.x for pt in ll2.centerline]), np.array([pt.y for pt in ll2.centerline])
    x, y = np.average(xs), np.average(ys)
    ax.text(x, y, text)


def plot_linestring(linestring, ax, color, linestyle, label, **kwargs):
    xs = [pt.x for pt in linestring]
    ys = [pt.y for pt in linestring]
    ax.plot(xs, ys, color=color, linestyle=linestyle, label=label, **kwargs)


def test_utility_utilities(lanelet_map, routing_graph):
    lanelet108 = lanelet_map.laneletLayer.get(108)
    lanelet108_next = query.getSucceedingLaneletSequences(routing_graph, lanelet108, 100.0)
    lanelet108_seq = [lanelet108, *lanelet108_next[0]]
    lanelet108_seq_combined = utilities.combineLaneletsShape(lanelet108_seq)
    lanelet108_seq_combined_fine_centerline = utilities.generateFineCenterline(
        lanelet108_seq_combined, 1.0
    )
    lanelet108_seq_combined_right_offset = utilities.getRightBoundWithOffset(
        lanelet108_seq_combined, 1.0, 1.0
    )
    lanelet108_seq_combined_left_offset = utilities.getLeftBoundWithOffset(
        lanelet108_seq_combined, 1.0, 1.0
    )
    fig = plt.figure()
    ax = fig.add_subplot(1, 3, 1)
    ax.axis("equal")
    plot_linestring(lanelet108_seq_combined.leftBound, ax, "orange", "-", "108 left")
    plot_linestring(lanelet108_seq_combined.rightBound, ax, "orange", "-", "108 right")
    plot_linestring(lanelet108_seq_combined_fine_centerline, ax, "orange", "--", "108 center")
    plot_linestring(lanelet108_seq_combined_right_offset, ax, "cyan", "--", "108 right offset")
    plot_linestring(lanelet108_seq_combined_left_offset, ax, "red", "--", "108 left offset")
    [plot_ll2_id(ll2, ax, str(ll2.id)) for ll2 in lanelet108_seq]
    ax.set_title("test of combineLaneletsShape ~ getLeftBoundWithOffset")

    expanded_lanelet108 = utilities.getExpandedLanelet(lanelet108, 1.0, -1.0)
    expanded_lanelet108_seq = utilities.getExpandedLanelets(lanelet108_seq, 2.0, -2.0)
    ax = fig.add_subplot(1, 3, 2)
    ax.axis("equal")
    plot_linestring(expanded_lanelet108.leftBound, ax, "orange", "-", "expanded 108 left(1.0)")
    plot_linestring(expanded_lanelet108.rightBound, ax, "orange", "-", "expanded 108 right(-1.0)")
    plot_linestring(expanded_lanelet108.centerline, ax, "orange", "--", "expanded 108 center")
    [
        (
            plot_linestring(ll2.leftBound, ax, "orange", "-", None),
            plot_linestring(ll2.rightBound, ax, "orange", "-", None),
        )
        for ll2 in expanded_lanelet108_seq
    ]
    [plot_ll2_id(ll2, ax, f"{ll2.id}(2.0)") for ll2 in expanded_lanelet108_seq]
    ax.set_title("test of getExpandedLanelet(s)")

    print(f"""{utilities.getLaneletLength2d(lanelet108)=}""")
    print(f"""{utilities.getLaneletLength2d(lanelet108_seq)=}""")
    print(f"""{utilities.getLaneletLength3d(lanelet108)=}""")
    print(f"""{utilities.getLaneletLength3d(lanelet108_seq)=}""")

    search_pose = Pose(position=Point(x=3685.0, y=73750.0))
    arc_coords = utilities.getArcCoordinates(lanelet108_seq, search_pose)
    print(
        f"""utilities.getArcCoordinates(lanelet108_seq, search_pose) = (length: {arc_coords.length}, distance: {arc_coords.distance})"""
    )
    closest_lanelet = query.getClosestLanelet(lanelet108_seq, search_pose)
    assert closest_lanelet.id == 156
    closest_segment = utilities.getClosestSegment(
        lanelet2.core.BasicPoint2d(x=search_pose.position.x, y=search_pose.position.y),
        closest_lanelet.centerline,
    )
    ax = fig.add_subplot(1, 3, 3)
    ax.axis("equal")
    plot_linestring(closest_lanelet.leftBound, ax, "orange", "--", "156 left")
    plot_linestring(closest_lanelet.rightBound, ax, "orange", "--", "156 right")
    plot_linestring(closest_lanelet.centerline, ax, "cyan", "--", "156 center")
    plot_linestring(closest_segment, ax, "red", "-", "closest segment", linewidth=2)
    ax.scatter([search_pose.position.x], [search_pose.position.y], marker="o", label="search_pose")

    print(f"""{utilities.getLaneletAngle(closest_lanelet, search_pose.position)=}""")
    print(f"""{utilities.isInLanelet(search_pose, closest_lanelet)=}""")
    print(f"""{utilities.isInLanelet(search_pose, closest_lanelet, 5.0)=}""")
    closest_center_pose = utilities.getClosestCenterPose(closest_lanelet, search_pose.position)
    ax.scatter(
        [closest_center_pose.position.x],
        [closest_center_pose.position.y],
        marker="o",
        label="closest_center_pose",
    )
    print(f"""{utilities.getLateralDistanceToCenterline(closest_lanelet, search_pose)=}""")
    print(f"""{utilities.getLateralDistanceToClosestLanelet(lanelet108_seq, search_pose)=}""")
    plt.legend()
    plt.show()


def test_utility_query(lanelet_map, routing_graph):
    lanelets = query.laneletLayer(lanelet_map)
    lanelet56 = lanelet_map.laneletLayer.get(56)
    lanelet108 = lanelet_map.laneletLayer.get(108)
    print(f"""{len(query.subtypeLanelets(lanelets, "road"))=}""")
    print(f"""{len(query.subtypeLanelets(lanelets, "road"))=}""")
    print(f"""{len(query.crosswalkLanelets(lanelets))=}""")
    print(f"""{len(query.walkwayLanelets(lanelets))=}""")
    print(f"""{len(query.roadLanelets(lanelets))=}""")
    print(f"""{len(query.shoulderLanelets(lanelets))=}""")
    print(f"""{len(query.trafficLights(lanelets))=}""")
    print(f"""{len(query.autowareTrafficLights(lanelets))=}""")
    print(f"""{len(query.detectionAreas(lanelets))=}""")
    print(f"""{len(query.noStoppingAreas(lanelets))=}""")
    print(f"""{len(query.noParkingAreas(lanelets))=}""")
    print(f"""{len(query.speedBumps(lanelets))=}""")
    print(f"""{len(query.crosswalks(lanelets))=}""")
    print(f"""{len(query.curbstones(lanelet_map))=}""")
    print(f"""{len(query.getAllPolygonsByType(lanelet_map, "parking_lot"))=}""")
    print(f"""{len(query.getAllParkingLots(lanelet_map))=}""")
    print(f"""{len(query.getAllPartitions(lanelet_map))=}""")
    print(f"""{len(query.getAllParkingSpaces(lanelet_map))=}""")
    print(f"""{len(query.getAllFences(lanelet_map))=}""")
    print(f"""{len(query.getAllPedestrianPolygonMarkings(lanelet_map))=}""")
    print(f"""{len(query.getAllPedestrianLineMarkings(lanelet_map))=}""")
    print(f"""{len(query.stopLinesLanelets(lanelets))=}""")
    print(f"""{len(query.stopLinesLanelet(lanelet108))=}""")
    print(f"""{len(query.stopSignStopLines(lanelets))=}""")
    lanelet56_centerline_center = np.sum([p.basicPoint() for p in lanelet56.centerline]) * (
        1.0 / len(lanelet56.centerline)
    )
    search_point = Point(
        x=lanelet56_centerline_center.x,
        y=lanelet56_centerline_center.y,
        z=lanelet56_centerline_center.z,
    )
    print(f"""{search_point=}""")
    print(f"""{[ll2.id for ll2 in query.getLaneletsWithinRange(lanelets, search_point, 15.0)]=}""")
    search_point_2d = lanelet2.core.BasicPoint2d(
        x=lanelet56_centerline_center.x, y=lanelet56_centerline_center.y
    )
    print(
        f"""{[ll2.id for ll2 in query.getLaneletsWithinRange(lanelets, search_point_2d, 15.0)]=}"""
    )
    print(f"""{[ll2.id for ll2 in query.getLaneChangeableNeighbors(routing_graph, lanelet108)]=}""")
    print(
        f"""{[ll2.id for ll2 in query.getLaneChangeableNeighbors(routing_graph, query.roadLanelets(lanelets), search_point)]=}"""
    )
    print(f"""{[ll2.id for ll2 in query.getAllNeighbors(routing_graph, lanelet56)]=}""")
    print(
        f"""{[ll2.id for ll2 in query.getAllNeighbors(routing_graph, query.roadLanelets(lanelets), search_point)]=}"""
    )
    print(f"""{[ll2.id for ll2 in query.getAllNeighborsLeft(routing_graph, lanelet56)]=}""")
    print(f"""{[ll2.id for ll2 in query.getAllNeighborsRight(routing_graph, lanelet56)]=}""")
    search_pose = Pose()
    search_pose.position = search_point
    print(
        f"""{(query.getClosestLanelet(lanelets, search_pose).id if query.getClosestLanelet(lanelets, search_pose) else None)=}"""
    )
    print(f"""{[ll2.id for ll2 in query.getCurrentLanelets(lanelets, search_point)]=}""")
    print(
        f"""{[[ll2.id for ll2 in ll2s] for ll2s in query.getSucceedingLaneletSequences(routing_graph, lanelet108, 100.0)]=}"""
    )
    print(
        f"""{[[ll2.id for ll2 in ll2s] for ll2s in query.getPrecedingLaneletSequences(routing_graph, lanelet108, 100.0)]=}"""
    )


if __name__ == "__main__":
    proj = test_projection()
    # https://docs.google.com/uc?export=download&id=1499_nsbUbIeturZaDj7jhUownh5fvXHd.
    # See https://autowarefoundation.github.io/autoware-documentation/main/tutorials/ad-hoc-simulation/planning-simulation/
    lanelet_map = test_io("<path to lanelet2_map.osm>", proj)

    traffic_rules = lanelet2.traffic_rules.create(
        lanelet2.traffic_rules.Locations.Germany,
        lanelet2.traffic_rules.Participants.Vehicle,
    )
    routing_graph = lanelet2.routing.RoutingGraph(lanelet_map, traffic_rules)

    test_utility_utilities(lanelet_map, routing_graph)
    test_utility_query(lanelet_map, routing_graph)
