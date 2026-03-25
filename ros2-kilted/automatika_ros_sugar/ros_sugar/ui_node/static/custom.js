/**
 * custom.js
 * Handles general UI interactions, Drag & Drop, Resize, HTMX configs, and logging.
 */


// Helper to persist forms
function persistForm(form) {
    if (typeof FormPersistence !== "undefined") {
        FormPersistence.persist(form);
    }
}

// Toggle Theme Button
function toggleTheme() {
    // Default to dark theme
    document.documentElement.classList.toggle('light');
    const html = document.documentElement;
    if (html.classList.contains('dark')) {
        html.classList.remove('dark');
        localStorage.setItem('theme', 'light');
    } else {
        html.classList.add('dark');
        localStorage.setItem('theme', 'dark');
    }
}

// Initial theme check
(function () {
    const saved = localStorage.getItem('theme');
    if (saved === 'dark' || (!saved && window.matchMedia('(prefers-color-scheme: dark)').matches)) {
        document.documentElement.classList.add('dark');
    }
})()


// Toggle Fullscreen Mode
function toggleFullScreen(btn, containerId) {
    const container = document.getElementById(containerId);
    if (!container) return;

    // Toggle the class
    container.classList.toggle('fullscreen-overlay');

    // Toggle icon
    const icon = btn.querySelector('[uk-icon]') || btn.querySelector('svg');
    if (icon && icon.hasAttribute('icon')) {
        const current = icon.getAttribute('icon');
        icon.setAttribute('icon', current === 'expand' ? 'shrink' : 'expand');
    }

    // --- RESIZE MAP IF PRESENT ---
    // Check if this container has a map-canvas inside
    const mapContainer = container.querySelector('[name="map-canvas"]');
    if (mapContainer && typeof resizeMap === 'function') {
        resizeMap(mapContainer);
    }
}

function openAtButton(btnId, dialogId) {
    const btn = document.getElementById(btnId);
    const dialog = document.getElementById(dialogId);

    if (btn && dialog) {
        // Get button position
        const rect = btn.getBoundingClientRect();

        // Position dialog next to button
        dialog.style.top = `${rect.top - 50}px`;
        dialog.style.left = `${rect.right + 10}px`;

        dialog.showModal();
    }
}

document.addEventListener("DOMContentLoaded", () => {
    // 1. Logging Auto-scroll
    const log_parent = document.getElementById("logging-card-parent");
    if (log_parent) {
        const observer = new MutationObserver(() => {
            const log = document.getElementById("outputs-log");
            if (log) log.scrollTop = log.scrollHeight;
        });
        observer.observe(log_parent, { childList: true, subtree: true });
    }

    // 2. Actions WebSocket (Frontend <-> Backend general comms)
    const actions = document.getElementById("actions-frontend");
    if (actions) {
        const wsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const ws_actions = new WebSocket(`${wsProtocol}//${window.location.host}/ws_actions`);

        ws_actions.onopen = () => {
            console.log("Actions Websocket connection established");
        };
    }

    // 3. Initialize Draggable and Resizable cards
    makeDraggable("draggable");
    makeResizable("draggable");
});

// Fix parsing boolean values from UI Switch elements for HTMX
document.addEventListener("htmx:configRequest", function (event) {
    const form = event.detail.elt.closest("form");
    if (!form) return;

    // Find all checkboxes or FastHTML Switch elements
    form.querySelectorAll("input[type='checkbox'], .Switch input").forEach(el => {
        const name = el.name;
        if (!name) return;
        // Always include the key, even if unchecked
        event.detail.parameters[name] = el.checked ? '1' : '0';
    });
});

// --- Draggable Logic ---
function makeDraggable(className = "draggable") {
    const draggables = document.querySelectorAll(`.${className}`);
    let draggedItem = null;
    let placeholder = null;
    let offsetX = 0;
    let offsetY = 0;
    let siblings = [];
    const placeholderClass = "drag-placeholder";

    draggables.forEach(item => {
        const handle = item.querySelector('.drag-handle');
        const target = handle || item;

        if (handle) {
            handle.style.cursor = "grab";
        }

        target.addEventListener('mousedown', (e) => startDrag(e, item));
        target.addEventListener('touchstart', (e) => startDrag(e, item), { passive: false });
    });

    function startDrag(e, innerItem) {
        if (["INPUT", "BUTTON", "SELECT", "TEXTAREA"].includes(e.target.tagName)) return;
        if (e.target.classList.contains('resize-handle')) return;
        if (e.cancelable) e.preventDefault();

        // Global Selection Block
        document.body.style.userSelect = 'none';

        const wrapper = innerItem.parentNode;
        draggedItem = wrapper;
        const grid = wrapper.parentNode;

        const clientX = e.clientX || e.touches[0].clientX;
        const clientY = e.clientY || e.touches[0].clientY;

        const wrapperRect = wrapper.getBoundingClientRect();
        const innerRect = innerItem.getBoundingClientRect();

        offsetX = clientX - wrapperRect.left;
        offsetY = clientY - wrapperRect.top;

        // Create Placeholder
        placeholder = document.createElement('div');
        placeholder.className = innerItem.className;
        placeholder.classList.add(placeholderClass);
        placeholder.classList.remove(className);
        placeholder.removeAttribute('id');

        // --- Match Masonry Layout ---
        // We use 1px rows, and set explicit height.
        placeholder.style.height = `${innerRect.height}px`;
        // We must mimic the margin-bottom gap used in masonry
        const computedStyle = window.getComputedStyle(grid);
        placeholder.style.marginBottom = computedStyle.columnGap || '0px';

        placeholder.style.flex = "0 0 auto";
        placeholder.style.backgroundColor = "rgba(0,0,0,0.05)";
        placeholder.style.borderColor = "rgba(100,100,100,0.3)";
        placeholder.style.borderStyle = "dashed";
        placeholder.style.borderWidth = "0.2rem";
        placeholder.style.borderRadius = "0.8rem";

        // Insert placeholder
        grid.insertBefore(placeholder, wrapper);

        // Float Wrapper
        draggedItem.style.position = 'fixed';
        draggedItem.style.zIndex = '9999';
        draggedItem.style.width = `${innerRect.width}px`;
        draggedItem.style.height = `${innerRect.height}px`;
        draggedItem.style.pointerEvents = 'none';
        draggedItem.style.left = `${wrapperRect.left}px`;
        draggedItem.style.top = `${wrapperRect.top}px`;
        draggedItem.style.opacity = '0.9';

        if (innerItem.querySelector('.drag-handle')) {
            innerItem.querySelector('.drag-handle').style.cursor = "grabbing";
        }

        siblings = Array.from(grid.children).filter(child =>
            child !== wrapper &&
            child !== placeholder &&
            child.tagName !== 'SCRIPT'
        );

        document.addEventListener('mousemove', onDragMove);
        document.addEventListener('mouseup', onDragEnd, { old_item: innerItem });
        document.addEventListener('touchmove', onDragMove, { passive: false });
        document.addEventListener('touchend', onDragEnd, { old_item: innerItem });
    }

    function onDragMove(e) {
        if (!draggedItem) return;
        if (e.type === 'touchmove') e.preventDefault();

        const clientX = e.clientX || e.touches[0].clientX;
        const clientY = e.clientY || e.touches[0].clientY;

        // 1. Move the floating wrapper
        draggedItem.style.left = `${clientX - offsetX}px`;
        draggedItem.style.top = `${clientY - offsetY}px`;

        // 2. Find Closest and Swap (Standard Sortable Logic)
        let closest = null;
        let minDistance = Infinity;

        siblings.forEach(target => {
            if (target.style.display === 'none') return;

            const rect = target.getBoundingClientRect();
            const targetCenterX = rect.left + rect.width / 2;
            const targetCenterY = rect.top + rect.height / 2;

            const dist = Math.pow(clientX - targetCenterX, 2) + Math.pow(clientY - targetCenterY, 2);
            if (dist < minDistance) {
                minDistance = dist;
                closest = target;
            }
        });

        if (closest) {
            const rect = closest.getBoundingClientRect();
            const targetCenterX = rect.left + rect.width / 2;

            if (clientX < targetCenterX) {
                if (placeholder.nextElementSibling !== closest) {
                    closest.parentNode.insertBefore(placeholder, closest);
                }
            } else {
                if (placeholder.previousElementSibling !== closest) {
                    closest.parentNode.insertBefore(placeholder, closest.nextSibling);
                }
            }
        }
    }

    function onDragEnd(_eventObj) {
        if (!draggedItem || !placeholder) return;

        document.body.style.userSelect = '';

        placeholder.parentNode.insertBefore(draggedItem, placeholder);
        placeholder.remove();
        placeholder = null;

        draggedItem.style.position = '';
        draggedItem.style.zIndex = '';
        draggedItem.style.width = '';
        draggedItem.style.height = '';
        draggedItem.style.left = '';
        draggedItem.style.top = '';
        draggedItem.style.pointerEvents = '';
        draggedItem.style.opacity = '';

        const handle = draggedItem.querySelector('.drag-handle');
        if (handle) handle.style.cursor = "grab";

        draggedItem = null;
        siblings = [];

        // Update layout to ensure items pack densely
        updateMasonryLayout(className);

        document.removeEventListener('mousemove', onDragMove);
        document.removeEventListener('mouseup', onDragEnd);
        document.removeEventListener('touchmove', onDragMove);
        document.removeEventListener('touchend', onDragEnd);
    }
}

// --- Resizable Logic (Smart Grid Update + Masonry) ---
function makeResizable(className = "draggable") {
    const items = document.querySelectorAll(`.${className}`);

    items.forEach(item => {
        if (!item.querySelector('.resize-handle')) {
            const resize_handler = document.createElement('div');
            resize_handler.classList.add('resize-handle');
            item.appendChild(resize_handler);

            if (getComputedStyle(item).position === 'static') {
                item.style.position = 'relative';
            }

            resize_handler.addEventListener('mousedown', (e) => initResize(e, item));
            resize_handler.addEventListener('touchstart', (e) => initResize(e, item), { passive: false });
        }
    });

    // Initial Layout Calculation
    setTimeout(() => updateMasonryLayout(className), 100);

    let startX, startY, startWidth, startHeight;
    let currentItem = null;

    function initResize(e, item) {
        e.stopPropagation();
        if (e.type === 'touchstart') e.preventDefault();

        currentItem = item;
        startX = e.clientX || e.touches[0].clientX;
        startY = e.clientY || e.touches[0].clientY;
        startWidth = item.offsetWidth;
        startHeight = item.offsetHeight;

        document.body.style.userSelect = 'none';
        document.body.style.cursor = 'nwse-resize';

        document.addEventListener('mousemove', doResize);
        document.addEventListener('mouseup', stopResize);
        document.addEventListener('touchmove', doResize, { passive: false });
        document.addEventListener('touchend', stopResize);
    }

    function doResize(e) {
        if (!currentItem) return;

        const currentX = e.clientX || e.touches[0].clientX;
        const currentY = e.clientY || e.touches[0].clientY;

        const newWidth = startWidth + (currentX - startX);
        const newHeight = startHeight + (currentY - startY);

        if (newWidth > 150) {
            currentItem.style.width = `${newWidth}px`;
            currentItem.style.maxWidth = 'none';
        }

        if (newHeight > 100) {
            currentItem.style.height = `${newHeight}px`;
            currentItem.style.maxHeight = 'none';
        }
    }

    function stopResize() {
        if (currentItem) {
            // --- SMART GRID UPDATE ---
            const wrapper = currentItem.parentNode;
            const grid = wrapper.parentNode;

            if (grid && wrapper) {
                const gridWidth = grid.offsetWidth;
                const itemWidth = currentItem.offsetWidth;

                if (itemWidth < (gridWidth * 0.6)) {
                    wrapper.classList.remove('col-span-full');
                    wrapper.classList.remove('col-span-2');
                    currentItem.style.maxWidth = '100%';
                } else {
                    wrapper.classList.add('col-span-full');
                }
            }

            // Recalculate Masonry Spans
            updateMasonryLayout(className);
        }

        currentItem = null;
        document.body.style.userSelect = '';
        document.body.style.cursor = '';

        document.removeEventListener('mousemove', doResize);
        document.removeEventListener('mouseup', stopResize);
        document.removeEventListener('touchmove', doResize);
        document.removeEventListener('touchend', stopResize);
    }
}

// --- HELPER: Masonry Layout Logic ---
// Enforces 1px grid rows and dense flow to allow items to fill vertical gaps
function updateMasonryLayout(className = "draggable") {
    const draggables = document.querySelectorAll(`.${className}`);
    if (!draggables.length) return;

    const grids = new Set();
    draggables.forEach(d => {
        if (d.parentNode && d.parentNode.parentNode) {
            grids.add(d.parentNode.parentNode);
        }
    });

    grids.forEach(grid => {
        // 1. Enable Dense Grid
        grid.style.gridAutoRows = '1px';
        grid.style.gridAutoFlow = 'dense';
        grid.style.rowGap = '0px'; // Disable native row gap (we use margin)

        // Use column gap as the standard gap size
        const colGap = window.getComputedStyle(grid).columnGap || '0px';
        const gapVal = parseFloat(colGap) || 0;

        // 2. Update all Wrappers
        const wrappers = Array.from(grid.children).filter(el =>
            el.tagName !== 'SCRIPT' &&
            !el.classList.contains('drag-placeholder') &&
            el.style.position !== 'fixed' // Ignore dragging item
        );

        wrappers.forEach(wrapper => {
            const inner = wrapper.querySelector(`.${className}`);
            if (!inner) return;

            // Apply margin to simulate gap
            wrapper.style.marginBottom = colGap;

            // Calculate Grid Span
            // span = height + margin (since row unit is 1px)
            const h = inner.getBoundingClientRect().height;
            const span = Math.ceil(h + gapVal);

            wrapper.style.gridRowEnd = `span ${span}`;
        });
    });
}
