const socket = io();

// ========================================
// ARDUINO Q
// ========================================

const Q_UPLOAD_URL = "http://localhost:5000/upload";

console.log("Socket Connected");

socket.on("connect", () => {
    console.log("Connected to server:", socket.id);
});

const broadcasterBtn = document.getElementById("broadcasterBtn");
const viewerBtn = document.getElementById("viewerBtn");
const toggleBtn = document.getElementById("toggleBtn");

const controls = document.getElementById("controls");
const roleMenu = document.getElementById("roleMenu");

const singleView = document.getElementById("singleView");
const vrView = document.getElementById("vrView");

const singleVideo = document.getElementById("singleVideo");
const leftVideo = document.getElementById("leftVideo");
const rightVideo = document.getElementById("rightVideo");

let stream = null;
let vrMode = false;
let role = "";

let peerConnection = null;

// ========================================
// SEND ONE FRAME TO ARDUINO Q
// ========================================

async function sendFrameToQ() {

    console.log("Preparing frame for Q...");

    if (!stream) {
        console.log("ERROR: Camera stream does not exist.");
        return;
    }

    if (singleVideo.videoWidth === 0 || singleVideo.videoHeight === 0) {
        console.log("ERROR: Video has no dimensions yet.");
        return;
    }

    const canvas = document.createElement("canvas");

    canvas.width = singleVideo.videoWidth;
    canvas.height = singleVideo.videoHeight;

    const ctx = canvas.getContext("2d");

    ctx.drawImage(
        singleVideo,
        0,
        0,
        canvas.width,
        canvas.height
    );

    canvas.toBlob(async (blob) => {

        if (!blob) {
            console.log("ERROR: Could not create JPEG.");
            return;
        }

        console.log(
            "JPEG created:",
            blob.size,
            "bytes"
        );

        const formData = new FormData();

        formData.append(
            "frame",
            blob,
            "frame.jpg"
        );

        try {

            console.log("Sending frame to Q...");

            const response = await fetch(
                Q_UPLOAD_URL,
                {
                    method: "POST",
                    body: formData
                }
            );

            const result = await response.text();

            console.log("Q Response:", result);

        }
        catch (error) {

            console.error(
                "Q Upload Error:",
                error
            );

        }

    }, "image/jpeg", 0.7);
}

// ========================================
// CREATE WEBRTC PEER CONNECTION
// ========================================
function createPeerConnection() {

    peerConnection = new RTCPeerConnection({

        iceServers: [
            {
                urls: "stun:stun.l.google.com:19302"
            }
        ]

    });

console.log("Peer Connection Created");

// Only broadcaster sends video
if (role === "broadcaster") {

    // Add camera tracks
    stream.getTracks().forEach(track => {

        peerConnection.addTrack(track, stream);

    });

    console.log("Camera Tracks Added");

    peerConnection.createOffer()

        .then((offer) => {

            return peerConnection.setLocalDescription(offer);

        })

        .then(() => {

            console.log("Offer Created");

            socket.emit("offer", peerConnection.localDescription);

        });

}

    peerConnection.onicecandidate = (event) => {

    if (!event.candidate)
        return;

    console.log("ICE Candidate Generated");

    socket.emit("ice-candidate", event.candidate);

    };
    peerConnection.ontrack = (event) => {

         console.log("Receiving Video");

        const remoteStream = event.streams[0];

        singleVideo.srcObject = remoteStream;
        leftVideo.srcObject = remoteStream;
        rightVideo.srcObject = remoteStream;

        singleVideo.onloadedmetadata = () => {
             console.log("Single Video Ready");
            singleVideo.play();
    };

    leftVideo.onloadedmetadata = () => {
        leftVideo.play();
    };

    rightVideo.onloadedmetadata = () => {
        rightVideo.play();
    };

};

    peerConnection.onconnectionstatechange = () => {

        console.log("Connection State:", peerConnection.connectionState);

    };

}

// Hide everything except role menu
controls.style.display = "none";
singleView.style.display = "none";
vrView.style.display = "none";

// ========================================
// BROADCASTER
// ========================================
broadcasterBtn.onclick = async () => {

    role = "broadcaster";

    socket.emit("broadcaster");

    roleMenu.style.display = "none";
    controls.style.display = "block";
    singleView.style.display = "block";

    try {

        stream = await navigator.mediaDevices.getUserMedia({

            video: true,
            audio: false

        });

        singleVideo.srcObject = stream;
        leftVideo.srcObject = stream;
        rightVideo.srcObject = stream;

        // Wait 2 seconds for the camera to fully load
setTimeout(() => {
    
    // setInterval runs the code inside it repeatedly
    setInterval(() => {
        sendFrameToQ();
    }, 500); // 500 milliseconds = 2 frames per second

}, 2000);


    }
    catch (err) {

        alert("Camera Error:\n" + err.message);
        console.error(err);

    }

};

// ========================================
// VIEWER
// ========================================
viewerBtn.onclick = () => {

    role = "viewer";

    socket.emit("viewer");

    createPeerConnection();

    roleMenu.style.display = "none";
    controls.style.display = "block";
    singleView.style.display = "block";

    alert("Viewer Mode\n\nWaiting for Broadcaster...");

};

// ========================================
// VR TOGGLE
// ========================================
toggleBtn.onclick = () => {

    vrMode = !vrMode;

    if (vrMode) {

        singleView.style.display = "none";
        vrView.style.display = "flex";

        toggleBtn.innerText = "Normal Mode";

    }
    else {

        singleView.style.display = "block";
        vrView.style.display = "none";

        toggleBtn.innerText = "Switch to VR";

    }

};

// ========================================
// SOCKET EVENTS
// ========================================
socket.on("broadcaster", () => {

    console.log("Broadcaster is online.");

});

socket.on("viewer", () => {

    console.log("VIEWER EVENT RECEIVED");
    console.log("Current role =", role);

    if (role !== "broadcaster") {
        console.log("Ignored because role is not broadcaster");
        return;
    }

    console.log("Viewer Joined");
    createPeerConnection();

});

socket.on("offer", async (offer) => {

    console.log("Offer Received");

    if (role !== "viewer")
        return;

    await peerConnection.setRemoteDescription(offer);

    console.log("Remote Description Set");

    const answer = await peerConnection.createAnswer();

    await peerConnection.setLocalDescription(answer);

    console.log("Answer Created");

    socket.emit("answer", peerConnection.localDescription);

});
socket.on("answer", async (answer) => {

    if (role !== "broadcaster")
        return;

    console.log("Answer Received");

    await peerConnection.setRemoteDescription(answer);

    console.log("Handshake Complete");
    
});
// ========================================
// RECEIVE ICE CANDIDATES
// ========================================
socket.on("ice-candidate", async (candidate) => {

    if (!peerConnection)
        return;

    console.log("ICE Candidate Received");

    try {

        await peerConnection.addIceCandidate(candidate);

        console.log("ICE Candidate Added");

    }
    catch (err) {

        console.error("ICE Error:", err);

    }

});
const sendFrameBtn = document.getElementById("sendFrameBtn");
const captureCanvas = document.getElementById("captureCanvas");
const qResponse = document.getElementById("qResponse");

sendFrameBtn.addEventListener("click", async () => {
    try {
        // Change "video" to "singleVideo" here
        if (!singleVideo.videoWidth || !singleVideo.videoHeight) {
            qResponse.textContent = "Camera frame is not ready.";
            return;
        }

        // Match canvas size to the actual video frame.
        captureCanvas.width = singleVideo.videoWidth;
        captureCanvas.height = singleVideo.videoHeight;

        const ctx = captureCanvas.getContext("2d");

        // Change "video" to "singleVideo" here too
        ctx.drawImage(
            singleVideo,
            0,
            0,
            captureCanvas.width,
            captureCanvas.height
        );
        
        // ... (keep the rest of your blob and fetch code the same)

        // Convert the canvas frame to JPEG.
        const blob = await new Promise(resolve => {
            captureCanvas.toBlob(resolve, "image/jpeg", 0.8);
        });

        if (!blob) {
            throw new Error("Could not create JPEG image.");
        }

        // Prepare HTTP multipart upload.
        const formData = new FormData();
        formData.append("image", blob, "webcam_frame.jpg");

        qResponse.textContent = "Sending frame to Q...";

        const response = await fetch(Q_UPLOAD_URL, {
            method: "POST",
            body: formData
        });

        const result = await response.json();

        qResponse.textContent =
            JSON.stringify(result, null, 2);

    } catch (error) {
        console.error(error);
        qResponse.textContent =
            "Error: " + error.message;
    }
});

async function sendFrameToQ() {

    console.log("Preparing frame for Q...");

    if (!stream) {
        console.log("ERROR: Camera stream does not exist.");
        return;
    }

    if (singleVideo.videoWidth === 0 || singleVideo.videoHeight === 0) {
        console.log("ERROR: Video has no dimensions yet.");
        return;
    }

    const canvas = document.createElement("canvas");

    canvas.width = singleVideo.videoWidth;
    canvas.height = singleVideo.videoHeight;

    const ctx = canvas.getContext("2d");

    ctx.drawImage(
        singleVideo,
        0,
        0,
        canvas.width,
        canvas.height
    );

    canvas.toBlob(async (blob) => {

        if (!blob) {
            console.log("ERROR: Could not create JPEG.");
            return;
        }

        console.log(
            "JPEG created:",
            blob.size,
            "bytes"
        );

        const formData = new FormData();

        formData.append(
            "frame",
            blob,
            "frame.jpg"
        );

        try {

            console.log("Sending frame to Q...");

            const response = await fetch(Q_UPLOAD_URL, {
                method: "POST",
                body: formData
            });

            const result = await response.text();

            console.log("Q Response:", result);

        }
        catch (error) {

            console.error(
                "Q Upload Error:",
                error
            );

        }

    }, "image/jpeg", 0.7);
}
