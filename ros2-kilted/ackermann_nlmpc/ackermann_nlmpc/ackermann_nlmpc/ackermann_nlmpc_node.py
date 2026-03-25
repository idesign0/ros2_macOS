# Copyright (C) 2025, Georg Schildbach, Jasper Pflughaupt
# --------------------------------------------------------------------------------------------------
# This program is free software: you can redistribute it and/or modify it under the terms of the 
# GNU General Public License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
# This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
# You should have received a copy of the GNU General Public License along with this program. If
# not, see <http://www.gnu.org/licenses/>.
# --------------------------------------------------------------------------------------------------
# Your feedback or questions or bug reports are highly welcome! Contact me if you are interested in
# using this product commercially. Please address all correspondance to: gschildbach(at)gmail.com
# --------------------------------------------------------------------------------------------------

import ctypes
import math
import os
import time
from threading import Lock
from transforms3d.euler import euler2quat
from ament_index_python.packages import get_package_share_directory

import rclpy
from rclpy.node import Node
from rclpy.time import Time, Duration
from rclpy.executors import MultiThreadedExecutor

from ackermann_msgs.msg import AckermannDriveStamped
from geometry_msgs.msg import Quaternion, PoseStamped, PolygonStamped, Point32, TransformStamped
from nav_msgs.msg import Path, Odometry
from std_msgs.msg import Float32
from ackermann_nlmpc_msgs.msg import Trajectory

from tf2_ros import Buffer, TransformListener, TransformBroadcaster

from .utils import pose2yaw, transform_2d_pose


class UnloadableCDLL:
    """
    Wrapper for ctypes.CDLL that unloads the library when deleted.
    
    Fixes issues where static valiables are not fully cleared on reload. Adapted from https://stackoverflow.com/a/64483246.
    """

    def __init__(self, lib_path):
        self._dlclose_func = ctypes.cdll.LoadLibrary('').dlclose
        self._dlclose_func.argtypes = [ctypes.c_void_p]
        self._ctypes_lib = ctypes.cdll.LoadLibrary(lib_path)
        self._handle = self._ctypes_lib._handle

    def __getattr__(self, attr):
        return self._ctypes_lib.__getattr__(attr)

    def __del__(self):
        del self._ctypes_lib
        self._dlclose_func(self._handle)


class NLMPCNode(Node):
    """
    ROS2 python wrapper for the Ackermann NLMPC library.

    This node subscribes to trajectory, odometry, and steering angle messages,
    executes the NLMPC algorithm, and publishes control commands and predicted trajectories.
    """

    def __init__(self):
        super().__init__('ackermann_nlmpc_node')

        self.traj : list[float] | None = None                       # The currently active trajectory in a custom format, None if no trajectory is set
        self.odom : Odometry | None = None                          # The latest odometry message
        self.steering_angle : float | None = None                   # The latest steering angle message
        self.steering_angle_msg_time = self.get_clock().now()       # The time when the latest steering angle message was received

        self._state : list[float | None] = [None] * 5               # The current state of the vehicle (position x and y [m], heading [rad], velocity [m/s], steering angle [rad])
        self._control : list[float] = [0.0, 0.0]                    # The current control command (acceleration [m/s^2], steering rate [rad/s])

        self._traj_lock = Lock()

        # Library path
        self.declare_parameter('lib_path', 'codegen/nlmpc.so')

        # Coordinate frames
        self.declare_parameter('base_link_frame', 'base_link')
        self.declare_parameter('odom_frame', 'odom')

        # Fundamental parameters
        self.declare_parameter('dt', 0.1)
        self.declare_parameter('max_nodes', 1000)
        self.declare_parameter('horizon', 40)
        self.declare_parameter('max_message_age', 0.5)
        self.declare_parameter('publish_predicted_trajectory', True)
        self.declare_parameter('publish_reference_trajectory', True)

        # Tuning parameters
        self.declare_parameter('weight_acceleration', 0.5)
        self.declare_parameter('weight_steering_rate', 0.5)
        self.declare_parameter('weight_lateral_offset', 10.0)
        self.declare_parameter('weight_longitudinal_offset', 10.0)
        self.declare_parameter('weight_heading', 100.0)
        self.declare_parameter('weight_velocity', 0.1)
        self.declare_parameter('weight_steering_angle', 0.1)
        self.declare_parameter('contolerance', 0.1)
        self.declare_parameter('conpenalty', 1e3)

        # Input bounds
        self.declare_parameter('acceleration_min', -5.0)
        self.declare_parameter('acceleration_max', 2.5)
        self.declare_parameter('jerk_min', -50.0)
        self.declare_parameter('jerk_max', 50.0)
        self.declare_parameter('steering_rate_limit', 0.2167)
        self.declare_parameter('steering_rate_change_limit', 5.0)

        # Subscribers
        self.create_subscription(Trajectory, 'trajectory', self._cb_traj, 10)
        self.create_subscription(Odometry, 'odom', self._cb_odom, 10)
        self.create_subscription(Float32, 'steering_angle', self._cb_steering_angle, 10)

        # Publishers
        self._control_pub = self.create_publisher(AckermannDriveStamped, 'control', 10)
        self._pred_pub = self.create_publisher(Path, 'predicted_trajectory', 10)
        self._traj_pub = self.create_publisher(Path, 'reference_trajectory', 10)
        self._constraints_pub = self.create_publisher(PolygonStamped, 'trajectory_constraints', 10)

        # TF2
        self._tf_buffer = Buffer()
        self._tf_listener = TransformListener(self._tf_buffer, self)
        self._traj_frame_broadcaster = TransformBroadcaster(self)

        # Timers
        self.create_timer(self.get_parameter('dt').get_parameter_value().double_value, self._update_mpc)

        self.get_logger().info('NLMPC Node has been initialized.')

    def destroy_node(self):
        """Override the destroy_node method to ensure the MPC library is unloaded."""
        if hasattr(self, '_lib'):
            del self._lib
        super().destroy_node()


    def _cb_odom(self, msg: Odometry):
        self.odom = msg

    def _cb_steering_angle(self, msg: Float32):
        self.steering_angle = msg.data
        self.steering_angle_msg_time = self.get_clock().now()

    def _cb_traj(self, msg: Trajectory):
        self.get_logger().debug('Received trajectory data.')
        if msg.nodes is None or len(msg.nodes) == 0:
            self.get_logger().error('Trajectory does not contain any nodes.')
            return

        # Initialize or update the reference trajectory if a new trajectory is received
        if self.traj is None or Time.from_msg(msg.header.stamp) > self.traj_start_time:
            self._setup_trajectory(msg)
        else:
            self.get_logger().info('Received trajectory data, but it is older than the current trajectory. Ignoring it.')

    def _setup_trajectory(self, traj: Trajectory):
        if not self._check_sensors():
            self.get_logger().error('Trajectory setup failed due to missing or outdated sensor data.')
            return

        # use lock to prevent simultaneous update and apply of trajectory
        with self._traj_lock:

            num_nodes = len(traj.nodes)
            num_nodes = min(num_nodes, self.get_parameter('max_nodes').get_parameter_value().integer_value)
            odom_frame = self.get_parameter('odom_frame').get_parameter_value().string_value
            self.traj_start_time = Time.from_msg(traj.header.stamp)
            self.traj_start_time_s = self.traj_start_time.nanoseconds * 1e-9

            # Convert trajectory origin pose to odometry frame if necessary
            if traj.header.frame_id != odom_frame:
                x0, y0, heading0 = transform_2d_pose(
                    traj.x0,
                    traj.y0,
                    traj.heading0,
                    self._tf_buffer.lookup_transform(
                        source_frame=traj.header.frame_id,
                        target_frame=odom_frame,
                        time=Time()  # get latest
                    )
                )
            else:
                x0 = traj.x0
                y0 = traj.y0
                heading0 = traj.heading0

            # The header of the data provided to the MPC code (self.traj) contains the trajectory origin in the odom frame.
            # The nodes are stored relative to this origin ("ref_trajectory" frame). We compute and publish a transform from
            # the odom frame to the "ref_trajectory" frame here.
            self._odom2traj_transform = TransformStamped()
            self._odom2traj_transform.header.stamp = traj.header.stamp
            self._odom2traj_transform.header.frame_id = odom_frame
            self._odom2traj_transform.child_frame_id = "ref_trajectory"
            self._odom2traj_transform.transform.translation.x = x0
            self._odom2traj_transform.transform.translation.y = y0
            self._odom2traj_transform.transform.translation.z = 0.0
            q = euler2quat(0, 0, heading0)
            self._odom2traj_transform.transform.rotation = Quaternion(
                x=q[1],
                y=q[2],
                z=q[3],
                w=q[0]
            )
            # Publish the transformation
            self._traj_frame_broadcaster.sendTransform(self._odom2traj_transform)

            # Set up data structure for the MPC library
            # Make trajectory header
            self.traj = [
                self.traj_start_time_s, # timestamp of trajectory start
                x0,                     # x coordinate of trajectory origin
                y0,                     # y coordinate of trajectory origin
                heading0,               # heading of trajectory origin
                traj.type,              # type: 0: trajectory, 1: path, 2: circular path
                num_nodes               # number of nodes
            ]

            # Make trajectory nodes
            transforms = {}
            for i, node in enumerate(traj.nodes):
                # Node timestamp is relative to the trajectory start time from header
                t = (Time.from_msg(node.header.stamp) - self.traj_start_time).nanoseconds * 1e-9
                # Check timestamp
                if t < 0:
                    self.get_logger().error(f"Node {i} has a negative timestamp: {t}.")
                    self.traj = None
                    return
                elif i > 0 and t < self.traj[-11]:
                    self.get_logger().error(f"Node {i} has a timestamp smaller than the previous node: {t} < {self.traj[-11]}.")
                    self.traj = None
                    return

                # Convert node pose to ref_trajectory frame
                if node.header.frame_id != "ref_trajectory":
                    if node.header.frame_id not in transforms:
                        while not self._tf_buffer.can_transform(
                            source_frame=node.header.frame_id,
                            target_frame="ref_trajectory",
                            time=Time(),
                            timeout=Duration(seconds=1.0)
                        ):
                            self.get_logger().info(f'Waiting for TF transform {node.header.frame_id} -> ref_trajectory to become available...')
                            time.sleep(0.1)
                        transforms[node.header.frame_id] = self._tf_buffer.lookup_transform(
                            source_frame=node.header.frame_id,
                            target_frame="ref_trajectory",
                            time=Time()  # get latest
                        )
                    x, y, heading = transform_2d_pose(
                        node.x,
                        node.y,
                        node.heading,
                        transforms[node.header.frame_id]
                    )
                else:
                    x = node.x
                    y = node.y
                    heading = node.heading

                # Avoid numerical issues that might appear if the first node is exactly zero
                if i == 0:
                    if x == 0.0 and y == 0.0 and heading == 0.0:
                        x += 1e-3

                # Append node data to the trajectory
                self.traj.extend([
                    t,              # timestamp
                    x,              # x
                    y,              # y
                    heading,        # heading
                    node.v,         # velocity
                    node.a,         # acceleration
                    node.delta,     # steering angle
                    node.beta,      # side slip angle
                    node.mode,      # mode (0: standstill, 1: forward, 2: backward)
                    node.pw_l,      # path width left
                    node.pw_r       # path width right
                ])

                # Stop if maximum number of nodes is reached
                if i >= num_nodes - 1:
                    break
            
            # self.get_logger().info(f"Trajectory header: {self.traj[:6]}")
            # for i in range(num_nodes):
            #     j = 6 + 11 * i
            #     self.get_logger().info(f"Node {i}: t={self.traj[j]:.2f}, x={self.traj[j+1]:.2f}, y={self.traj[j+2]:.2f}, heading={self.traj[j+3]:.2f}, v={self.traj[j+4]:.2f}, a={self.traj[j+5]:.2f}, delta={self.traj[j+6]:.2f}, beta={self.traj[j+7]:.2f}, mode={self.traj[j+8]}, pw_l={self.traj[j+9]:.2f}, pw_r={self.traj[j+10]:.2f}")

            # Check trajectory length
            if len(self.traj) != 6 + 11 * num_nodes:
                self.get_logger().error(f"Trajectory length mismatch: {len(self.traj)} != {6 + 11 * num_nodes}.")
                self.traj = None
                return

            # Initialize MPC library
            self._init_mpc()

            # Publish the reference trajectory and constraints
            if self.get_parameter('publish_reference_trajectory').get_parameter_value().bool_value:
                self._publish_reference_trajectory()

            self.get_logger().info(f"Trajectory setup completed.")

    def _init_mpc(self):
        """Load the MPC library, initialize function pointer and setup MPC state memory."""
        # Unload the library for proper reset
        if hasattr(self, '_lib'):
            del self._lib

        # Get library path from parameter, prepend package path if relative
        lib_path = self.get_parameter('lib_path').get_parameter_value().string_value
        if not lib_path.startswith('/'):
            package_path = get_package_share_directory('ackermann_nlmpc')
            lib_path = os.path.join(package_path, lib_path)
        self.get_logger().debug(f'Library path: {lib_path}')
        path, ext = os.path.splitext(lib_path)
        c_path = path + '.c'
        so_path = path + '.so'
        so_fallback_path = os.path.join(os.path.expanduser('~'), '.ros', 'ackermann_nlmpc', 'codegen', f'{os.path.basename(path)}.so')
        # If path parameter ends with .c, always compile.
        if ext == '.c':
            self.get_logger().debug('Compiling C code to .so file...')
            # Check if path is writable
            if os.access(os.path.dirname(path), os.W_OK):
                out_path = so_path
            else:
                self.get_logger().warning(f'Library path {os.path.dirname(path)} is not writable, falling back to ~/.ros/ackermann_nlmpc/')
                os.makedirs(os.path.dirname(so_fallback_path), exist_ok=True)
                out_path = so_fallback_path
            self._compile_lib(c_path, out_path)
            load_path = out_path
        # If path parameter ends with .so, just load it.
        elif ext == '.so':
            if os.path.exists(so_path):
                load_path = so_path
            else:
                self.get_logger().error(f'File {so_path} does not exist.')
                raise FileNotFoundError(f'File {so_path} does not exist.')
        # No file extension, check for .so and .c file
        else:
            self.get_logger().debug('Checking for existing .so file...')
            if not os.path.exists(so_path):
                self.get_logger().debug('No .so file found, checking for .c file...')
                if not os.path.exists(c_path):
                    self.get_logger().error(f'No .c or .so file found at {lib_path}.')
                    raise FileNotFoundError(f'No .c or .so file found at {lib_path}.')
                # Check if path is writable
                if os.access(os.path.dirname(path), os.W_OK):
                    self._compile_lib(c_path, so_path)
                    load_path = so_path
                else:
                    self.get_logger().warning(f'Library path {os.path.dirname(path)} is not writable, falling back to ~/.ros/ackermann_nlmpc/')
                    os.makedirs(os.path.dirname(so_fallback_path), exist_ok=True)
                    # Check if fallback .so file exists and is up to date
                    if os.path.exists(so_fallback_path):
                        c_mtime = os.path.getmtime(c_path)
                        so_mtime = os.path.getmtime(so_fallback_path)
                        if c_mtime > so_mtime:
                            self.get_logger().debug('C file is newer than existing fallback .so file, recompiling...')
                            self._compile_lib(c_path, so_fallback_path)
                        else:
                            self.get_logger().debug('Fallback .so file is up to date.')
                    else:
                        self._compile_lib(c_path, so_fallback_path)
                    load_path = so_fallback_path
            # Found .so file, check if up to date
            else:
                self.get_logger().debug('Found .so file, checking if it is up to date...')
                if os.path.exists(c_path):
                    c_mtime = os.path.getmtime(c_path)
                    so_mtime = os.path.getmtime(so_path)
                    if c_mtime > so_mtime:
                        if os.access(os.path.dirname(path), os.W_OK):
                            self.get_logger().debug('C file is newer than existing .so file, recompiling...')
                            self._compile_lib(c_path, so_path)
                        else:
                            self.get_logger().warning(f'C file is newer than existing .so file, but library path {os.path.dirname(path)} is not writable. Using existing .so file.')
                    else:
                        self.get_logger().debug('.so file is up to date.')
                else:
                    self.get_logger().debug('No .c file found, using existing .so file.')
                load_path = so_path
        self.get_logger().info(f'Loading MPC library: {load_path}')

        # Load library and setup function pointer
        self._lib = UnloadableCDLL(load_path)
        self._mpc_fun = self._lib.autompc_run
        self._mpc_fun.argtypes = [
            ctypes.POINTER(ctypes.c_double),  # t
            ctypes.POINTER(ctypes.c_double),  # u0
            ctypes.POINTER(ctypes.c_double),  # x0
            ctypes.POINTER(ctypes.c_double),  # R
            ctypes.POINTER(ctypes.c_double),  # Q
            ctypes.POINTER(ctypes.c_double),  # X_con
            ctypes.POINTER(ctypes.c_double),  # U_con
            ctypes.POINTER(ctypes.c_double),  # traj
            ctypes.POINTER(ctypes.c_double),  # mpc state
            ctypes.POINTER(ctypes.c_int8),    # drivmode
            ctypes.POINTER(ctypes.c_double),  # u
            ctypes.POINTER(ctypes.c_double),  # U
            ctypes.POINTER(ctypes.c_double),  # Ref
            ctypes.POINTER(ctypes.c_double)   # X
        ]
        self._mpc_fun.restype = None

        # Initialize MPC memory
        self._mpc_state = (ctypes.c_double * 88)()
        self._control = [0.0, 0.0]

    def _compile_lib(self, in_path: str, out_path: str):
        """Compile the MPC library C code into a .so file."""
        self.get_logger().info(f'Compiling MPC library from {in_path}...')
        compile_cmd = f'gcc -fPIC -shared -o {out_path} {in_path}'
        result = os.system(compile_cmd)
        if result != 0:
            self.get_logger().error(f'Failed to compile MPC library with command: {compile_cmd}, exit code {result}.')
            raise RuntimeError('MPC library compilation failed')
        self.get_logger().info('MPC library compiled successfully.')

    def _publish_reference_trajectory(self):
        """Publish the reference trajectory as a Path message with the path width constraints as a Polygon."""
        if self.traj is None:
            self.get_logger().error('No trajectory set, cannot publish reference trajectory.')
            return

        fid = "ref_trajectory"
        stamp = self.traj_start_time.to_msg()

        path_msg = Path()
        path_msg.header.frame_id = fid
        path_msg.header.stamp = stamp
        path_msg.poses = []

        constraints_msg = PolygonStamped()
        constraints_msg.header.frame_id = fid
        constraints_msg.header.stamp = stamp
        constraints_msg.polygon.points = []

        points_left = []
        points_right = []

        for i in range(int(self.traj[5])):
            j = 6 + 11 * i

            pose = PoseStamped()
            pose.header = path_msg.header
            pose.pose.position.x = self.traj[j + 1]
            pose.pose.position.y = self.traj[j + 2]
            q = euler2quat(0, 0, self.traj[j + 3])
            pose.pose.orientation.w = q[0]
            pose.pose.orientation.x = q[1]
            pose.pose.orientation.y = q[2]
            pose.pose.orientation.z = q[3]
            path_msg.poses.append(pose)

            left_point = Point32()
            left_point.x = self.traj[j + 1] + self.traj[j + 9] * math.cos(self.traj[j + 3] + math.pi / 2)
            left_point.y = self.traj[j + 2] + self.traj[j + 9] * math.sin(self.traj[j + 3] + math.pi / 2)
            points_left.append(left_point)
            
            right_point = Point32()
            right_point.x = self.traj[j + 1] - self.traj[j + 10] * math.cos(self.traj[j + 3] + math.pi / 2)
            right_point.y = self.traj[j + 2] - self.traj[j + 10] * math.sin(self.traj[j + 3] + math.pi / 2)
            points_right.append(right_point)

        constraints_msg.polygon.points = points_left + points_right[::-1]  # Close the polygon by adding right points in reverse order
        self._traj_pub.publish(path_msg)
        self._constraints_pub.publish(constraints_msg)


    def _check_sensors(self) -> bool:
        """
        Check odometry and steering angle data presence and age.

        :return: True if both data are available and not older than max age, False otherwise.
        """
        if self.odom is None:
            self.get_logger().debug('No odometry data available.')
            return False
        if self.steering_angle is None:
            self.get_logger().debug('No steering angle data available.')
            return False
        t = self.get_clock().now()
        max_age = Duration(seconds=self.get_parameter('max_message_age').get_parameter_value().double_value)
        if t - Time.from_msg(self.odom.header.stamp) > max_age:
            self.get_logger().error(f'Odometry data is older than max age ({max_age.nanoseconds * 1e-9} s).')
            return False
        if t - self.steering_angle_msg_time > max_age:
            self.get_logger().error(f'Steering angle data is older than max age ({max_age.nanoseconds * 1e-9} s).')
            return False
        return True

    def _pub_control(self, v: float, a: float, ddelta: float):
        """
        Publish a control command.

        :param v: Velocity [m/s]
        :param a: Acceleration [m/s^2]
        :param ddelta: Steering rate [rad/s]
        """
        control_msg = AckermannDriveStamped()
        control_msg.header.stamp = self.get_clock().now().to_msg()
        control_msg.drive.speed = v
        control_msg.drive.acceleration = a
        control_msg.drive.steering_angle_velocity = ddelta
        self._control_pub.publish(control_msg)

    def _brake(self):
        """Publish maximum braking command."""
        self._pub_control(0.0, 0.0, 0.0)
        self._control = [0.0, 0.0]

    def _update_mpc(self):
        """Update MPC, publish control command and predicted trajectory."""
        time_start = time.time()
        dt = self.get_parameter('dt').get_parameter_value().double_value
        horizon = self.get_parameter('horizon').get_parameter_value().integer_value

        # Check if trajectory is set and sensors are available
        if not self._check_sensors():
            self.get_logger().error('MPC update failed due to missing or outdated sensor data.')
            self._brake()
            return
        if self.traj is None:
            self.get_logger().info("No trajectory set.")
            self._brake()
            return
        
        # use lock to prevent simultaneous update and apply of trajectory
        with self._traj_lock:

            # Update the state
            velocity = self.odom.twist.twist.linear.x
            self._state = [
                self.odom.pose.pose.position.x,
                self.odom.pose.pose.position.y,
                pose2yaw(self.odom.pose.pose),
                abs(velocity),
                self.steering_angle
            ]

            # Prepare input variables
            t = (ctypes.c_double * 1)(self.get_clock().now().nanoseconds * 1e-9)
            u0 = (ctypes.c_double * 2)(*self._control)
            x0 = (ctypes.c_double * 5)(*self._state)
            R = (ctypes.c_double * 2)(
                self.get_parameter('weight_acceleration').get_parameter_value().double_value,
                self.get_parameter('weight_steering_rate').get_parameter_value().double_value
            )
            Q = (ctypes.c_double * 5)(
                self.get_parameter('weight_longitudinal_offset').get_parameter_value().double_value,
                self.get_parameter('weight_lateral_offset').get_parameter_value().double_value,
                self.get_parameter('weight_heading').get_parameter_value().double_value,
                self.get_parameter('weight_velocity').get_parameter_value().double_value,
                self.get_parameter('weight_steering_angle').get_parameter_value().double_value
            )
            X_con = (ctypes.c_double * 2)(
                self.get_parameter('contolerance').get_parameter_value().double_value,
                self.get_parameter('conpenalty').get_parameter_value().double_value,
            )
            U_con = (ctypes.c_double * 8)(
                self.get_parameter('acceleration_min').get_parameter_value().double_value,
                -self.get_parameter('steering_rate_limit').get_parameter_value().double_value,
                self.get_parameter('acceleration_max').get_parameter_value().double_value,
                self.get_parameter('steering_rate_limit').get_parameter_value().double_value,
                self.get_parameter('jerk_min').get_parameter_value().double_value,
                -self.get_parameter('steering_rate_change_limit').get_parameter_value().double_value,
                self.get_parameter('jerk_max').get_parameter_value().double_value,
                self.get_parameter('steering_rate_change_limit').get_parameter_value().double_value
            )
            traj = (ctypes.c_double * len(self.traj))(*self.traj)

            # Prepare output variables
            drivmode = ctypes.c_int8()
            u = (ctypes.c_double * 2)()
            U = (ctypes.c_double * (2 * (1 + horizon)))()
            Ref = (ctypes.c_double * (9 * horizon))()
            X = (ctypes.c_double * (5 * (1 + horizon)))()

            # Call the library function
            self.get_logger().debug(f'Calling MPC function, trajectory time: {t[0]:.2f}, x: {x0[0]:.2f}, y: {x0[1]:.2f}, heading: {x0[2]:.2f}, velocity: {x0[3]:.2f}, steering angle: {x0[4]:.2f}')
            time_start_mpc = time.time()
            self._mpc_fun(
                t,                                          # current time [s]
                u0,                                         # previous control command
                x0,                                         # current state
                R, Q, X_con, U_con,                         # weights and constraints
                traj,                                       # reference trajectory
                self._mpc_state,                            # storage for the library
                drivmode,                                   # drivemode output (0 = standstill, 1 = forward, 2 = reverse)
                u,                                          # control command output
                U,                                          # control command prediction output
                Ref,                                        # reference trajectory output [x, y, heading, v, a, steering_angle, ?, ?, ?] * horizon
                X                                           # state prediction output
            )
            time_end_mpc = time.time()
            self.get_logger().debug(f'MPC output: u = [{u[0]:.2f}, {u[1]:.2f}], drivmode = {drivmode.value}')
            self._control = [u[0], u[1]]
            if any(math.isnan(x) for x in X):
                self.get_logger().error('State prediction contains NaN values.')
                self._brake()
                return

            # Set speed and acceleration based on drivemode and current velocity
            a_max_brake = min(abs(self.get_parameter('acceleration_min').get_parameter_value().double_value), -velocity / dt)
            ddelta_max = min(abs(self.get_parameter('steering_rate_limit').get_parameter_value().double_value), -self.steering_angle / dt)
            if drivmode.value == 0: # standstill
                # Brake to reach standstill
                self._pub_control(0.0, a_max_brake, ddelta_max)
            elif drivmode.value == 1: # forward
                # If currently going backward
                if velocity < -1e-2:  # small threshold to avoid numerical issues
                    # Brake to reach standstill
                    self._pub_control(0.0, a_max_brake, ddelta_max)
                else:
                    self._pub_control(velocity + self._control[0] * dt, self._control[0], self._control[1])
            elif drivmode.value == 2: # backward
                # If currently going forward
                if velocity > 1e-2:  # small threshold to avoid numerical issues
                    # Brake to reach standstill
                    self._pub_control(0.0, a_max_brake, ddelta_max)
                else:
                    self._pub_control(velocity - self._control[0] * dt, -self._control[0], self._control[1])
            else:
                self.get_logger().error(f'Invalid drivemode: {drivmode.value}. Using maximum braking acceleration.')
                self._pub_control(0.0, a_max_brake, ddelta_max)

            time_pred = time.time()

            # Publish odom -> ref_trajectory transform
            if self._odom2traj_transform is not None:
                self._traj_frame_broadcaster.sendTransform(self._odom2traj_transform)

            # Publish predicted trajectory
            now = self.get_clock().now()
            base_link_frame = self.get_parameter('odom_frame').get_parameter_value().string_value
            if self.get_parameter('publish_predicted_trajectory').get_parameter_value().bool_value:
                pred_msg = Path()
                pred_msg.header.stamp = now.to_msg()
                pred_msg.header.frame_id = base_link_frame
                pred_msg.poses = []
                # No prediction in standstill mode
                if drivmode.value != 0:
                    for i in range(horizon):
                        # Extract pose from state prediction
                        x = X[i * 5 + 0]
                        y = X[i * 5 + 1]
                        phi = X[i * 5 + 2]
                        # Sometimes prediction does not cover entire horizon, last poses are 0 in this case
                        if x != 0.0 or y != 0.0 or phi != 0.0:
                            pose = PoseStamped()
                            pose.header.stamp = (now + Duration(seconds=i * dt)).to_msg()
                            pose.header.frame_id = base_link_frame
                            pose.pose.position.x = x
                            pose.pose.position.y = y
                            q = euler2quat(0.0, 0.0, phi)
                            pose.pose.orientation.w = q[0]
                            pose.pose.orientation.x = q[1]
                            pose.pose.orientation.y = q[2]
                            pose.pose.orientation.z = q[3]
                            pred_msg.poses.append(pose)
                self._pred_pub.publish(pred_msg)

            time_end = time.time()
            self.get_logger().debug(f'MPC update took {(time_end - time_start) * 1000:5.2f} ms, prep: {(time_start_mpc - time_start) * 1000:5.2f} ms, mpc: {(time_end_mpc - time_start_mpc) * 1000:5.2f} ms, control: {(time_pred - time_end_mpc) * 1000:5.2f} ms, pred: {(time_end - time_pred) * 1000:5.2f} ms')


def main(args=None):
    rclpy.init(args=args)
    node = NLMPCNode()
    executor = MultiThreadedExecutor(num_threads=2)

    try:
        rclpy.spin(node, executor=executor)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        executor.shutdown()


if __name__ == '__main__':
    main()