const express = require("express");
const http = require("http");
const { Server } = require("socket.io");

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Log every HTTP request
app.use((req, res, next) => {
    console.log("Request:", req.method, req.url);
    next();
});

// Serve files from /public
app.use(express.static("public"));

let broadcaster = null;

// ======================
// Socket.IO
// ======================

io.on("connection", (socket) => {

    console.log("Client Connected:", socket.id);

    socket.on("broadcaster", () => {

        console.log(socket.id + " became Broadcaster");

        socket.broadcast.emit("broadcaster");

    });

    socket.on("viewer", () => {

        console.log(socket.id + " became Viewer");

        socket.broadcast.emit("viewer");

    });

    // -----------------------------
    // OFFER
    // -----------------------------
    socket.on("offer", (offer) => {

        console.log("Offer received.");

        socket.broadcast.emit("offer", offer);

    });

    socket.on("disconnect", () => {

        console.log("Client Disconnected:", socket.id);

    });
    // -----------------------------
    // ANSWER
    // -----------------------------
    socket.on("answer", (answer) => {

        console.log("Answer received.");

        socket.broadcast.emit("answer", answer);

    });
    // -----------------------------
    // ICE CANDIDATES
    // -----------------------------
    socket.on("ice-candidate", (candidate) => {

        console.log("ICE Candidate received.");

        socket.broadcast.emit("ice-candidate", candidate);

    });
    

});

server.listen(3000, "0.0.0.0", () => {

    console.log("=================================");
    console.log("VERONICA SERVER RUNNING");
    console.log("http://localhost:3000");
    console.log("=================================");

});
