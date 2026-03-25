/**
 * ros_maps.js
 * - Handles Map Visualization with ROS2DJS.
 */

// State to track if we are in "Publish Point" mode
const mapInteractionState = {}; // { topicName: { isPublishing: boolean, btn: HTMLElement } }


document.addEventListener("DOMContentLoaded", () => {
    const mapElements = document.getElementsByName('map-canvas');
    if (mapElements.length === 0) return;

    mapElements.forEach((container) => {
        initSingleMap(container);
    });
});

// --- EXPORTED FUNCTIONS (Called by Python Buttons) ---
window.zoomMap = function (topicName, zoomFactor) {
    const container = document.getElementById(topicName);
    if (!container || !container.mapViewer) return;

    const viewer = container.mapViewer;
    applyZoom(viewer, zoomFactor, null); // Null center means zoom to center of view
};

window.togglePublishPoint = function (btn) {
    const mapElements = document.getElementsByName('map-canvas');
    if (mapElements.length === 0) {
        if (typeof UIkit !== 'undefined') {
            UIkit.notification({
                message: "<span uk-icon='icon: warning'></span> No available input map elements found.",
                status: 'danger',
                pos: 'top-center',
                timeout: 5000
            });
        } else {
            console.warn("No map elements found.");
        }
        return;
    }

    // Determine if we are turning ON or OFF based on button state
    const isActive = btn.classList.contains('active-brand-red');
    const isTurningOn = !isActive;

    // --- Click Outside Handler ---
    // We define this handler to detect clicks outside the map/button
    const handleOutsideClick = (event) => {
        // Check if click was inside ANY map container
        const isClickOnMap = Array.from(mapElements).some(el => el.contains(event.target));
        const isClickOnBtn = btn.contains(event.target);

        // If click is outside map AND outside the toggle button, turn off
        if (!isClickOnMap && !isClickOnBtn) {
            window.togglePublishPoint(btn);
        }
    };
    // ----------------------------------------

    // Update Button UI
    if (isTurningOn) {
        btn.classList.remove('uk-button-default', 'bg-white/80', 'dark:bg-gray-800/80', 'backdrop-blur');
        btn.classList.add('active-brand-red');

        // Add global listener for clicking outside (Timeout prevents immediate trigger from the current click)
        setTimeout(() => {
            document.addEventListener('click', handleOutsideClick);
            // Store reference on button so we can remove it later
            btn._outsideClickHandler = handleOutsideClick;
        }, 10);

    } else {
        btn.classList.remove('active-brand-red');
        btn.classList.add('uk-button-default', 'bg-white/80', 'dark:bg-gray-800/80', 'backdrop-blur');

        // Clean up global listener
        if (btn._outsideClickHandler) {
            document.removeEventListener('click', btn._outsideClickHandler);
            btn._outsideClickHandler = null;
        }
    }

    // Apply to ALL Maps
    mapElements.forEach(container => {
        const topicName = container.id;

        if (!mapInteractionState[topicName]) {
            mapInteractionState[topicName] = { isPublishing: false, btn: null };
        }
        const state = mapInteractionState[topicName];

        state.isPublishing = isTurningOn;
        state.btn = btn;

        // Force cursor on the Canvas specifically
        // because setupInteractions modifies the canvas style directly.
        if (container.mapViewer && container.mapViewer.scene.canvas) {
            container.mapViewer.scene.canvas.style.cursor = isTurningOn ? 'crosshair' : 'default';
        }
        container.style.cursor = isTurningOn ? 'crosshair' : 'default';

        // Determine Settings if Turning On
        if (isTurningOn) {
            if (btn.id === `${topicName}-publish-btn`) {
                const settingsForm = document.getElementById(`${topicName}-settings-form`);
                let targetTopic = 'clicked_point';
                let msgType = 'PointStamped';

                if (state.settings) {
                    targetTopic = state.settings.topic;
                    msgType = state.settings.type;
                } else if (settingsForm) {
                    const tInput = settingsForm.querySelector('[name="clicked_point_topic"]');
                    let mInput = settingsForm.querySelector('input[name="clicked_point_type"]');
                    if (!mInput) mInput = settingsForm.querySelector('select[name="clicked_point_type"]');

                    if (tInput && tInput.value) targetTopic = tInput.value;
                    if (mInput && mInput.value) msgType = mInput.value;
                    state.settings = { topic: targetTopic, type: msgType };
                }
                state[btn.id] = { topic: targetTopic, type: msgType };
            } else {
                const customTopic = btn.getAttribute('data-topic') || 'clicked_point';
                const customType = btn.getAttribute('data-type') || 'PointStamped';
                state[btn.id] = { topic: customTopic, type: customType };
            }
        }
    });
};

window.openMapSettings = function (topicName) {
    const modal = document.getElementById(`${topicName}-settings-modal`);
    if (!modal) return;

    // Show Modal
    modal.style.display = 'grid';

    // Restore Saved Values (if they exist)
    const state = mapInteractionState[topicName];
    if (state && state.settings) {
        const form = document.getElementById(`${topicName}-settings-form`);
        if (form) {
            // A. Restore Text Input
            const tInput = form.querySelector('input[name="clicked_point_topic"]');
            if (tInput) tInput.value = state.settings.topic;

            // B. Restore Dropdown (Complex Component)
            // We must find the HIDDEN NATIVE SELECT inside the custom component to update the UI
            // Selector: Find the uk-select with this name, then find the native select inside it
            const selectContainer = form.querySelector(`uk-select[name="clicked_point_type"]`);
            if (selectContainer) {
                const nativeSelect = selectContainer.querySelector('select');
                if (nativeSelect) {
                    nativeSelect.value = state.settings.type;
                    // Dispatch change event so the Custom UI updates its text
                    nativeSelect.dispatchEvent(new Event('change', { bubbles: true }));
                }
            } else {
                // Fallback: try finding any select with that name
                const simpleSelect = form.querySelector(`select[name="clicked_point_type"]`);
                if (simpleSelect) simpleSelect.value = state.settings.type;
            }
        }
    }
};

window.saveMapSettings = function (topicName) {
    const modal = document.getElementById(`${topicName}-settings-modal`);
    const form = document.getElementById(`${topicName}-settings-form`);

    if (form) {
        // Initialize State
        if (!mapInteractionState[topicName]) mapInteractionState[topicName] = {};

        // Save Publishing Settings
        const pubTopic = form.querySelector('[name="clicked_point_topic"]').value;

        // Handle Custom Select for Message Type
        let typeValue = 'PointStamped';
        const mInput = form.querySelector('input[name="clicked_point_type"]');
        if (mInput) {
            typeValue = mInput.value;
        } else {
            const sInput = form.querySelector('select[name="clicked_point_type"]');
            if (sInput) typeValue = sInput.value;
        }

        mapInteractionState[topicName].settings = { topic: pubTopic, type: typeValue };

        // Save Visual Settings
        if (!mapInteractionState[topicName].visuals) mapInteractionState[topicName].visuals = {};

        // Get the Wrapper
        const wrapper = document.getElementById(`visual-selector-${topicName}`);

        // Get the Native Select inside the wrapper
        const nativeSelect = wrapper ? wrapper.querySelector('select') : null;

        if (nativeSelect && nativeSelect.options) {
            const formData = new FormData(form);

            Array.from(nativeSelect.options).forEach(opt => {
                const oid = opt.value;
                const visualConfig = {};

                // We use 'width' to detect if it's a path because ranges always have a value.
                // 'style' might be an empty string if unselected, which evaluates to false in if(style).
                const width = formData.get(`width_${oid}`);
                const style = formData.get(`style_${oid}`);
                const color = formData.get(`color_${oid}`);

                // Determine Type based on which fields exist in the form data
                // formData.get returns null if the field doesn't exist for that ID
                if (width !== null) {
                    // It's a Path
                    visualConfig.type = 'path';
                    visualConfig.style = style || 'solid'; // Default to solid if empty
                    visualConfig.width = width;
                    visualConfig.color = color;
                } else {
                    // It's a Point/Overlay
                    visualConfig.type = 'overlay';
                    visualConfig.color = color;
                }

                if (visualConfig.type) {
                    mapInteractionState[topicName].visuals[oid] = visualConfig;
                }
            });
        }

        // Force Redraw
        const container = document.getElementById(topicName);
        if (container && container.overlays) {
            Object.values(container.overlays).forEach(m => m.graphics.clear());
        }
        if (container && container.paths) {
            Object.values(container.paths).forEach(p => p.graphics.clear());
        }
    }

    if (modal) modal.style.display = 'none';
};


/**
 * Toggles the visibility of markers setting blocks in the map settings modal.
 * * @param {string} mapId - The ID of the map (e.g. 'map_1')
 */
window.updateVisualSettingsVisibility = function (target, mapId) {
    // RETRIEVE VALUE
    // Since 'target' is the custom <uk-select> wrapper (LabelSelect), we look inside it.
    // We try the hidden input first (most reliable), then fallback to direct .value
    let selectedId;

    if (!target) return;

    const hiddenInput = target.querySelector('input[name="selected_visual_id"]');
    if (hiddenInput) {
        selectedId = hiddenInput.value;
    } else {
        // Fallback: Try getting .value directly from the custom element
        selectedId = target.value;
    }


    if (!selectedId) return;

    // Scope the query to the specific modal if possible, or use the unique map ID classes
    const blocks = document.querySelectorAll(`.visual-settings-block-${mapId}`);

    blocks.forEach(el => {
        const targetId = `visuals-${mapId}-${selectedId}`;

        if (el.id === targetId) {
            el.classList.remove('hidden');
            el.style.display = 'block';
        } else {
            el.classList.add('hidden');
            el.style.display = 'none';
        }
    });
};


/**
 * Sets up a watcher on the selector to trigger updates automatically.
 * Call this ONCE when the modal opens.
 */
window.initVisualSettingsObserver = function (mapId) {
    const selector = document.getElementById(`visual-selector-${mapId}`);
    if (!selector || selector._hasObserver) return; // Prevent double binding

    // Find the source of truth (the hidden input)
    const hiddenInput = selector.querySelector('input[name="selected_visual_id"]');
    if (!hiddenInput) return;

    // Create an observer instance
    const observer = new MutationObserver((mutations) => {
        mutations.forEach((mutation) => {
            if (mutation.type === 'attributes' && mutation.attributeName === 'value') {
                // Value changed! Trigger the update.
                updateVisualSettingsVisibility(selector, mapId);
            }
        });
    });

    // Start observing the hidden input for 'value' attribute changes
    observer.observe(hiddenInput, { attributes: true, attributeFilter: ['value'] });

    // Mark as observed so we don't attach multiple times
    selector._hasObserver = true;
};

function initSingleMap(container) {
    const topicName = container.id;
    if (!topicName) return;

    // --- Setup Viewer ---
    const viewer = new ROS2D.Viewer({
        divID: topicName,
        width: 1, height: 1,
        background: '#7f7f7f'
    });

    viewer.scene.enableMouseOver(10); // Check for hovers 10 times per second

    container.mapViewer = viewer;

    const canvas = viewer.scene.canvas;
    canvas.style.display = 'block';
    const ctx = canvas.getContext("2d");
    if (ctx) ctx.imageSmoothingEnabled = false;

    // --- Setup Mock ROS ---
    const mockRos = new EventEmitter2();
    mockRos.callOnConnection = () => { };
    mockRos.send = () => { };
    mockRos.connect = () => { };
    mockRos.close = () => { };
    mockRos.isConnected = true;

    // --- Setup Layers to allow adding markers on the map ---
    const mapGroup = new createjs.Container(); // The main wrapper
    container.mapGroup = mapGroup;
    viewer.scene.addChild(mapGroup);

    // Layer 1: The Map (Bottom)
    const mapLayer = new createjs.Container();
    mapGroup.addChild(mapLayer);

    // Layer 2: The Overlays (Top)
    // Anything added here will ALWAYS be drawn on top of the map
    const overlayLayer = new createjs.Container();
    container.overlayLayer = overlayLayer;
    mapGroup.addChild(overlayLayer);


    // --- Setup Grid Client ---
    const gridClient = new ROS2D.OccupancyGridClient({
        ros: mockRos,
        rootObject: mapLayer, // RENDER MAP INTO THE BOTTOM LAYER
        topic: topicName,
        continuous: false
    });
    container.mapGridClient = gridClient;

    gridClient.on('change', () => {
        requestAnimationFrame(() => resizeMap(container));
    });

    const protocol = window.location.protocol === "https:" ? "wss:" : "ws:";
    const wsUrl = `${protocol}//${window.location.host}/ws_${topicName}`;
    const ws = new WebSocket(wsUrl);
    ws.binaryType = 'arraybuffer';
    container.mapWs = ws;
    container.topicName = topicName;

    ws.onmessage = (event) => {
        try {
            let msgData;
            if (event.data instanceof ArrayBuffer) {
                const decoder = new TextDecoder("utf-8");
                msgData = JSON.parse(decoder.decode(event.data));
            } else {
                msgData = JSON.parse(event.data);
            }
            let mapMessage = msgData.msg ? msgData.msg : msgData;

            if (mapMessage.info && mapMessage.header) {
                container.mapInfo = mapMessage.info;
                container.mapHeader = mapMessage.header;
            }

            if (mapMessage.data && typeof mapMessage.data === 'string') {
                const binaryString = atob(mapMessage.data);
                const len = binaryString.length;
                const bytes = new Int8Array(len);
                for (let i = 0; i < len; i++) {
                    bytes[i] = binaryString.charCodeAt(i);
                }
                mapMessage.data = bytes;
                mockRos.emit(topicName, mapMessage);
            }
            // --- Handle Points ---
            else if (msgData.op === 'overlay') {
                if (container.mapHeader && (msgData.frame_id != "") && (container.mapHeader.frame_id != msgData.frame_id)) {
                    // If the map frame_id exists and the point frame_id exists and they are not the same
                    // -> Skip point display
                    return;
                }
                else {
                    updateMapOverlay(container, msgData.id, {
                        x: msgData.x,
                        y: msgData.y,
                        rotation: msgData.theta
                    }, 'red', 'arrow');

                    container.mapViewer.scene.update();
                }
            }
            // --- Handle Path ---
            else if (msgData.op === 'path') {
                if (container.mapHeader && (msgData.frame_id != "") && (container.mapHeader.frame_id != msgData.frame_id)) {
                    // If the map frame_id exists and the point frame_id exists and they are not the same
                    // -> Skip point display
                    return;
                }
                else {
                    updateMapPath(container, msgData.id, msgData.points);
                    container.mapViewer.scene.update();
                }
            }
        } catch (e) { console.error(e); }
    };

    setupInteractions(viewer, container);
    const resizeObserver = new ResizeObserver(() => resizeMap(container));
    resizeObserver.observe(container);
}


/**
 * Helper: Safely retrieves the visual settings object for a specific ID on a specific Map.
 */
function getVisualSettings(mapId, objectId) {
    if (mapId && mapInteractionState[mapId] && mapInteractionState[mapId].visuals) {
        return mapInteractionState[mapId].visuals[objectId];
    }
    return null;
}

/**
 * Helper: Gets the color for a map marker object from the saved map settings
 * If the objectId is not found in the settings -> Gets a random color
 */
function getColorForId(mapId, objectId) {
    const settings = getVisualSettings(mapId, objectId);
    // If settings exist, check if we should use the fixed color
    if (settings && settings.color) {
        return settings.color;
    }

    // Fallback: Generate consistent Auto Color from ID
    const palette = [
        '#E83F3F', '#2ECC71', '#3498DB', '#F1C40F', '#9B59B6',
        '#E67E22', '#1ABC9C', '#FF00FF', '#BFFF00', '#FF69B4'
    ];

    let hash = 0;
    for (let i = 0; i < objectId.length; i++) {
        hash = objectId.charCodeAt(i) + ((hash << 5) - hash);
    }
    const index = Math.abs(hash) % palette.length;
    return palette[index];
}


/**
 * Helper: Gets the line style for a path marker on the map
 * Returns: { width: number, dash: Array|null }
 */
function getPathStyleForId(mapId, objectId) {
    const settings = getVisualSettings(mapId, objectId);

    // Default Values
    let width = 3;
    let dash = null; // Solid line

    if (settings) {
        if (settings.width) width = parseInt(settings.width, 10);

        if (settings.style === 'dashed') dash = [10, 5]; // 10px line, 5px gap
        else if (settings.style === 'dots') dash = [2, 4]; // 2px dot, 4px gap
    }

    return { width, dash };
}



/**
 * Updates or Creates a dynamic marker on the map.
 * @param {string} id - Unique ID for this object (e.g., 'robot_pose')
 * @param {object} pose - {x, y, rotation} (Rotation in degrees, optional)
 * @param {string} type - 'circle' or 'arrow'
 */
function updateMapOverlay(container, id, pose, _ignoredColor, type = 'arrow') {
    // Safety Checks
    if (!container.mapGridClient || !container.mapGridClient.currentGrid || !container.overlayLayer) return;

    // Transform Coordinates
    const pixelCoords = transformRosToGridPixels(container, pose.x, pose.y);
    if (!pixelCoords) return;

    // Calculate Proportional Size
    const info = container.mapInfo;
    let sizePx = 25;

    if (info) {
        const resolution = info.resolution;
        const mapWidthMeters = info.width * resolution;
        const targetSizeMeters = Math.max(mapWidthMeters * 0.02, 0.6);
        sizePx = targetSizeMeters / resolution;
    }

    // --- COLOR & HEADING ---
    // We use container.id (the map topic) to look up the settings
    const assignedColor = getColorForId(container.id, id);

    let showHeading = false;
    let rotationDegrees = 0;

    // Check heading (0 to 2PI):
    // Other values of heading (minus) will be used for point objects i.e. no heading
    if (type === 'arrow' && pose.rotation !== undefined && typeof pose.rotation === 'number') {
        showHeading = true;
        rotationDegrees = pose.rotation * (180 / Math.PI);
    }

    // Get or Create Marker
    if (!container.overlays) container.overlays = {};
    let marker = container.overlays[id];

    if (!marker) {
        marker = new createjs.Shape();
        container.overlayLayer.addChild(marker);
        container.overlays[id] = marker;

        // --- TOOLTIP LOGIC ---
        marker.cursor = "pointer";
        marker.addEventListener("mouseover", (evt) => {
            const tip = document.createElement('div');
            tip.id = `tooltip-${id}`;
            tip.className = 'uk-tooltip uk-active';

            // Get latest pose stored on the marker (prevents stale data)
            const currentPose = marker._rosPose || { x: 0, y: 0 };

            // Format HTML: ID on line 1, Coords on line 2 (small & semi-transparent)
            tip.innerHTML = `
                <div>${id}</div>
                <div style="font-size: 0.85em; opacity: 0.8; font-family: monospace; margin-top: 2px;">
                    (${currentPose.x.toFixed(3)}, ${currentPose.y.toFixed(3)})
                </div>
            `;

            // Styling
            tip.style.position = 'fixed';
            tip.style.zIndex = 10000;
            tip.style.pointerEvents = 'none';
            tip.style.transform = 'translate(-50%, -150%)';
            tip.style.border = `1px solid ${assignedColor}`;
            tip.style.color = assignedColor;
            tip.style.textAlign = 'center'; // Center align the text

            if (evt.nativeEvent) {
                tip.style.left = `${evt.nativeEvent.clientX}px`;
                tip.style.top = `${evt.nativeEvent.clientY}px`;
            }
            document.body.appendChild(tip);
            marker._tooltipElement = tip;
        });

        marker.addEventListener("mousemove", (evt) => {
            if (marker._tooltipElement && evt.nativeEvent) {
                marker._tooltipElement.style.left = `${evt.nativeEvent.clientX}px`;
                marker._tooltipElement.style.top = `${evt.nativeEvent.clientY}px`;
            }
        });

        marker.addEventListener("mouseout", () => {
            if (marker._tooltipElement) {
                marker._tooltipElement.remove();
                marker._tooltipElement = null;
            }
        });
    }

    // We save the pose to the marker object so the tooltip always sees the CURRENT location
    marker._rosPose = pose;

    // Redraw Custom Graphics
    marker.graphics.clear();

    const dotRadius = sizePx * 0.2;

    if (showHeading) {
        // Full Arrow
        const gap = sizePx * 0.15;
        const arrowStart = dotRadius + gap;
        const arrowLength = sizePx * 0.6;
        const arrowWidth = sizePx * 0.5;

        marker.graphics
            .beginFill(assignedColor)
            .drawCircle(0, 0, dotRadius)
            .endStroke();

        marker.graphics
            .beginFill(assignedColor)
            .moveTo(arrowStart, -arrowWidth / 2)
            .lineTo(arrowStart + arrowLength, 0)
            .lineTo(arrowStart, arrowWidth / 2)
            .lineTo(arrowStart + (arrowLength * 0.25), 0)
            .closePath();

        marker.shadow = new createjs.Shadow("rgba(0,0,0,0.4)", 1, 1, 3);
        // APPLY ROTATION
        marker.rotation = -rotationDegrees;

    } else {
        // Point Only
        marker.graphics
            .beginFill(assignedColor)
            .drawCircle(0, 0, dotRadius)
            .endStroke();

        marker.shadow = new createjs.Shadow("rgba(0,0,0,0.2)", 1, 1, 2);
        marker.rotation = 0;
    }

    // Update Position
    marker.x = pixelCoords.x;
    marker.y = pixelCoords.y;

    marker.visible = true;
}


/**
 * Helper: Renders a path (trajectory) on the map efficiently.
 * Expects 'points' to be a flat array [x1, y1, x2, y2, ...]
 */
function updateMapPath(container, id, points) {
    // Safety Checks
    if (!container.mapGridClient || !container.mapGridClient.currentGrid || !container.overlayLayer) return;
    if (!points || points.length < 4) return; // Need at least 2 points (4 coords) to draw a line

    // --- FETCH STYLES ---
    const assignedColor = getColorForId(container.id, id);
    const styleSettings = getPathStyleForId(container.id, id)

    // Get or Create Path Shape
    if (!container.paths) container.paths = {};
    let pathShape = container.paths[id];

    if (!pathShape) {
        pathShape = new createjs.Shape();

        // Add to overlay layer (Draw paths below markers?)
        // If you want paths BELOW the robot marker, use 'addChildAt(pathShape, 0)'
        container.overlayLayer.addChildAt(pathShape, 0);
        container.paths[id] = pathShape;
    }

    // Clear old graphics
    pathShape.graphics.clear();

    // Calculate Stroke Width (Meters -> Pixels)
    const info = container.mapInfo;
    let strokeWidthPx = 2;
    if (info) {
        // Map user setting (1-10) to physical size (e.g. 5cm to 50cm)
        // Default (3) becomes ~15cm wide
        const baseMeters = styleSettings.width * 0.05;
        strokeWidthPx = Math.max(baseMeters / info.resolution, 1);
    }

    // Apply Styles
    pathShape.graphics.setStrokeStyle(strokeWidthPx, "round", "round");

    // Apply Dash Pattern
    if (styleSettings.dash) {
        // Scale dashes by stroke width for consistent look
        const scaledDash = styleSettings.dash.map(x => x * strokeWidthPx);
        pathShape.graphics.setStrokeDash(scaledDash);
    } else {
        pathShape.graphics.setStrokeDash(null);
    }

    // Start Drawing
    pathShape.graphics.beginStroke(assignedColor);


    // Loop through points and convert to Grid Pixels

    // Transform First Point
    let p0 = transformRosToGridPixels(container, points[0], points[1]);
    if (!p0) return; // Map info likely missing

    pathShape.graphics.moveTo(p0.x, p0.y);

    for (let i = 2; i < points.length; i += 2) {
        const rosX = points[i];
        const rosY = points[i + 1];

        // calling the helper is cleaner and usually fast enough for <1000 points.
        const px = transformRosToGridPixels(container, rosX, rosY);

        if (px) {
            pathShape.graphics.lineTo(px.x, px.y);
        }
    }

    pathShape.graphics.endStroke();

    pathShape.visible = true;
}


/**
 * Coordinate Transformation Logic
 * Uses EaselJS built-in matrix math to convert Screen -> Grid Pixels -> ROS
 */
function transformScreenToRos(container, screenX, screenY) {
    const gridClient = container.mapGridClient;
    const grid = gridClient.currentGrid;

    if (!grid) return null;

    // Convert Screen (Logical) to Canvas (Physical) coordinates
    // We must match the physical resolution used in resizeViewer
    const dpr = window.devicePixelRatio || 1;
    const physX = screenX * dpr;
    const physY = screenY * dpr;

    // Use EaselJS to transform from Canvas space -> Local Grid Image space
    // This automatically handles the Viewer Zoom, Scene Pan, Grid Rotation,
    // Grid Centering offsets (x/y), and Registration Points (regX/regY).
    const localPt = grid.globalToLocal(physX, physY);

    // localPt.x and localPt.y are now the pixel coordinates on the original map image
    const imageX = localPt.x;
    const imageY = localPt.y;

    // Convert Grid Pixels -> ROS Coordinates
    const info = container.mapInfo;

    if (!info) {
        console.warn("[Map] Metadata missing.");
        return null;
    }

    const res = info.resolution;
    const originX = info.origin.position.x;
    const originY = info.origin.position.y;
    const mapHeight = info.height;

    // ROS Map Standard: (0,0) is bottom-left. Image: (0,0) is top-left.
    // Invert Y axis to match ROS convention
    const rosX = (imageX * res) + originX;
    const rosY = ((mapHeight - imageY) * res) + originY;

    return { x: rosX, y: rosY, z: 0.0 };
}

/**
 * Transforms Real-World ROS coordinates (Meters) -> Map Image Pixels
 * Used to place overlays (robots, paths) on the map.
 */
function transformRosToGridPixels(container, rosX, rosY) {
    const info = container.mapInfo;
    if (!info) return null;

    const res = info.resolution;
    const originX = info.origin.position.x;
    const originY = info.origin.position.y;
    const mapHeight = info.height; // Height in pixels

    // Calculate X (Standard: Left to Right)
    const x = (rosX - originX) / res;

    // Calculate Y (Inverted: Map Bottom is Image Bottom)
    // ROS Origin (ymin) corresponds to Image Bottom (y = height)
    // ROS Max (ymax) corresponds to Image Top (y = 0)
    const y = mapHeight - ((rosY - originY) / res);

    return { x: x, y: y };
}

/**
 * Apply Zoom helper
 * @param {Object} center - {x, y} in screen coordinates (optional)
 */
function applyZoom(viewer, factor, center) {
    const scene = viewer.scene;

    const newScale = scene.scaleX * factor;

    if (center) {
        // Zoom towards mouse point
        const localX = (center.x - scene.x) / scene.scaleX;
        const localY = (center.y - scene.y) / scene.scaleY;

        scene.scaleX = newScale;
        scene.scaleY = newScale;

        scene.x = center.x - localX * newScale;
        scene.y = center.y - localY * newScale;
    } else {
        // Zoom towards center of screen
        const centerX = viewer.width / 2;
        const centerY = viewer.height / 2;

        const localX = (centerX - scene.x) / scene.scaleX;
        const localY = (centerY - scene.y) / scene.scaleY;

        scene.scaleX = newScale;
        scene.scaleY = newScale;

        scene.x = centerX - localX * newScale;
        scene.y = centerY - localY * newScale;
    }
}

/**
 * Helper method to publish a clicked point on a map canvas
 */
function publishPoint(container, targetTopic, rosPoint, msgType) {
    if (!container || !container.mapWs) {
        console.warn("Cannot publish point: Websocket or Container missing");
        return;
    }

    // Default Orientation
    const defaultOrientation = { ori_x: 0.0, ori_y: 0.0, ori_z: 0.0, ori_w: 1.0 };
    let messageData = {};

    // Safely get frame_id (Default to 'map' if header is missing)
    const frameId = (container.mapHeader && container.mapHeader.frame_id) ? container.mapHeader.frame_id : 'map';

    if (msgType === 'Point' || msgType === 'PointStamped') {
        messageData = rosPoint; // {x, y, z}
    } else if (msgType === 'Pose' || msgType === 'PoseStamped') {
        messageData = { ...rosPoint, ...defaultOrientation };
    }

    const payload = {
        topic_name: targetTopic,
        frame_id: frameId,
        topic_type: msgType,
        data: messageData
    };

    container.mapWs.send(JSON.stringify(payload));
    console.log(`Published [${msgType}] to [${targetTopic}]`, payload);
};


function setupInteractions(viewer, container) {
    const canvas = viewer.scene.canvas;
    if (!canvas) return;

    let isDragging = false;
    let lastX, lastY;

    // --- MOUSE DOWN (No Changes needed here) ---
    canvas.addEventListener('mousedown', (event) => {
        let topicName = container.topicName;
        const state = mapInteractionState[topicName];

        // ... PUBLISH POINT LOGIC ...
        if (state && state.isPublishing && event.button === 0) {
            const rect = canvas.getBoundingClientRect();
            const mouseX = event.clientX - rect.left;
            const mouseY = event.clientY - rect.top;
            const rosPoint = transformScreenToRos(container, mouseX, mouseY);

            if (rosPoint) {
                let targetTopic = 'clicked_point';
                let msgType = 'PointStamped';
                if (state.btn && state[state.btn.id]) {
                    targetTopic = state[state.btn.id].topic;
                    msgType = state[state.btn.id].type;
                }
                publishPoint(container, targetTopic, rosPoint, msgType);
            }
            window.togglePublishPoint(state.btn);
            return;
        }

        // --- PAN LOGIC ---
        if (event.button === 0 || event.button === 1) {
            isDragging = true;
            lastX = event.clientX;
            lastY = event.clientY;
            canvas.style.cursor = 'grabbing';
            event.preventDefault();
        }
    });

    // --- MOUSE MOVE (No Changes needed here) ---
    window.addEventListener('mousemove', (event) => {
        if (!isDragging) return;
        const dpr = window.devicePixelRatio || 1;
        const dx = (event.clientX - lastX) * dpr;
        const dy = (event.clientY - lastY) * dpr;
        viewer.scene.x += dx;
        viewer.scene.y += dy;
        lastX = event.clientX;
        lastY = event.clientY;
    });

    // --- MOUSE UP (UPDATED) ---
    window.addEventListener('mouseup', () => {
        if (isDragging) {
            isDragging = false;

            // Check if we are in publishing mode before resetting cursor
            let topicName = container.topicName;
            const state = mapInteractionState[topicName];

            if (state && state.isPublishing) {
                canvas.style.cursor = 'crosshair';
            } else {
                canvas.style.cursor = 'default';
            }
        }
    });

    // --- ZOOM (No Changes needed here) ---
    canvas.addEventListener('wheel', (event) => {
        const isFullscreen = container.closest('.fullscreen-overlay') !== null;
        if (!isFullscreen) return;
        event.preventDefault();
        event.stopPropagation();
        const zoomFactor = event.deltaY < 0 ? 1.1 : 0.9;
        const rect = canvas.getBoundingClientRect();
        const dpr = window.devicePixelRatio || 1;
        const mouseX = (event.clientX - rect.left) * dpr;
        const mouseY = (event.clientY - rect.top) * dpr;
        applyZoom(viewer, zoomFactor, { x: mouseX, y: mouseY });
    }, { passive: false });
}


/**
 * Helper to manually resize the ROS2D Viewer canvas
 */
function resizeViewer(viewer, logicalWidth, logicalHeight) {
    const canvas = viewer.scene.canvas;
    const dpr = window.devicePixelRatio || 1;

    // Set the "Physical" size (buffer size)
    const physicalWidth = Math.floor(logicalWidth * dpr);
    const physicalHeight = Math.floor(logicalHeight * dpr);

    canvas.width = physicalWidth;
    canvas.height = physicalHeight;
    viewer.width = physicalWidth;
    viewer.height = physicalHeight;

    // Force the "Logical" size via CSS (how big it looks on screen)
    canvas.style.width = `${logicalWidth}px`;
    canvas.style.height = `${logicalHeight}px`;

    // Ensure smoothing is disabled for 2D maps after resize
    const ctx = canvas.getContext("2d");
    if (ctx) ctx.imageSmoothingEnabled = false;
}

function resizeMap(container) {
    if (!container || !container.mapViewer) return;

    const viewer = container.mapViewer;
    const gridClient = container.mapGridClient;
    const mapGroup = container.mapGroup;
    const mapInfo = container.mapInfo; // USE SAVED METADATA
    const MAP_ROTATION = -90;

    setTimeout(() => {
        const logicalWidth = container.clientWidth;
        const logicalHeight = container.clientHeight;
        if (logicalWidth === 0 || logicalHeight === 0) return;

        resizeViewer(viewer, logicalWidth, logicalHeight);

        // We need BOTH the grid client and the map metadata to align correctly
        if (gridClient && gridClient.currentGrid && mapInfo) {
            const grid = gridClient.currentGrid;

            // FORCE GRID TO 0,0
            // This ensures the Map Image top-left is exactly at mapLayer's 0,0
            grid.x = 0;
            grid.y = 0;
            grid.rotation = 0;
            grid.scaleX = 1;
            grid.scaleY = 1;

            // Use METADATA dimensions for centering, not Image dimensions
            // (Image dimensions can sometimes be unreliable depending on load state)
            const mapWidth = mapInfo.width;
            const mapHeight = mapInfo.height;

            // Align mapGroup Center
            mapGroup.regX = mapWidth / 2;
            mapGroup.regY = mapHeight / 2;
            mapGroup.rotation = MAP_ROTATION;
            mapGroup.x = mapWidth / 2;
            mapGroup.y = mapHeight / 2;

            // Zoom Logic (Using group bounds)
            const bounds = mapGroup.getTransformedBounds();
            if (!bounds) return;

            const isFullscreen = container.classList.contains('fullscreen-overlay');
            const canvasPhysWidth = viewer.width;
            const canvasPhysHeight = viewer.height;

            let zoom;
            if (isFullscreen) {
                zoom = Math.min(canvasPhysWidth / bounds.width, canvasPhysHeight / bounds.height);
            } else {
                // Calculate height to fit aspect ratio
                const mapAspectRatio = bounds.width / bounds.height;
                const newLogicalHeight = logicalWidth / mapAspectRatio;
                container.style.height = `${newLogicalHeight}px`;
                resizeViewer(viewer, logicalWidth, newLogicalHeight);

                // Recalculate physical width after resize
                zoom = viewer.width / bounds.width;
            }

            // Apply Zoom & Pan
            viewer.scene.scaleX = zoom;
            viewer.scene.scaleY = zoom;
            viewer.scene.x = (viewer.width - (bounds.width * zoom)) / 2 - (bounds.x * zoom);
            viewer.scene.y = (viewer.height - (bounds.height * zoom)) / 2 - (bounds.y * zoom);
        }
    }, 50);
}
