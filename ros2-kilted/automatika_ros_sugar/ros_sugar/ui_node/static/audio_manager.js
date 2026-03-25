/**
 * audio_manager.js
 * Handles Audio WebSocket connections, recording, and processing (WAV conversion).
 */

// Establish WebSocket connection for audio transmission
const audioWsProtocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
const ws_stream = new WebSocket(`${audioWsProtocol}//${window.location.host}/ws_audio`);

ws_stream.onopen = () => {
    console.log("Audio Websocket connection established");
};

ws_stream.onclose = () => {
    console.log("Audio Websocket connection closed");
};

ws_stream.onerror = (err) => {
    console.error(`Audio WebSocket error:`, err);
};

// --- Audio Recording Logic ---

let mediaRecorder;
let audioChunks = [];
let isRecording = false;
let recordingIndicatorEl = null;

// Reusable AudioContext
const audioContext = new (window.AudioContext || window.webkitAudioContext)();

async function startAudioRecording(button) {
    const uk_icon_recording = `<uk-icon icon="mic"><svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class=""><path d="M12 2a3 3 0 0 0-3 3v7a3 3 0 0 0 6 0V5a3 3 0 0 0-3-3Z"></path><path d="M19 10v2a7 7 0 0 1-14 0v-2"></path><line x1="12" x2="12" y1="19" y2="22"></line></svg></uk-icon>`;
    const uk_icon_stop = `<uk-icon icon="square"><svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class=""><rect width="18" height="18" x="3" y="3" rx="2"></rect></svg></uk-icon>`;

    button.classList.toggle("recording");

    if (isRecording) {
        if (mediaRecorder) mediaRecorder.stop();
        button.innerHTML = uk_icon_recording;
    } else {
        try {
            const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
            mediaRecorder = new MediaRecorder(stream);
            audioChunks = [];

            mediaRecorder.onstart = () => {
                isRecording = true;
                button.innerHTML = uk_icon_stop;
                button.title = 'End Recording';
            };

            mediaRecorder.ondataavailable = (event) => {
                if (event.data.size > 0) {
                    audioChunks.push(event.data);
                }
            };

            mediaRecorder.onstop = async () => {
                isRecording = false;

                if (audioChunks.length === 0) {
                    if (recordingIndicatorEl) recordingIndicatorEl.remove();
                    console.error("No Audio recorded.");
                    if (typeof addErrorMessage === "function") addErrorMessage("No audio was recorded. Please try again.");
                    return;
                }

                const audioBlob = new Blob(audioChunks, { type: "audio/wav" });

                // Process the audio to the correct format - 16000 Hz Mono
                try {
                    const resampledBlob = await processAudio(audioBlob, 16000);

                    const reader = new FileReader();
                    reader.readAsDataURL(resampledBlob);
                    reader.onloadend = () => {
                        const base64Audio = reader.result.split(",")[1];
                        if (base64Audio) {
                            // topic_name matches the button ID used to trigger recording
                            ws_stream.send(JSON.stringify({ type: "audio", payload: base64Audio, topic_name: button.id }));
                        }
                    };
                } catch (error) {
                    console.error("Failed to process audio:", error);
                    if (recordingIndicatorEl) recordingIndicatorEl.remove();
                }

                button.innerHTML = uk_icon_recording;
                button.title = "Record";
            };

            mediaRecorder.start();
        } catch (error) {
            console.error("Error accessing microphone:", error);
        }
    }
};

async function processAudio(audioBlob, targetSampleRate) {
    const arrayBuffer = await audioBlob.arrayBuffer();
    const originalAudioBuffer = await audioContext.decodeAudioData(arrayBuffer);

    const numberOfChannels = originalAudioBuffer.numberOfChannels;
    const originalSampleRate = originalAudioBuffer.sampleRate;

    if (originalSampleRate === targetSampleRate && numberOfChannels === 1) {
        return audioBlob;
    }

    const duration = originalAudioBuffer.duration;
    const offlineContext = new OfflineAudioContext(1, duration * targetSampleRate, targetSampleRate);

    const source = offlineContext.createBufferSource();
    source.buffer = originalAudioBuffer;
    source.connect(offlineContext.destination);
    source.start(0);

    const resampledAudioBuffer = await offlineContext.startRendering();
    return bufferToWav(resampledAudioBuffer);
}

function bufferToWav(buffer) {
    const numOfChan = buffer.numberOfChannels;
    const length = buffer.length * numOfChan * 2 + 44;
    const bufferArr = new ArrayBuffer(length);
    const view = new DataView(bufferArr);
    const channels = [];
    let i;
    let sample;
    let offset = 0;
    let pos = 0;

    // WAV header construction
    setUint32(0x46464952); // "RIFF"
    setUint32(length - 8); // file length - 8
    setUint32(0x45564157); // "WAVE"
    setUint32(0x20746d66); // "fmt " chunk
    setUint32(16); // length of fmt data
    setUint16(1); // PCM
    setUint16(numOfChan);
    setUint32(buffer.sampleRate);
    setUint32(buffer.sampleRate * 2 * numOfChan);
    setUint16(numOfChan * 2);
    setUint16(16);
    setUint32(0x61746164); // "data"
    setUint32(length - pos - 4);

    for (i = 0; i < buffer.numberOfChannels; i++) {
        channels.push(buffer.getChannelData(i));
    }

    while (pos < length) {
        for (i = 0; i < numOfChan; i++) {
            sample = Math.max(-1, Math.min(1, channels[i][offset]));
            sample = (0.5 + sample < 0 ? sample * 32768 : sample * 32767) | 0;
            view.setInt16(pos, sample, true);
            pos += 2;
        }
        offset++;
    }

    return new Blob([view], { type: "audio/wav" });

    function setUint16(data) {
        view.setUint16(pos, data, true);
        pos += 2;
    }
    function setUint32(data) {
        view.setUint32(pos, data, true);
        pos += 4;
    }
}
