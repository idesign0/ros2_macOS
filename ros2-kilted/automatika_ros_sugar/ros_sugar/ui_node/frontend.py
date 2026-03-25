from typing import Dict, Sequence, Optional, List, Tuple
import logging
from datetime import datetime

from ros_sugar.io.topic import Topic
from . import elements

try:
    from fasthtml.common import *
    from monsterui.all import *

except ModuleNotFoundError as e:
    raise ModuleNotFoundError(
        "In order to use the dynamic web UI for your recipe, please install FastHTML & MonsterUI with `pip install python-fasthtml MonsterUI`"
    ) from e


class FHApp:
    def __init__(
        self,
        configs: Dict,
        in_topics: Sequence[Topic],
        out_topics: Sequence[Topic],
        srv_clients_configs: Optional[Sequence[Dict]] = None,
        action_clients_configs: Optional[Sequence[Dict]] = None,
        additional_input_elements: Optional[List[Tuple]] = None,
        additional_output_elements: Optional[List[Tuple]] = None,
        hide_settings_panel: bool = False,
    ):
        # --- Application Setup ---
        static_src = Path(__file__).resolve().parent / "static"

        hdrs = (
            Theme.red.headers(),  # Get theme from MonsterUI
            # --- 1. Add ROS Dependencies (CDN) ---
            Script(src="https://code.createjs.com/1.0.0/easeljs.min.js"),
            Script(
                src="https://cdn.jsdelivr.net/npm/eventemitter2@6.4.9/lib/eventemitter2.min.js"
            ),
            Script(src="https://cdn.jsdelivr.net/npm/roslib@1/build/roslib.min.js"),
            Script(src="https://cdn.jsdelivr.net/npm/ros2d@0/build/ros2d.min.js"),
            Script(
                src="custom.js",
            ),
            Script(
                src="audio_manager.js",
            ),
            Script(
                src="ros_maps.js",
            ),
            Script(
                src="video_manager.js",
            ),
            Link(rel="stylesheet", href="custom.css", type="text/css"),
        )
        self.app, self.rt = fast_app(
            hdrs=hdrs, exts=["ws"], static_path=str(static_src)
        )

        if not configs:
            logging.warning("No component configs provided to the UI")

        # Add any additional UI elements from derived packages (if any)
        # This method will populate the global elements._INPUT_ELEMENTS and elements._OUTPUT_ELEMENTS dictionaries
        # with the additional elements coming from derived packages
        elements.add_additional_ui_elements(
            input_elements=additional_input_elements,
            output_elements=additional_output_elements,
        )

        # Create settings UI
        self.configs = configs
        self.hide_settings_panel: bool = hide_settings_panel
        self.toggle_settings = False

        # Inputs and Outputs
        self.in_topics = in_topics
        self.out_topics = out_topics
        self.outputs = self._create_output_topics_ui(out_topics) if out_topics else None
        self.inputs = self._create_input_topics_ui(in_topics) if in_topics else None

        # Setup service clients
        self.srv_clients = (
            elements.styled_main_service_clients_container(
                srv_clients_configs, container_name="Services"
            )
            if srv_clients_configs
            else None
        )

        # Setup action clients
        self.action_clients_ft: Dict[str, elements.Task] = {}
        if action_clients_configs:
            for client in action_clients_configs:
                self.action_clients_ft[client["name"]] = elements.Task(
                    name=client["name"],
                    client_type=client["type"],
                    fields=client["fields"],
                )

        setup_toasts(self.app)

        # persistent elements
        self.outputs_log = elements.initial_logging_card()

    @property
    def action_clients(self) -> FT:
        if not self.action_clients_ft:
            return None
        all_clients_cards = Div(id="all_actions")
        for value in self.action_clients_ft.values():
            all_clients_cards(value.card)
        actions_main_card = Card(
            DivHStacked(
                H4("Tasks", cls="cool-subtitle-mini"),
                elements._toggle_button(div_to_toggle="all_actions"),
                cls="space-x-0 drag-handle",
            ),
            cls="main-card max-h-[80vh] overflow-y-auto draggable",
            id="actions",
            # NOTE: It is important to add these two variables to link this DOM element to the websocket.
            # This way any 'send' function applied from this websocket callback will update the element with the same class 'id'
            hx_ext="ws_actions",
            ws_connect="/ws_actions",
        )
        return actions_main_card(all_clients_cards)

    @property
    def _settings_button(self) -> str:
        return "Exit Settings" if self.toggle_settings else "Components Settings"

    def get_app(self):
        """Get the FastHTML app"""
        return self.app, self.rt

    def toasting(self, msg, session, toast_type="info", duration: int = 5000):
        session["duration"] = duration
        dismiss = (
            duration >= 20000
        )  # Id duration is more than 20seconds -> activate dismiss by default
        add_toast(session, f"{msg}", toast_type, dismiss)

    def get_all_stream_outputs(self) -> List[Tuple]:
        """Return all topics that connect to their own websocket for streaming"""
        return [
            (o.name, o.msg_type.__name__)
            for o in self.out_topics
            if (
                elements._OUTPUT_ELEMENTS.get(o.msg_type.__name__, None)
                == elements._out_image_element
            )
        ]

    def get_all_map_outputs(self) -> List[Tuple]:
        """Return all topics that connect to their own websocket for streaming"""
        return [
            (o.name, o.msg_type.__name__)
            for o in self.out_topics
            if (
                elements._OUTPUT_ELEMENTS.get(o.msg_type.__name__, None)
                == elements._out_map_element
            )
        ]

    def get_all_map_overlay_outputs(self) -> List[Tuple]:
        """Return all topics that should connect to the map websocket for displaying on the map
        This includes: (Point, PointStamped, Pose, PoseStamped, Odometry)"""
        # If a map element is present -> Get all point-like and path outputs to output as map markers
        if self.get_all_map_outputs():
            return [
                (o.name, o.msg_type.__name__)
                for o in self.out_topics
                if (
                    o.msg_type.__name__
                    in [
                        "Point",
                        "PointStamped",
                        "Pose",
                        "PoseStamped",
                        "Odometry",
                        "Path",
                    ]
                )
            ]
        return []

    def update_configs_from_data(self, data: Dict):
        """Update configs from a UI form data dict

        :param data: Component settings form data
        :type data: Dict
        """
        component_to_update = data["component_name"]
        for param in self.configs[component_to_update].keys():
            if param_value := data.get(param):
                self.configs[component_to_update][param]["value"] = param_value

    def _create_input_topics_ui(self, inputs: Sequence[Topic]):
        """Creates cards for Input Topics"""

        input_divs = []
        grid_id = "inputs-grid"
        inputs_container = elements.styled_main_inputs_container(grid_id)

        # Create a styled grid
        input_grid, inputs_columns_cls = elements.styled_inputs_grid(
            number_of_inputs=len(inputs)
        )

        has_map = True if self.get_all_map_outputs() else False

        for idx, inp in enumerate(inputs):
            input_divs.append(
                elements.input_topic_card(
                    topic_name=inp.name,
                    topic_type=inp.msg_type.__name__,
                    ros_msg_type=inp.ros_msg_type,
                    column_class=inputs_columns_cls[idx],
                    ft_has_map_element=has_map,
                ),
            )
        return inputs_container(input_grid(*input_divs, id=grid_id))

    def _create_output_topics_ui(self, outputs: Sequence[Topic]):
        """Creates cards for Output Topics"""
        displayed_outputs = [
            out
            for out in outputs
            if (
                (elm := elements._OUTPUT_ELEMENTS.get(out.msg_type.__name__, None))
                and (elm.__name__.startswith("_out"))
            )
        ]  # Get output elements that have specific display cards
        if not displayed_outputs:
            return None
        output_divs = []
        grid_id = "outputs-grid"
        outputs_container = elements.styled_main_outputs_container(grid_id)
        # Create a grid with max 2 outputs per line (1 per line for small views)
        output_grid, outputs_columns_cls = elements.styled_outputs_grid(
            number_of_outputs=len(displayed_outputs)
        )

        if map_overlays := self.get_all_map_overlay_outputs():
            map_outputs = {
                key: "path" if value == "Path" else "overlay"
                for (key, value) in map_overlays
            }
        else:
            map_outputs = None

        for idx, out in enumerate(displayed_outputs):
            output_divs.append(
                elements.output_topic_card(
                    out.name,
                    out.msg_type.__name__,
                    outputs_columns_cls[idx],
                    map_output_markers=map_outputs,
                )
            )
        return outputs_container(output_grid(*output_divs, id=grid_id))

    def _create_component_settings_ui(self, settings: Dict):
        """Creates a Div for component settings from a dictionary."""
        # Parse number of grid columns and column span based on the number of components
        # if len(settings) > 2:
        #     grid_num_cols = 2
        #     item_col_cls = "col-1"
        #     # Make the last component span over the whole width if we have an odd number of components
        #     last_col_cls = "col-span-full" if len(settings) % 2 == 1 else "col-1"
        # else:
        grid_num_cols = 1
        item_col_cls = "col-span-full"
        last_col_cls = "col-span-full"

        main_container = Grid(cols=grid_num_cols)

        all_component_forms = []
        for count, (component_name, component_settings) in enumerate(settings.items()):
            simple_ui_elements = []
            nested_ui_elements = []
            for setting_name, setting_details in component_settings.items():
                elements.parse_ui_elements_to_simple_and_nested(
                    component_name,
                    setting_name,
                    setting_details,
                    simple_ui_elements,
                    nested_ui_elements,
                )

            # Create an element for each component and add to the settings display
            all_component_forms.append(
                elements.component_settings_div(
                    component_name,
                    settings_col_cls=item_col_cls
                    if count < len(settings) - 1
                    else last_col_cls,
                    ui_elements=simple_ui_elements,
                    nested_ui_elements=nested_ui_elements,
                )
            )
        main_container(*all_component_forms)
        return main_container

    def _get_current_time(self):
        """Returns the current time as HH:MM."""
        return datetime.now().strftime("%H:%M")

    @property
    def settings(self):
        """Get components settings"""
        return (
            self._create_component_settings_ui(self.configs)
            if not self.hide_settings_panel
            else None
        )

    @property
    def _main_content(self):
        """Get the current main page content"""
        # Return the settings page if toggle button is pressed and settings display is allowed
        if self.toggle_settings and not self.hide_settings_panel:
            return self.settings

        # Otherwise return the main page
        main_grid = Grid(id="modal-container", cols=2, name="draggables-grid")
        if self.outputs:
            main_grid(
                Div(
                    elements.output_logging_card(self.outputs_log),
                    id="log-frontend",
                ),
                Div(self.outputs, id="outputs-frontend"),
            )
        else:
            main_grid(
                Div(
                    elements.output_logging_card(self.outputs_log),
                    cls="col-span-full",
                    id="log-frontend",
                ),
            )
        if self.inputs:
            main_grid(
                Div(
                    self.inputs,
                    cls="col-span-full",
                    id="inputs-frontend",
                ),
            )
        if self.srv_clients:
            main_grid(
                Div(
                    self.srv_clients,
                    cls="col-span-full",
                    id="srv-frontend",
                )
            )
        if self.action_clients:
            main_grid(
                Div(
                    self.action_clients,
                    cls="col-span-full",
                    id="actions-frontend",
                )
            )

        return Main(
            main_grid,
            Div(id="result"),
            id="main",
            cls="pt-2 pb-2",
            # connect to the default websocket
            hx_ext="ws",
            ws_connect="/ws",
            # NOTE: Function defined in custon js to reconnect streaming websockets
            # HTMX fires "htmx:afterSwap" after content is swapped into the DOM,
            # and "htmx:afterOnLoad" / "htmx:afterRequest" for other lifecycle stages
            # Respond to afterSwap and afterOnLoad to be safe.
            hx_on__after_on_load="ensureConnectionsForPresentFrames",
            hx_on__after_swap="ensureConnectionsForPresentFrames",
        )

    @property
    def _nav_bar(self) -> FT:
        # Case 1: Settings page is displayed
        if self.toggle_settings and not self.hide_settings_panel:
            nav_bar_items = [
                Button(
                    self._settings_button,
                    id="settings-button",
                    hx_get="/settings/show",
                    hx_target="#main",
                    hx_swap="outerHTML",
                    cls="primary-button",
                ),
            ]
        # Case 2: Main page
        else:
            nav_bar_items = [
                elements.filter_tag_button(name="log", div_to_hide="log-frontend")
            ]
            if self.outputs:
                nav_bar_items.append(
                    elements.filter_tag_button(
                        name="outputs", div_to_hide="outputs-frontend"
                    ),
                )
            if self.inputs:
                nav_bar_items.append(
                    elements.filter_tag_button(
                        name="inputs", div_to_hide="inputs-frontend"
                    ),
                )
            if self.srv_clients:
                nav_bar_items.append(
                    elements.filter_tag_button(
                        name="services", div_to_hide="srv-frontend"
                    ),
                )
            if self.action_clients:
                nav_bar_items.append(
                    elements.filter_tag_button(
                        name="tasks", div_to_hide="actions-frontend"
                    ),
                )
            # If user is allowed to access the settings panel -> add the toggle button
            if not self.hide_settings_panel:
                nav_bar_items.append(
                    Button(
                        self._settings_button,
                        id="settings-button",
                        hx_get="/settings/show",
                        hx_target="#main",
                        hx_swap="outerHTML",
                        cls="primary-button",
                    ),
                )
        nav_bar = NavBar(
            *nav_bar_items,
            Button(
                UkIcon("sun", cls="dark:hidden"),
                UkIcon("moon", cls="hidden dark:block"),
                onclick="toggleTheme()",  # from custom.js
                type="button",
                cls="glass-icon-btn",
                uk_tooltip="title: Change Theme; pos: left",
            ),
            brand=Div(
                # Dark Mode Logo (Visible by default)
                Img(
                    src="https://automatikarobotics.com/Emos_dark.png",
                    cls="brand-logo brand-dark",
                ),
                # Light Mode Logo (Hidden by default, shown when html.light exists)
                Img(
                    src="https://automatikarobotics.com/Emos_light.png",
                    cls="brand-logo brand-light",
                ),
            ),
        )
        return nav_bar

    def get_main_page(self):
        """Serves the main page of the UI"""
        # Serve main page
        return (
            Favicon(light_icon="automatika-icon.png", dark_icon="automatika-icon.png"),
            Title("EMOS UI"),
            Div(
                Container(
                    self._nav_bar,
                    self._main_content,
                    id="main",
                    cls="mt-0 mb-0",
                ),
                cls="grid-section",
            ),
        )
