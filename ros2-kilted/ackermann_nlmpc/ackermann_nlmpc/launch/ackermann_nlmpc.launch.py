from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # common parameters
    base_link_frame = 'base_link'  # name of the base link coordinate frame
    odom_frame = 'odom'  # name of the odometry coordinate frame
    lib_path = 'codegen/autompc_ros'  # Path to the shared library file, relative paths are assumed to be relative to the package path, absolute paths are possible.
                                      # Including a .c file extension will always recompile the library.
                                      # Including a .so file extension will just load the .so file.
                                      # When omitting the file extension the node will check for .c and .so files and compile if no .so file is present or the .c was modified more recently than the .so file.
                                      # If the lib_path is not writable the node will fall back to the ~/.ros/ackermann_nlmpc/ directory.
    dt = 0.1  # sampling time [s], must match the sampling time used in the MPC code generation
    max_nodes = 2500  # maximum number of nodes in the path, must match the maximum number of nodes used in the MPC code generation
    horizon = 40 # prediction horizon [time steps], must match the prediction horizon used in the MPC code generation

    # control node spawning
    spawn_sim = True  # whether to spawn the simulation node
    spawn_trajectory_converter = True  # whether to spawn the trajectory converter node

    nodes = []
    nodes.append(
        Node(
            package='ackermann_nlmpc',
            executable='ackermann_nlmpc_node',
            name='ackermann_nlmpc_node',
            namespace='ackermann_nlmpc',
            output='screen',
            emulate_tty=True,
            parameters=[{
                'base_link_frame': base_link_frame,
                'odom_frame': odom_frame,
                'lib_path': lib_path,
                'dt': dt,
                'max_nodes': max_nodes,
                'horizon': horizon,
                'max_message_age': 0.5,                 # maximum message age [s] for odometry and steering angle data, if latest data is older than this, the controler will not be executed
                'publish_predicted_trajectory': True,   # whether to publish the predicted trajectory as a Path message
                'publish_reference_trajectory': True,   # whether to publish the reference trajectory as a Path message and the state constraints as a Polygon message
                'weight_acceleration': 0.5,             # penalty weight on u(1), acceleration
                'weight_steering_rate': 0.5,            # penalty weight on u(2), steering rate
                'weight_lateral_offset': 100.0,         # penalty weight on x(1), lateral offset
                'weight_longitudinal_offset': 100.0,    # penalty weight on x(2), longitudinal offset
                'weight_heading': 100.0,                # penalty weight on x(3), heading coordinate psi
                'weight_velocity': 0.1,                 # penalty weight on x(4), velocity
                'weight_steering_angle': 0.1,           # penalty weight on x(5), steering angle
                'contolerance': 1e-3,                   # tolerance for state constraint violation (>0)
                'conpenalty': 1e6,                      # penalty parameter for soft state constraints (>0)
                'acceleration_min': -5.0,               # input constraint, minimum accleration [m/s^2]
                'acceleration_max': 2.5,                # input constraint, maximum accleration [m/s^2]
                'jerk_min': -50.0,                      # input constraint, minimum jerk [m/s^3]
                'jerk_max': 50.0,                       # input constraint, maximum jerk [m/s^3]
                'steering_rate_limit': 0.2167,          # input constraint, maximum steering rate [rad/s]
                'steering_rate_change_limit': 5.0       # input constraint, maximum steering rate change [rad/s^2]
            }],
            remappings=[
                ('trajectory', 'trajectory'),
                ('control', 'control'),
                ('odom', 'odom'),
                ('steering_angle', 'steering_angle'),
            ]
        )
    )
    if spawn_sim:
        nodes.append(
            Node(
                package='ackermann_nlmpc',
                executable='ackermann_sim_node',
                name='ackermann_sim_node',
                namespace='ackermann_nlmpc',
                output='screen',
                emulate_tty=True,
                parameters=[{
                    'base_link_frame': base_link_frame,
                    'odom_frame': odom_frame,               # publish odometry in this frame
                    'dt': dt,
                    'amax': 5.0,                            # maximum acceleration [m/s^2]
                    'dmax': 0.7854, # pi/4                  # maximum steering angle [rad]
                    'lr': 1.738,                            # distance from rear axle to center of mass [m]
                    'lf': 1.105,                            # distance from front axle to center of mass [m]
                }],
                remappings=[
                    ('control', 'control'),
                    ('odom', 'odom'),
                    ('steering_angle', 'steering_angle'),
                ]
            )
        )
    if spawn_trajectory_converter:
        nodes.append(
            Node(
                package='ackermann_nlmpc',
                executable='trajectory_converter_node',
                name='trajectory_converter_node',
                namespace='ackermann_nlmpc',
                output='screen',
                emulate_tty=True,
                parameters=[{
                    'base_link_frame': base_link_frame,
                    'max_nodes': max_nodes,
                    'default_velocity': 2.0,                # default target velocity [m/s] used for the trajectory
                    'default_pathwidth_l': 1.0,             # default path width left [m] used for the trajectory
                    'default_pathwidth_r': 1.0,             # default path width right [m] used for the trajectory
                }],
                remappings=[
                    ('trajectory', 'trajectory'),
                    ('path', 'path'),
                    ('goal_pose', '/goal_pose'),
                ]
            )
        )

    return LaunchDescription(nodes)