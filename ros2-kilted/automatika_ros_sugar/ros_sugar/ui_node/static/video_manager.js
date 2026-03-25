/**
 * video.js
 * Handles WebSocket connections for dynamic video/image streams (<img> elements).
 */

document.addEventListener("DOMContentLoaded", () => {
    const videoWsProtocol = window.location.protocol === "https:" ? "wss:" : "ws:";
    const MAX_RETRIES = 10;
    const wsConnections = new Map(); // id -> { ws, timer }

    // Create connection for a given <img> element
    function connectImageWebSocket(img, wsUrl, attempt = 0) {
        const ws = new WebSocket(wsUrl);
        ws.binaryType = "arraybuffer";

        ws.onopen = () => {
            console.log(`[${img.id}] WebSocket connected`);
            attempt = 0;
        };

        ws.onmessage = (event) => {
            try {
                const data = JSON.parse(event.data);
                if (data.payload) {
                    img.src = "data:image/jpeg;base64," + data.payload;
                    return;
                }
            } catch {
                // Not JSON / not base64 JSON; continue to check binary
            }

            if (event.data instanceof ArrayBuffer) {
                const bytes = new Uint8Array(event.data);
                let binary = '';
                bytes.forEach(b => binary += String.fromCharCode(b));
                img.src = "data:image/jpeg;base64," + btoa(binary);
            }
        };

        ws.onerror = (err) => {
            console.error(`[${img.id}] WebSocket error:`, err);
        };

        ws.onclose = () => {
            console.warn(`⚠️ [${img.id}] WebSocket closed`);
            wsConnections.delete(img.id);

            if (attempt < MAX_RETRIES) {
                const delay = Math.min(1000 * Math.pow(2, attempt), 30000);
                console.log(`[${img.id}] Reconnecting in ${delay / 1000}s...`);
                const timer = setTimeout(() => {
                    // Only reconnect if the <img> is still present in DOM
                    const stillThere = document.getElementById(img.id);
                    if (stillThere && stillThere.getAttribute("name") === "video-frame") {
                        const conn = connectImageWebSocket(stillThere, wsUrl, attempt + 1);
                        wsConnections.set(img.id, conn);
                    } else {
                        console.log(`[${img.id}] Element no longer present; aborting reconnect.`);
                    }
                }, delay);
                wsConnections.set(img.id, { ws: null, timer });
            } else {
                console.error(`[${img.id}] Max reconnect attempts reached`);
            }
        };

        return { ws };
    }

    // Start connections for all current frames (only if not already tracked)
    function ensureConnectionsForPresentFrames() {
        const frames = Array.from(document.getElementsByName("video-frame"));
        const presentIds = new Set(frames.map(f => f.id));

        // Add missing connections
        frames.forEach((img) => {
            if (!img.id) return;
            if (!wsConnections.has(img.id)) {
                const wsUrl = `${videoWsProtocol}//${window.location.host}/ws_${img.id}`;
                const conn = connectImageWebSocket(img, wsUrl, 0);
                wsConnections.set(img.id, conn);
            }
        });

        // Remove connections whose elements are gone
        Array.from(wsConnections.keys()).forEach((id) => {
            if (!presentIds.has(id)) {
                const entry = wsConnections.get(id);
                if (entry) {
                    try {
                        if (entry.timer) clearTimeout(entry.timer);
                        if (entry.ws) {
                            entry.ws.onclose = null;
                            entry.ws.close();
                        }
                    } catch (e) { }
                }
                wsConnections.delete(id);
                console.log(`[${id}] Connection removed because element is no longer present`);
            }
        });
    }

    // Close all connections (used on hide/unload)
    function closeAllConnections() {
        wsConnections.forEach((entry, _) => {
            try {
                if (entry.timer) clearTimeout(entry.timer);
                if (entry.ws) {
                    entry.ws.onclose = null;
                    entry.ws.close();
                }
            } catch (e) { }
        });
        wsConnections.clear();
    }

    // MutationObserver: watch for DOM changes to keep connections in sync
    let moTimer = null;
    const mo = new MutationObserver(() => {
        if (moTimer) clearTimeout(moTimer);
        moTimer = setTimeout(() => {
            ensureConnectionsForPresentFrames();
        }, 100); // 100ms debounce
    });
    mo.observe(document.body, { childList: true, subtree: true, attributes: false });

    // Visibility and bfcache handling
    document.addEventListener("visibilitychange", () => {
        if (document.visibilityState === "visible") {
            console.log("Page visible, ensuring streams...");
            ensureConnectionsForPresentFrames();
        } else {
            console.log("Page hidden, closing streams...");
            closeAllConnections();
        }
    });

    window.addEventListener("pageshow", (event) => {
        if (event.persisted) {
            console.log("Page restored from cache, ensuring streams...");
            ensureConnectionsForPresentFrames();
        }
    });

    window.addEventListener("pagehide", () => {
        closeAllConnections();
    });

    // Initial pass
    ensureConnectionsForPresentFrames();
});
