import importlib
from typing import List, Dict, Any, Optional
from functools import partial
from ..utils import logger
from ..io.supported_types import SupportedType, get_ros_msg_fields_dict

from .utils import parse_type

import subprocess

try:
    from fasthtml.common import *
    from monsterui.all import *

except ModuleNotFoundError as e:
    raise ModuleNotFoundError(
        "In order to use the dynamic web UI for your recipe, please install FastHTML & MonsterUI with `pip install python-fasthtml MonsterUI`"
    ) from e

# NOTE: All custom classes names are implemented in custom.css (for style specific classes)
# Some class names are linked with a custom behavior implemented in custom.js (such as 'draggable' class)

# NOTE: 'hx_post' calls (used in Buttons) are implemented in scripts/ui_node_executable


# ---- TASK ELEMENT ----
class Task:
    """Task (ROS2 Action) Element Class"""

    def __init__(self, name: str, client_type: str, fields):
        """

        :param name: Action Name
        :type name: str
        :param client_type: Action Type
        :type client_type: str
        :param fields: Action Request Fields
        :type fields: _type_
        """
        self._status = "inactive"
        self._feedback = None
        self._duration = None
        self._name = name
        self._type = client_type
        self._fields = fields
        self._feedback_card = Card(
            header=H6(">> Feedback Log", cls="tomorrow-night-green"),
            cls="terminal-container ml-2 mr-2 mt-0 overflow-y-auto ",
            id="feedback-log",
        )
        self._serving_component: Optional[str] = self.__get_server_node()
        self.total_calls = 0

    def is_active(self) -> bool:
        """Check if the task is active (running/active/accepted)

        :return: True if the task is active
        :rtype: bool
        """
        return self._status in ["running", "active", "accepted"]

    def update(
        self,
        *,
        status: Optional[str] = None,
        feedback: Any = None,
        duration: Optional[float] = None,
    ):
        """Update the task current status

        :param status: New status
        :type status: str
        """
        if status:
            self._status = status
        if feedback:
            self._feedback = feedback
        if duration is not None:
            self._duration = duration

    @property
    def card(self) -> FT:
        """Get a UI card element for the task tracking and request

        :return: _description_
        :rtype: FT
        """
        client_card = Card(
            header=DivHStacked(
                H4(self._name),
                self._badge,
                Button(
                    "?",
                    cls="info-btn",
                    id=f"{self._name}-info-btn",
                    onclick=f"openAtButton('{self._name}-info-btn', '{self._name}-modal')",  # Method openAtButton implemented in custom.js
                ),
                self._info,
            ),
            header_cls="mb-0",
            cls="m-2 max-h-[40vh] overflow-y-auto inner-main-card",
            id=self._name,
            ws_send=True,
        )
        # TODO: Format the feedback message and add a display card
        inside = Grid(cls="gap-2 ml-1 mr-1", cols=1)
        if self.feedback:
            self._feedback_card(self.feedback)
            inside(self._feedback_card)
        inside(_in_action_client_element(self._name, self._type, self._fields))
        return client_card(inside)

    @property
    def feedback(self):
        """Get a UI element for the task feedback"""
        if self._status not in ["active", "running"]:
            return None
        else:
            return P(self._feedback)

    @property
    def _info(self):
        """Gets the action server info"""
        # Get the server node name
        if self._serving_component is None:
            # Try to search again
            self._serving_component = self.__get_server_node()
        return (
            Dialog(
                Card(
                    Grid(
                        P(
                            Strong("Active Serving Component: "),
                            self._serving_component or "Not found",
                        ),
                        P(Strong("Total Sent Calls: "), self.total_calls),
                        P(Strong("Type: "), self._type),
                        cols=1,
                        cls="gap-1",
                    ),
                    header=DivHStacked(
                        Button(
                            "✕",
                            cls="info-btn",
                            onclick="this.closest('dialog').close()",
                        ),
                        H5("Task Info", cls="cool-subtitle-mini-blue"),
                    ),
                ),
                id=f"{self._name}-modal",
            ),
        )

    def __get_server_node(self) -> Optional[str]:
        """
        Executes 'ros2 action info <action_name>' and returns the active server node name if found.
        """
        try:
            # Execute the command
            result = subprocess.run(
                ["ros2", "action", "info", self._name],
                capture_output=True,
                text=True,
                check=True,
            )

            output = result.stdout

        except Exception:
            return None

        # The output format is standard:
        # Action: /turtle1/rotate_absolute
        # Action clients: 1
        #     /teleop_turtle
        # Action servers: 1
        #     /turtlesim
        for key, line in enumerate(output.splitlines()):
            line = line.strip()
            if line.startswith("Action servers:"):
                if line.removeprefix("Action servers:").strip() == "0":
                    return None
                else:
                    return output.splitlines()[key + 1].strip()

    @property
    def _badge(self):
        """Create a status badge"""
        if self._status in ["running", "active", "accepted"]:
            # Add loading
            status_div = DivHStacked(
                Span(
                    DivHStacked(
                        Loading(cls=f"{LoadingT.spinner} {LoadingT.xs}"), self._status
                    ),
                    cls=f"status-badge {self._status}",
                    id="status-badge-div",
                )
            )
            if self._duration is not None:
                minutes, seconds = divmod(self._duration, 60)
                status_div(Span(f"{minutes}:{seconds}", cls="slick-timer"))
            return status_div
        return Span(
            self._status, cls=f"status-badge {self._status}", id="status-badge-div"
        )


# ---- UTILITY ELEMENTS ----


def _toggle_button(div_to_toggle: Optional[str] = None, **kwargs):
    """UI arrow button to use for show/hide toggle of a Div with a given ID

    :param div_to_toggle: Id of the Div to show/hide on button click, defaults to None
    :type div_to_toggle: Optional[str], optional
    :return: UI Button Element
    :rtype: FT
    """
    _arrow_down = UkIcon("chevrons-down")
    _arrow_up = UkIcon("chevrons-up")
    # Toggle the button between up <> down on every click
    onclick = f"""
                if (this.name == 'down')
                {{
                    this.innerHTML=`{_arrow_up}`;
                    this.name = 'up';
                }}
                else{{
                    this.innerHTML=`{_arrow_down}`;
                    this.name = 'down'}};
                """
    if div_to_toggle:
        toggle_click = f"""
                    let toggleDiv = document.getElementById('{div_to_toggle}');
                    toggleDiv.hidden = ! toggleDiv.hidden;
                    if (toggleDiv.hidden){{
                        toggleDiv.style.display = "none";
                    }} else {{
                        toggleDiv.style.display = "";
                    }}
                """
        onclick = f"{onclick}\n{toggle_click}"
    if kwargs.get("onclick", None):
        onclick = f"{onclick}\n{kwargs.get('onclick')}"
        kwargs.pop("onclick")
    return Button(
        _arrow_down,
        type="button",
        name="down",
        cls="no-drag uk-icon-button  btn-touch-target",
        onclick=onclick,
        **kwargs,
    )


def _fullscreen_button(div_id: str):
    """
    Button to toggle fullscreen mode for a specific div_id.
    """
    return Button(
        UkIcon("expand"),
        type="button",
        cls="no-drag uk-icon-button",  # Prevent dragging when clicking this
        onclick=f"toggleFullScreen(this, '{div_id}')",
        uk_tooltip="title: Full Screen; pos: left",
    )


def _map_overlay_settings_panel(map_id: str, element_id: str, overlay_type: str):
    """Helper method to get a setting panel for one map overlay marker (point or path)

    :param map_id: Map ID
    :type map_id: str
    :param element_id: Marker element ID
    :type element_id: str
    :param overlay_type: Path or Overlay Point
    :type overlay_type: str

    :return: Settings panel
    :rtype: FT
    """
    # Unique wrapper ID for toggling visibility
    wrapper_id = f"visuals-{map_id}-{element_id}"
    # We hide all blocks by default; the script below will show the selected one
    block_cls = f"visual-settings-block-{map_id} hidden"

    if overlay_type == "path":
        # === PATH SETTINGS ===
        # Needs: Color, Line Width, Line Style
        content = Div(
            # Path Color
            LabelInput(
                label="Path Color",
                type="color",
                name=f"color_{element_id}",
                value="#2ECC71",
                id=f"picker-{map_id}-{element_id}",
                cls="form-input space-y-1",
            ),
            # Width Slider
            LabelInput(
                label="Line Width",
                name=f"width_{element_id}",
                type="range",
                value="3",
                min="1",
                max="10",
                cls="form-input space-y-1",
            ),
            # Style Select
            LabelSelect(
                Option("Solid", value="solid"),
                Option("Dashed", value="dashed"),
                Option("Dots", value="dots"),
                label="Line Style",
                name=f"style_{element_id}",
                cls="form-input space-y-1",
            ),
            id=wrapper_id,
            cls=block_cls,
        )
    else:
        # === OVERLAY/POINT SETTINGS (Matches your snippet) ===
        # Needs: Shape, Color Picker
        content = Div(
            # Color Picker (Hidden by default)
            LabelInput(
                label="Select Color",
                type="color",
                name=f"color_{element_id}",
                value="#E83F3F",
                id=f"picker-{map_id}-{element_id}",
                cls="form-input space-y-1",
            ),
            id=wrapper_id,
            cls=block_cls,
        )
    return content


def _map_settings_modal(map_id: str, overlays: Optional[Dict] = None):
    """
    Creates a popup dialog to configure Clicked Point settings and Visuals.
    overlays: dict e.g. {'robot_pose': 'overlay', 'global_plan': 'path'}
    """
    if overlays is None:
        overlays = {}

    # 1. Create Dropdown Options
    id_options = [Option(k, value=k) for k in overlays.keys()]

    # 2. Generate Hidden Settings Blocks for each ID
    settings_blocks = []

    for oid, otype in overlays.items():
        settings_blocks.append(
            _map_overlay_settings_panel(
                map_id=map_id, element_id=oid, overlay_type=otype
            )
        )

    return Div(
        Div(
            Form(
                # --- CLICKED POINT SETTINGS ---
                Card(
                    H5("Published Point Settings", cls="cool-title"),
                    LabelInput(
                        label="Topic Name",
                        name="clicked_point_topic",
                        value="clicked_point",
                        placeholder="e.g. /goal_pose",
                        cls="form-input space-y-1",
                    ),
                    LabelSelect(
                        Option("PointStamped", value="PointStamped", selected=True),
                        Option("Point", value="Point"),
                        Option("PoseStamped", value="PoseStamped"),
                        Option("Pose", value="Pose"),
                        label="Message Type",
                        name="clicked_point_type",
                        cls="form-input space-y-1",
                    ),
                    cls="space-y-3 mb-4 main-card",
                ),
                Card(
                    H5("Output Visuals", cls="cool-title"),
                    # 1. The ID Selector
                    LabelSelect(
                        *id_options,
                        label="Topic Name",
                        placeholder="Select output topic...",
                        name="selected_visual_id",
                        cls="form-input space-y-1",
                        id=f"visual-selector-{map_id}",
                    ),
                    # 2. Settings Container
                    Div(
                        *settings_blocks,
                        cls="p-3 min-h-[160px]",
                    ),
                    cls="space-y-3 main-card",
                ),
                # --- SAVE BUTTON ---
                Div(
                    Button(
                        "Save Settings",
                        cls="primary-button w-full justify-center",
                        onclick=f"saveMapSettings('{map_id}')",
                        type="button",
                    ),
                    cls="pt-4",
                ),
                id=f"{map_id}-settings-form",
                cls="flex flex-col space-y-2",
            ),
            cls="modal-box p-5 space-y-4 max-w-sm",
            onclick="event.stopPropagation()",
        ),
        id=f"{map_id}-settings-modal",
        cls="custom-overlay backdrop-blur-sm",
        # Initialize Observer (Runs once)
        # Force Initial Update (Runs every time mouse enters to ensure UI is in sync)
        onmouseenter=f"initVisualSettingsObserver('{map_id}'); updateVisualSettingsVisibility(document.getElementById('visual-selector-{map_id}'), '{map_id}')",
        onclick="this.style.display='none'",
    )


def _map_control_buttons(map_id: str, map_output_markers: Optional[Dict] = None):
    """
    Overlay buttons for Zoom In/Out and Publish Point.
    """
    return DivHStacked(
        # Zoom In
        Button(
            UkIcon("plus"),
            cls="glass-icon-btn",
            onclick=f"zoomMap('{map_id}', 1.2)",
            type="button",
            uk_tooltip="title: Zoom In; pos: left",
        ),
        # Zoom Out
        Button(
            UkIcon("minus"),
            cls="glass-icon-btn",
            onclick=f"zoomMap('{map_id}', 0.8)",
            type="button",
            uk_tooltip="title: Zoom Out; pos: left",
        ),
        # Publish Point Button
        Button(
            UkIcon("mapPin"),
            cls="glass-icon-btn",
            onclick="togglePublishPoint(this)",  # Calls JS function
            id=f"{map_id}-publish-btn",
            type="button",
            uk_tooltip="title: Publish Clicked Point; pos: left",
        ),
        # Settings (Gear Icon)
        Button(
            UkIcon("settings"),
            cls="glass-icon-btn",
            id=f"{map_id}-settings-btn",
            onclick=f"openMapSettings('{map_id}')",
            type="button",
            uk_tooltip="title: Settings; pos: left",
        ),
        # THE SETTINGS MODAL (Hidden by default, popped up by openMapSettings)
        _map_settings_modal(map_id, map_output_markers),
        cls="flex flex-row space-x-2 no-drag",
    )


def filter_tag_button(name: str, div_to_hide: str, **kwargs):
    """UI arrow button to use for show/hide toggle of a Div with a given ID

    :param div_to_toggle: Id of the Div to show/hide on button click, defaults to None
    :type div_to_toggle: Optional[str], optional
    :return: UI Button Element
    :rtype: FT
    """
    # Toggle the button between up <> down on every click
    onclick = """
                console.log(this);
                if (this.name == 'active')
                {{
                    this.classList.value = "uk-btn filter-btn inactive";
                    this.name = 'inactive';
                }}
                else{{
                    this.classList.value = "uk-btn filter-btn active";
                    this.name = 'active'}};
                """
    toggle_click = f"""
                let toggleDiv = document.getElementById('{div_to_hide}');
                toggleDiv.hidden = ! toggleDiv.hidden;
                if (toggleDiv.hidden){{
                    toggleDiv.style.display = "none";
                }} else {{
                    toggleDiv.style.display = "";
                }}
            """

    onclick = f"{onclick}\n{toggle_click}"
    if kwargs.get("onclick", None):
        onclick = f"{onclick}\n{kwargs.get('onclick')}"
        kwargs.pop("onclick")
    return Button(
        name,
        type="button",
        name="active",
        cls="filter-btn",
        onclick=onclick,
        **kwargs,
    )


# ---- CUSTOM INPUT MESSAGES ELEMENTS ----


def _in_text_element(topic_name: str, topic_type: str, **_):
    """FastHTML element for input String type"""
    field_type = "number" if topic_type in ["Float32", "Float64"] else "text"
    return (
        Form(cls="mb-1 p-1")(
            Input(name="topic_name", type="hidden", value=topic_name),
            Input(name="topic_type", type="hidden", value=topic_type),
            Input(
                name="data",
                placeholder="String data...",
                type=field_type,
                required=True,
                autocomplete="off",
            ),
            id=f"{topic_name}-form",
            ws_send=True,
            hx_on__ws_after_send="this.reset(); return false;",
        ),
    )


def _in_bool_element(topic_name: str, topic_type: str, **_):
    """FastHTML element for input String type"""
    return (
        Form(cls="mb-1 p-1")(
            Input(name="topic_name", type="hidden", value=topic_name),
            Input(name="topic_type", type="hidden", value=topic_type),
            DivLAligned(
                Switch(
                    id=topic_name,
                    checked=False,
                    name="data",
                ),
                Button("Send", type="submit", title="Send", cls="primary-button"),
                cls="space-x-4 ml-2",
            ),
            id=f"{topic_name}-form",
            ws_send=True,
            hx_on__ws_after_send="this.reset(); return false;",  # Reset the form fields values in the UI after sending
        ),
    )


def _in_audio_element(topic_name: str, **_):
    """FastHTML element for input Audio type"""
    return DivCentered(
        DivHStacked(
            P("Send audio: "),
            Button(
                UkIcon(icon="mic"),
                id=topic_name,
                onclick="startAudioRecording(this)",  # Method implemented in custom.js
                cls=f"{AT.primary} uk-icon-button",
                uk_tooltip="title: Record; pos: left",
            ),
        ),
        cls="no-drag ",
    )


def __pop_up_form(topic_name: str, form_elements: tuple):
    """Helper method to construct a cool pop-up form"""
    return Div(
        Div(
            H4(
                f"Message Value for sending '{topic_name}'",
                cls="cool-subtitle-mini-blue mb-2",
            ),
            Form(
                DivVStacked(
                    *form_elements,
                    DivHStacked(
                        Button("Submit", cls="primary-button", type="submit"),
                        Button(
                            "Cancel",
                            cls="secondary-button",
                            onclick="this.closest('.custom-overlay').style.display='none'",
                            type="button",
                        ),
                        cls="space-x-2 justify-center mt-4",
                    ),
                ),
                cls="space-x-2 space-y-2 mr-2 mb-2",
                ws_send=True,
                hx_on__ws_after_send="this.reset(); this.closest('.custom-overlay').style.display='none'; return false;",
            ),
            cls="modal-box",
            onclick="event.stopPropagation()",
        ),
        cls="custom-overlay",
        id=f"{topic_name}-manual-form",
        # Logic: Clicking the dark background (the overlay) closes itself
        onclick="this.style.display='none'",
    )


def __location_element_input(
    topic_name: str, topic_type: str, elements: tuple, has_map: bool = True
):
    """Helper method to construct element for generic location types

    :param topic_name: Topic name
    :type topic_name: str
    :param topic_type: Message type
    :type topic_type: str
    :param elements: Manual input form elements
    :type elements: tuple
    :param has_map: If the UI has a map element, defaults to True
    :type has_map: bool, optional
    :return: Location element
    :rtype: FT
    """
    inner_fields = DivFullySpaced(
        Input(name="topic_name", type="hidden", value=topic_name),
        Input(name="topic_type", type="hidden", value=topic_type),
        cls="space-x-2",
    )
    if has_map:
        inner_fields(
            Button(
                UkIcon("mapPin"),
                cls="glass-icon-btn",
                onclick="togglePublishPoint(this)",  # Calls JS function
                id=f"{topic_name}-publish-btn",
                type="button",
                uk_tooltip="title: Publish Point On Map; pos: left",
                data_topic=topic_name,
                data_type=topic_type,
            ),
            Button(
                "Enter Data Manually",
                cls="primary-button",
                type="button",
                onclick=f"document.getElementById('{topic_name}-manual-form').style.display='grid'",
            ),
            __pop_up_form(
                topic_name,
                elements,
            ),
        )
    else:
        inner_fields(
            *elements,
            Button("Submit", cls="primary-button", type="submit"),
        )
    return (
        Form(
            DivVStacked(
                *inner_fields,
            ),
            cls="space-x-2 space-y-2 mr-2 mb-2",
            id=f"{topic_name}-form",
            ws_send=True,
            hx_on__ws_after_send="this.reset(); return false;",
        ),
    )


def _in_point_element(
    topic_name: str, topic_type: str, stamped: bool, has_map: bool = False, **_
):
    """FastHTML element for 3D point type"""
    elements = DivHStacked(
        Input(
            placeholder="X",
            name="x",
            type="number",
            required=True,
            autocomplete="off",
        ),
        Input(
            placeholder="Y",
            name="y",
            type="number",
            required=True,
            autocomplete="off",
        ),
        Input(
            placeholder="Z",
            name="z",
            type="number",
            required=True,
            autocomplete="off",
        ),
    )
    if stamped:
        elements(
            Input(
                placeholder="FrameId",
                name="frame_id",
                type="text",
                required=True,
                autocomplete="off",
            ),
        )
    return __location_element_input(
        topic_name=topic_name,
        topic_type=topic_type,
        has_map=has_map,
        elements=(elements,),
    )


def _in_pose_element(
    topic_name: str, topic_type: str, stamped: bool, has_map: bool = False, **_
):
    """FastHTML element for 3D point type"""
    _pose_form_fields = (
        P("Position:"),
        DivFullySpaced(
            Input(name="topic_name", type="hidden", value=topic_name),
            Input(name="topic_type", type="hidden", value=topic_type),
            Input(
                placeholder="X",
                name="x",
                type="number",
                required=True,
                autocomplete="off",
            ),
            Input(
                placeholder="Y",
                name="y",
                type="number",
                required=True,
                autocomplete="off",
            ),
            Input(
                placeholder="Z",
                name="z",
                type="number",
                required=True,
                autocomplete="off",
            ),
            cls="space-x-2",
        ),
        DivHStacked(
            P("Orientation (Optional):"),
            # Toggle button for only the orientation fields (skips the first 6 position fields of the form)
            _toggle_button(
                onclick="""
                                for (let i = 6; i < this.form.length -1 ; i++)
                                {{this.form[i].hidden = !this.form[i].hidden;}}
                                """
            ),
            cls="space-x-0",
        ),
        DivFullySpaced(
            Input(
                placeholder="W",
                name="ori_w",
                type="number",
                autocomplete="off",
                hidden=True,
            ),
            Input(
                placeholder="X",
                name="ori_x",
                type="number",
                autocomplete="off",
                hidden=True,
            ),
            Input(
                placeholder="Y",
                name="ori_y",
                type="number",
                autocomplete="off",
                hidden=True,
            ),
            Input(
                placeholder="Z",
                name="ori_z",
                type="number",
                autocomplete="off",
                hidden=True,
            ),
            cls="space-x-2",
        ),
    )
    if stamped:
        _pose_form_fields(
            Input(
                placeholder="FrameId",
                name="frame_id",
                type="text",
                required=True,
                autocomplete="off",
            ),
        )
    return __location_element_input(
        topic_name=topic_name,
        topic_type=topic_type,
        has_map=has_map,
        elements=_pose_form_fields,
    )


# ---- CUSTOM OUTPUT MESSAGES ELEMENTS ----


def _out_image_element(topic_name: str, **_):
    """FastHTML element for output Image/CompressedImage type"""
    return DivCentered(
        Img(id=topic_name, name="video-frame", src="", cls="h-[40vh] w-auto")
    )


def _out_map_element(topic_name: str, map_output_markers: Optional[Dict] = None, **_):
    """FastHTML element for output OccupancyGrid typ"""
    return (
        Grid(
            DivHStacked(
                _map_control_buttons(topic_name, map_output_markers),
            ),
            Div(
                id=topic_name,
                name="map-canvas",
                style="width: 100%; height: auto; background-color: #333;",
            ),
            cols=1,
            cls="space-y-2 inner-main-card p-2 m-0",
        ),
    )


def _log_audio_element(logging_card, output, data_src: str, id: str = "audio"):
    """Adds an Audio output or input to the main logging card"""
    return logging_card(_styled_logging_audio(output, data_src, id))


def _log_text_element(logging_card, output, data_src: str, id: str = "text"):
    """Adds a Text output or input to the main logging card"""
    return logging_card(_styled_logging_text(output, data_src, id))


# _INPUT_ELEMENTS and _OUTPUT_ELEMENTS are the main dictionaries used to link message types with their UI elements
# (both inputs: _INPUT_ELEMENTS and outputs: _OUTPUT_ELEMENTS)
# Other Sugarcoat-based packages can implement their own UI elements
# and these dictionaries will be populated and updated automatically

_INPUT_ELEMENTS: Dict = {
    "String": _in_text_element,
    "Float32": _in_text_element,
    "Float64": _in_text_element,
    "Bool": _in_bool_element,
    "Audio": _in_audio_element,
    "Point": partial(_in_point_element, stamped=False),
    "PointStamped": partial(_in_point_element, stamped=True),
    "Pose": partial(_in_pose_element, stamped=False),
    "PoseStamped": partial(_in_pose_element, stamped=True),
}

_OUTPUT_ELEMENTS: Dict = {
    "String": _log_text_element,
    "Float32": _log_text_element,
    "Float64": _log_text_element,
    "Bool": _log_text_element,
    "Audio": _log_audio_element,
    "Image": _out_image_element,
    "CompressedImage": _out_image_element,
    "OccupancyGrid": _out_map_element,
}


# ---- ADDITIONAL MESSAGES ELEMENTS ----


def _deserialize_additional_element(k_t: str, i_t: str) -> Optional[Tuple]:
    """Deserialize one additional element"""
    # Get key type
    module_name_key, _, type_name = k_t.rpartition(".")
    if not module_name_key:
        logger.error(f"Could not find module name for {k_t}")
        return
    # Get item func
    module_name_item, _, func_name = i_t.rpartition(".")
    if not module_name_item:
        logger.error(f"Could not find module name for {i_t}")
        return
    module_key = importlib.import_module(module_name_key)
    module_item = importlib.import_module(module_name_item)
    key = getattr(module_key, type_name)
    if not issubclass(key, SupportedType):
        logger.error(f"Could not find {type_name} name in {module_key} module")
        return
    item = getattr(module_item, func_name)
    if not callable(item):
        logger.error(f"Could not find {func_name} name in {module_item} module")
        return
    return key, item


def add_additional_ui_elements(
    input_elements: Optional[List[Tuple]], output_elements: Optional[List[Tuple]]
):
    """Deserialize additional elements and add them"""
    global _INPUT_ELEMENTS, _OUTPUT_ELEMENTS

    # Add input elements
    if input_elements:
        for k_t, i_t in input_elements:
            deserialized = _deserialize_additional_element(k_t, i_t)
            if deserialized:
                _INPUT_ELEMENTS[deserialized[0].__name__] = deserialized[1]

    # Add output elements
    if output_elements:
        for k_t, i_t in output_elements:
            deserialized = _deserialize_additional_element(k_t, i_t)
            if deserialized:
                _OUTPUT_ELEMENTS[deserialized[0].__name__] = deserialized[1]


# ---- GENERIC MESSAGES ELEMENTS ----


def _generic_message_form(msg_fields: Dict[str, Dict[str, Dict]]) -> FT:
    """Creates an input UI element (form) for any ROS2 message

    :param msg_fields: Dictionary of the message fields {name: type | Dict}
    :type msg_fields: Dict[str, Dict[str, Dict]]
    :return: UI form input element
    :rtype: FT
    """
    ui_fields = Grid(cls="gap-2 m-1", cols=2)
    for field_name, field_type in msg_fields.items():
        if isinstance(field_type, Dict):
            ui_fields(
                DivVStacked(
                    P(f"{field_name}", cls="cool-subtitle-mini-blue m-2"),
                    _generic_message_form(field_type),
                    cls="card-border",
                ),
            )
            continue
        elif field_type in [
            "float",
            "float32",
            "float64",
            "double",
            "int8",
            "int16",
            "int32",
            "int64",
            "uint8",
            "uint16",
            "uint32",
            "uint64",
        ]:
            field_input_type = "number"
            default_value = "0"
        elif field_type == "bool":
            field_input_type = "checkbox"
            default_value = "0"
        else:
            field_input_type = "text"
            default_value = ""
        ui_fields(
            LabelInput(
                label=field_name,
                type=field_input_type,
                name=field_name,
                placeholder=field_name,
                required=False,
                value=default_value,
                autocomplete="off",
            ),
        )
    return ui_fields


def _generic_message_form_with_topic_info(
    topic_name: str, topic_type: str, msg_fields: Dict[str, Dict[str, Dict]]
) -> FT:
    ui_fields = _generic_message_form(msg_fields)
    ui_fields(
        Input(name="topic_name", type="hidden", value=topic_name),
        Input(name="topic_type", type="hidden", value=topic_type),
    )
    return ui_fields


# ---- ROS SERVICES/ACTIONS ELEMENTS ----
def _in_service_element(
    srv_name: str, srv_type: str, request_fields: Dict[str, Dict[str, Dict]]
):
    """Creates an Input form for a ROS2 service call

    :param srv_name: Service name
    :type srv_name: str
    :return: Service Form UI element
    :rtype: FT
    """
    service_form = Form(
        cls="space-x-2 space-y-2 m-2",
        id=f"{srv_name}-form",
    )
    if ui_element := _INPUT_ELEMENTS.get(srv_type, None):
        ui_fields = ui_element(srv_name, srv_type)
    else:
        ui_fields = _generic_message_form(request_fields)
    _loading_content = DivHStacked(
        Loading(cls=(LoadingT.spinner, LoadingT.md)), P(" Sending Service Request")
    )
    return service_form(
        ui_fields,
        DivCentered(
            Button(
                "Send Service Call",
                cls="primary-button",
                hx_post="/service/call",
                hx_target="#main",
                hx_on__before_request=f"""
                        this.innerHTML = `{_loading_content}`;
                        this.disabled = true;
                        """,
            )
        ),
        Input(name="srv_name", type="hidden", value=srv_name),
    )


def _in_action_client_element(
    action_name: str, action_type: str, request_fields: Dict[str, Dict[str, Dict]]
):
    """Creates an Input form for a ROS2 Action Server call

    :param srv_name: Action name
    :type srv_name: str
    :return: Service Form UI element
    :rtype: FT
    """
    action_form = Form(
        cls="space-x-2 space-y-2 m-2",
        id=f"{action_name}-form",
    )
    if ui_element := _INPUT_ELEMENTS.get(action_type, None):
        ui_fields = ui_element(action_name, action_type)
    else:
        ui_fields = _generic_message_form(request_fields)
    _loading_content_send = DivHStacked(
        Loading(cls=(LoadingT.spinner, LoadingT.md)), P(" Sending Start Call")
    )
    _loading_content_cancel = DivHStacked(
        Loading(cls=(LoadingT.spinner, LoadingT.md)), P(" Cancelling Ongoing Task")
    )
    _pop_up_content = Div(
        Div(
            H4("Task request Values", cls="cool-subtitle-mini-blue"),
            ui_fields,
            DivHStacked(
                Button(
                    "Send",
                    cls="primary-button",
                    hx_post="/action/goal",
                    hx_target="#main",
                    hx_on__before_request=f"""
                        this.innerHTML = `{_loading_content_send}`;
                        this.disabled = true;
                        """,
                ),
                Button(
                    "Cancel",
                    cls="secondary-button",
                    type="button",
                    onclick="this.closest('.custom-overlay').style.display='none'",
                ),
                cls="space-x-2 justify-center mt-4",
            ),
            cls="modal-box",
            onclick="event.stopPropagation()",
        ),
        cls="custom-overlay",
        id=f"{action_name}-request-from",
        # Logic: Clicking the dark background (the overlay) closes itself
        onclick="this.style.display='none'",
    )
    return action_form(
        _pop_up_content,
        DivCentered(
            DivHStacked(
                Button(
                    "Start",
                    cls="primary-button",
                    id=f"{action_name}-form-btn",
                    onclick=f"document.getElementById('{action_name}-request-from').style.display='grid'",
                ),
                Button(
                    "Cancel",
                    cls="primary-button",
                    hx_post="/action/cancel",
                    hx_target="#main",
                    hx_on__before_request=f"""
                        this.innerHTML = `{_loading_content_cancel}`;
                        this.disabled = true;
                        """,
                ),
            ),
        ),
        Input(name="action_name", type="hidden", value=action_name),
    )


def styled_main_service_clients_container(
    srv_clients_config: Sequence[Dict], container_name: str, column_class: str = ""
) -> FT:
    """Creates a UI element for all service clients

    :param srv_clients_config: Set of service clients configs
    :type srv_clients_config: Sequence[Dict]
    :param column_class: UI columns class
    :type column_class: str, defaults to ""
    :return: Input Service Clients UI element
    """
    # cool-subtitle-mini: automatika red color cool title (in custom.css)
    _id = container_name.lower().replace(" ", "_")
    main_card = Card(
        DivHStacked(
            H4(container_name, cls="cool-subtitle-mini"),
            _toggle_button(div_to_toggle=f"all_{_id}"),
            cls="space-x-0 drag-handle",
        ),
        cls=f"main-card {column_class} max-h-[80vh] overflow-y-auto draggable",
        id=_id,
    )
    all_services_cards = Div(id=f"all_{_id}")
    for client in srv_clients_config:
        client_card = Card(
            H4(client["name"]),
            cls=f"m-2 {column_class} max-h-[40vh] overflow-y-auto inner-main-card",
            id=client["name"],
        )
        client_card(
            _in_service_element(client["name"], client["type"], client["fields"])
        )
        all_services_cards(client_card)
    return main_card(all_services_cards)


# ---- INPUTS CARD ELEMENTS ----
def input_topic_card(
    topic_name: str,
    topic_type: str,
    ros_msg_type: type,
    column_class: str = "",
    ft_has_map_element: bool = False,
) -> FT:
    """Creates a UI element for an input topic

    :param topic_name: Topic name
    :type topic_name: str
    :param topic_type: Topic message type
    :type topic_type: str
    :param column_class: CSS class for number of columns, if not set the Div will span over the whole parent width
    :type column_class: str
    :return: Input topic UI element
    """
    # An inner card (with margin m-2) and max height of 20vh
    card = Card(
        H4(topic_name),
        cls=f"m-2 {column_class} max-h-[20vh] overflow-y-auto inner-main-card",
        id=topic_name,
    )
    if ui_element := _INPUT_ELEMENTS.get(topic_type, None):
        return card(
            ui_element(topic_name, topic_type=topic_type, has_map=ft_has_map_element)
        )
    else:
        # Unknown message type -> return the generic message element
        # Get fields dictionary
        msg_fields = get_ros_msg_fields_dict(ros_msg_type)
        return card(
            _generic_message_form_with_topic_info(
                topic_name=topic_name, topic_type=topic_type, msg_fields=msg_fields
            )
        )


def styled_main_inputs_container(inputs_grid_div_id: str) -> FT:
    """Creates main section for all the UI inputs

    :return: Main inputs card
    :rtype: FT Card
    """
    return Card(
        DivHStacked(
            H4("Inputs", cls="cool-subtitle-mini"),
            _toggle_button(div_to_toggle=inputs_grid_div_id),
            cls="space-x-0 drag-handle",
        ),
        cls="draggable main-card max-h-[40vh] overflow-y-auto",
        body_cls="space-y-0",
    )


def styled_inputs_grid(number_of_inputs: int) -> tuple:
    """Creates a styled grid for the number of inputs

    :param number_of_inputs: Number of input cards
    :type number_of_inputs: int
    :return: Styled Grid, Style class to use for each element in the grid
    :rtype: tuple
    """
    # Create a grid with max 3 inputs per line (1 per line for small views)
    input_grid = Grid(cls="gap-0", cols_lg=min(3, number_of_inputs), cols_sm=1)
    inputs_columns_span = ["col-1"] * number_of_inputs
    # Adjust the column span of the remaining inputs
    if remaining_items := number_of_inputs % 3:
        if remaining_items == 1:
            inputs_columns_span[-1] = "col-span-full"
        elif remaining_items == 2:
            inputs_columns_span[-2:] = "flex flex-col items-center justify-center"
    return (input_grid, inputs_columns_span)


# ---- OUTPUTS CARD ELEMENTS ----
def output_topic_card(topic_name: str, topic_type: str, column_class: str = "", map_output_markers: Optional[Dict] = None) -> FT:
    """Creates a UI element for an output topic

    :param topic_name: Topic name
    :type topic_name: str
    :param topic_type: Topic message type
    :type topic_type: str
    :return: Output topic UI element
    """
    return Card(
        DivHStacked(H4(topic_name), _fullscreen_button(f"card-{topic_name}")),
        _OUTPUT_ELEMENTS[topic_type](topic_name, map_output_markers=map_output_markers),
        cls=f"m-2 {column_class} inner-main-card",
        id=f"card-{topic_name}",
    )


def styled_main_outputs_container(outputs_grid_div_id: str) -> FT:
    """Creates main section for all the UI outputs

    :return: Main outputs card
    :rtype: FT Card
    """
    return Card(
        DivHStacked(
            H4("Outputs", cls="cool-subtitle-mini"),
            _toggle_button(div_to_toggle=outputs_grid_div_id),
            cls="space-x-0 drag-handle",
        ),
        cls="draggable main-card overflow-y-auto max-h-[60vh]",
        body_cls="space-y-0",
    )


def styled_outputs_grid(number_of_outputs: int) -> tuple:
    """Creates a styled grid for the number of outputs

    :param number_of_outputs: Number of output cards
    :type number_of_outputs: int
    :return: Styled Grid, Style class to use for each element in the grid
    :rtype: tuple
    """
    # Create a grid with max 2 outputs per line (1 per line for small views)
    output_grid = Grid(cls="gap-0", cols_lg=min(2, number_of_outputs), cols_sm=1)
    outputs_columns_span = ["col-1"] * number_of_outputs
    if number_of_outputs % 2:
        outputs_columns_span[-1] = "col-span-full"
    return (output_grid, outputs_columns_span)


# ---- OUTPUT LOGGING CARD ELEMENTS ----

LOG_STYLES = {
    "alert": {"prefix": ">>>", "cls": f"{TextT.lg} tomorrow-night-green"},
    "error": {"prefix": ">>> ERROR: ", "cls": f"{TextT.lg} tomorrow-night-red"},
    "warn": {"prefix": ">>> WARNNING: ", "cls": f"{TextT.lg} tomorrow-night-yellow"},
    "user": {
        "prefix": "> User:",
        "cls": f"{TextT.medium} font-bold tomorrow-night-blue",
    },
    "robot": {
        "prefix": "> Robot:",
        "cls": f"{TextT.medium} font-bold tomorrow-night-green",
    },
}
# Default style for "info" or any other unspecified source
DEFAULT_STYLE = {"prefix": ">", "cls": ""}


def _styled_logging_text(text: str, output_src: str = "info", div_id: str = "text"):
    """Builds a styled text log component."""
    container = Div(cls="whitespace-pre-wrap ml-2 p-2 flex items-start", id=div_id)
    style = LOG_STYLES.get(output_src, DEFAULT_STYLE)

    if output_src in ["user", "robot"]:
        prefix_element = Strong(style["prefix"] + " ", cls=style["cls"])
        content_element = Span(prefix_element, f"{text}", id="inner-text")
        container(content_element)
    # All other types have the text inside the main Strong tag
    else:
        full_text = f"{style['prefix']} {text}"
        if output_src in ["error", "warn"]:
            full_text += "!"  # Add the original exclamation mark

        container(Strong(full_text, cls=style["cls"]))

    return container


def _styled_logging_audio(output, output_src: str = "info", div_id: str = "audio"):
    """Builds a styled audio log component."""
    container = DivLAligned(cls="whitespace-pre-wrap ml-2 p-2", id=div_id)
    style = LOG_STYLES.get(output_src, DEFAULT_STYLE)

    audio_element = Audio(
        src=f"data:audio/wav;base64,{output}",
        type="audio/wav",
        controls=True,
        style="border-radius:0.5rem;outline:none;",
    )

    prefix_element = Strong(style["prefix"] + " ", cls=style["cls"])
    container(prefix_element, audio_element)

    return container


def output_logging_card(current_log):
    """Creates a main container for the logging card and adds current logging

    :param current_log: Current logging card
    :type current_log: FT
    :return: Logging card main container
    :rtype: FT
    """
    return Card(
        DivHStacked(
            current_log,
            cls="space-x-0",
        ),
        header=H4("Log", cls="cool-subtitle-mini"),
        header_cls="drag-handle",
        cls="fix-size draggable main-card h-[60vh] max-h-[60vh]",
        id="logging-card-parent",
    )


def initial_logging_card():
    """Creates an empty logging card with 'Log Started' text

    :return: Empty logging card
    :rtype: FT
    """
    output_card = Card(
        cls="terminal-container absolute top-20 inset-x-2 bottom-2 overflow-y-auto",
        id="outputs-log",
    )
    return output_card(_styled_logging_text("Log Started ...", output_src="alert"))


def remove_child_from_logging_card(logging_card, target_id="loading-dots"):
    """Remove the last child in logging_card.children with a matching id."""
    children = logging_card.children

    for i in range(len(children) - 1, -1, -1):
        if getattr(children[i], "id", None) == target_id:
            logging_card.children = children[:i] + children[i + 1 :]
            break


def augment_text_in_logging_card(
    logging_card,
    new_txt: str,
    target_id="text",
):
    """Update the inner text of a child in logging_card.children with a matching id."""
    children = logging_card.children
    target_child = None
    for i in range(len(children) - 1, -1, -1):
        if getattr(children[i], "id", None) == target_id:
            target_child = children[i]
            break
    if not target_child:
        return logging_card
    # Update inner text
    for i in range(len(target_child.children) - 1, -1, -1):
        if getattr(target_child.children[i], "id", None) == "inner-text":
            # Append the new text
            target_child.children[i](Span(f"{new_txt}"))
            return logging_card
    return logging_card


def update_logging_card(
    logging_card, output: str, data_type: str, data_src: str = "info"
):
    """Updates the logging card with a response text

    :param logging_card: Main logging card to update
    :type logging_card: FT
    :param output: Text to add to the card
    :type output: str
    :param data_type: Type of the data (String/error)
    :type data_type: str
    :param data_src: Source of the data (info, error, robot, etc.), defaults to "info"
    :type data_src: str, optional
    :return: Updated logging card
    :rtype: FT
    """
    # Remove any previous loading that exists on the card
    remove_child_from_logging_card(logging_card)
    # Handle errors originating from ROS node
    if data_type == "error":
        data_type = "String"
        data_src = "error"
    return _OUTPUT_ELEMENTS[data_type](logging_card, output, data_src)


def update_logging_card_with_loading(logging_card):
    """Adds a robot 'loading' element to the main logging card.
    Used to add loading to the card until a response is received
    """
    return logging_card(
        Div(
            Strong(
                "> Robot:", cls=f"{TextT.medium} font-bold tomorrow-night-green mr-2"
            ),
            Loading(cls=(LoadingT.dots, LoadingT.md)),
            cls="whitespace-pre-wrap ml-2 p-2 flex items-start",
            id="loading-dots",
            name="loading-dots",
        )
    )


# ---------------------------------------------------------


# ------------- SETTINGS PANEL ELEMENTS -------------------


def settings_ui_element(
    setting_name: str, setting_details: dict, field_type, type_args, input_name=None
):
    """Creates a UI element based on the setting's type and validators

    :param setting_name: Config parameter name
    :type setting_name: str
    :param setting_details: Details of the parsed config
    :type setting_details: dict

    :return: Setting parameter UI element
    """
    try:
        field_type, type_args = parse_type(setting_details.get("type", ""))
    except Exception as e:
        logger.error(
            f"Could not render setting: {setting_name}, with value: {setting_details} due to error: {e}"
        )
        field_type, type_args = None, None
    validators = setting_details.get("validators", [])
    value = setting_details.get("value")

    if not input_name:
        input_name = setting_name

    # Handle validators first
    if validators:
        return validated_config(
            setting_name=setting_name,
            value=value,
            attrs_validators=validators,
            field_type=field_type,
            type_args=type_args,
            input_name=input_name,
        )
    if field_type:
        return nonvalidated_config(
            setting_name=setting_name,
            value=value,
            field_type=field_type,
            type_args=type_args,
            input_name=input_name,
        )


def component_settings_div(
    component_name: str, settings_col_cls: str, ui_elements, nested_ui_elements
):
    """Creates a UI element for a component to show and update the config parameters

    :param component_name: Name of the component (ROS2 node name)
    :type component_name: str
    :param settings_col_cls: UI Div columns span in the display grid
    :type settings_col_cls: str
    :param ui_elements: A set ot UI elements for each parameter in the component config
    :type ui_elements: List
    :return: Component config UI element
    """
    component_div = Card(
        cls=f"p-4 {settings_col_cls} main-card",
    )
    settings_grid = Grid(*ui_elements, cols=4, cls="space-y-3 gap-4 p-4")
    nested_settings_grid = Grid(*nested_ui_elements, cols=1, cls="space-y-3 gap-4 p-4")
    _loading_content = DivHStacked(
        Loading(cls=(LoadingT.spinner, LoadingT.md)), P(" Sending")
    )
    return component_div(
        H3(component_name),
        Form(cls="space-y-4")(
            Input(name="component_name", type="hidden", value=component_name),
            settings_grid,
            nested_settings_grid,
            DivCentered(
                Grid(
                    Button(
                        "Submit",
                        cls="primary-button",
                        hx_post="/settings/submit",
                        hx_target="#main",
                        hx_on__before_request=f"""
                        this.innerHTML = `{_loading_content}`;
                        this.disabled = true;
                        """,
                    ),
                    Button(
                        "Close",
                        cls="secondary-button",
                        hx_get="/",
                        hx_target="#main",
                    ),
                    cols=2,
                    cls="gap-2",
                )
            ),
            id=component_name,
        ),
        DivCentered(id="notification"),
        header=Div(
            CardTitle(component_name),
        ),
    )


def nonvalidated_config(
    setting_name: str, value: Any, field_type: str, type_args, input_name: str
):
    """Sets up a UI element for a component settings element WITHOUT validators

    :param setting_name: Name of the settings field
    :type setting_name: str
    :param value: Value of the settings field
    :type value: Any
    :param field_type: Type of the settings field
    :type field_type: _type_
    :param type_args: Type arguments (if any)
    :type type_args: _type_
    :param input_name: Corresponding input name
    :type input_name: _type_
    :return: UI input element
    :rtype: FT
    """
    if field_type == "bool":
        # The 'checked' attribute is a boolean flag, so it doesn't need a value
        return LabelSwitch(
            label=setting_name,
            id=setting_name,
            checked=int(value),
            name=input_name,
            cls="form-input",
        )

    elif field_type in ["str", "unknown"]:
        return LabelInput(
            label=setting_name,
            id=setting_name,
            type="text",
            value=value,
            name=input_name,
            cls="form-input",
        )

    elif field_type in ["int", "float"]:
        return LabelInput(
            label=setting_name,
            id=setting_name,
            type="number",
            value=str(value),
            name=input_name,
            cls="form-input",
        )

    elif field_type == "literal":
        # Parsing to hanlde enum literals
        parsed_value = value
        if value not in type_args and type(value) is str and value.upper() in type_args:
            parsed_value = value.upper()
        return LabelSelect(
            map(Option, type_args),
            id=setting_name,
            label=setting_name,
            value=parsed_value,
            name=input_name,
            cls="form-input",
        )

    return None


def validated_config(
    setting_name: str,
    value: Any,
    attrs_validators: List[Dict],
    field_type,
    type_args,
    input_name,
):
    """Sets up UI element for a component settings element with validators

    :param setting_name: Name of the settings field
    :type setting_name: str
    :param value: Value of the settings field
    :type value: Any
    :param attrs_validators: Validators of the settings field
    :type attrs_validators: List[Dict]
    :param field_type: Type of the settings field
    :type field_type: _type_
    :param type_args: Type arguments (if any)
    :type type_args: _type_
    :param input_name: Corresponding input name
    :type input_name: _type_
    :return: UI input element
    :rtype: FT
    """
    validator = attrs_validators[0]
    validator_name = list(validator.keys())[0]
    validator_props = validator[validator_name]
    if validator_name in ["in_range", "in_range_discretized"]:
        return LabelInput(
            label=setting_name,
            id=setting_name,
            min=validator_props.get("min_value", 1e-9),
            max=validator_props.get("max_value", 1e9),
            step=validator_props.get("step", 1),
            value=str(value),
            name=input_name,
            cls="form-input",
        )

    elif validator_name == "in":
        options = validator_props.get("ref_value", [])
        return LabelSelect(
            map(Option, options),
            label=setting_name,
            id=setting_name,
            value=value,
            name=input_name,
            cls="form-input",
        )

    elif validator_name == "less_than":
        return LabelInput(
            setting_name,
            id=setting_name,
            type="number",
            max=validator_props.get("ref_value", None),
            value=str(value),
            name=input_name,
            cls="form-input",
        )
    elif validator_name == "greater_than":
        return LabelInput(
            setting_name,
            id=setting_name,
            type="number",
            min=validator_props.get("ref_value", None),
            value=str(value),
            name=input_name,
            cls="form-input",
        )
    return nonvalidated_config(
        setting_name,
        value,
        field_type,
        type_args,
        input_name,
    )


def _parse_nested_settings_dict(
    nested_dict: Dict, all_elements_list: List, nested_root_name: str
):
    """Parse a UI element for nested settings (config) classses

    :param nested_dict: Nested config dictionary
    :type nested_dict: Dict
    :param all_elements_list: List to be populated with nested ui elements
    :type all_elements_list: List
    :param nested_root_name: Root name of the nested config attribute
    :type nested_root_name: str
    """
    for nested_setting_name, nested_setting_details in nested_dict.items():
        field_type, type_args = parse_type(nested_setting_details.get("type", ""))
        if field_type == "BaseAttrs":
            all_elements_list.append(H5(nested_setting_name, cls="col-span-4"))
            value = nested_setting_details.get("value", None)
            if value:
                _parse_nested_settings_dict(
                    nested_setting_details.get("value", {}),
                    all_elements_list,
                    f"{nested_root_name}.{nested_setting_name}",
                )
        else:
            single_div = DivLAligned(
                settings_ui_element(
                    nested_setting_name,
                    nested_setting_details,
                    field_type,
                    type_args,
                    input_name=f"{nested_root_name}.{nested_setting_name}",
                ),
                id=f"{nested_setting_name}",
            )
            all_elements_list.append(single_div)
    return


def parse_ui_elements_to_simple_and_nested(
    component_name: str,
    setting_name: str,
    setting_details: Any,
    simple_ui_elements: list,
    nested_ui_elements: list,
):
    """Parses the component settings element to determine if it is a simple type (int, float, etc.) or  a nested type (for elements that are BaseAttrs classes themselves), then adds the element to the corresponding list

    :param component_name: Name of the component
    :type component_name: str
    :param setting_name: Name of the settings field
    :type setting_name: str
    :param setting_details: Value of the settings field
    :type setting_details: Any
    :param simple_ui_elements: Set of component's simple UI elements to populate
    :type simple_ui_elements: list
    :param nested_ui_elements: Set of component's nested UI elements to populate
    :type nested_ui_elements: list
    """
    field_type, type_args = parse_type(setting_details.get("type", ""))
    nested_root_name = f"{setting_name}"

    if field_type == "BaseAttrs":
        section_id = f"{component_name}-{setting_name}-settings-grid"
        main_nested_container = Card(
            DivLAligned(
                H4(setting_name),
                _toggle_button(div_to_toggle=section_id),
            ),
            cls="p-4 main-card",
        )
        all_elements = Grid(
            id=section_id,
            cols=4,
            cls="space-y-3 gap-4 p-4",
            hidden=True,
            style="display: none;",
        )
        all_elements_list = []
        _parse_nested_settings_dict(
            setting_details.get("value", {}), all_elements_list, nested_root_name
        )
        nested_ui_elements.append(
            main_nested_container(all_elements(*all_elements_list))
        )

    else:
        # simple type: create and add the simple UI element
        simple_ui_elements.append(
            DivLAligned(
                settings_ui_element(
                    setting_name,
                    setting_details,
                    field_type,
                    type_args,
                    nested_root_name,
                ),
            )
        )
