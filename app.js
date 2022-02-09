const express = require("express");
const child_process = require("child_process")
const http = require("http");
const {Blob} = require("buffer")
const websocket = require("ws").Server;
const { createReadStream } = require("fs");

const PORT = process.env.PORT || 8000;

const app=express();

const server = http.createServer(app);
const wss = new websocket({server})

app.use(express.static("public"));

function tackaimage(){
    return new Promise((resolve, reject)=>{
        let _buf = []
        let proces = child_process.spawn("ffmpeg", ["-f", "x11grab", '-i', ':99', "-q:v", "10","-preset", "veryfast" ,'-f', 'mjpeg', "-vframes", "1", '-']);
        proces.stdout.on("data", (chunk)=>_buf.push(chunk));
        proces.stdout.on("end", ()=>resolve(Buffer.concat(_buf)));
        proces.stderr.setEncoding('utf8');
        proces.stderr.on("error", (error)=>reject(error));
    })
}
let socketCliendList = [];
function sendImage(){
    if(socketCliendList.length > 0)
    {
        tackaimage()
        .then(data=>{
            for(let i=0; i < socketCliendList.length; i++)
            {
                socketCliendList[i].send(data);
            }
        })
        console.log(Date.now());
    }
}
wss.on("connection", (wsc)=>{
    socketCliendList.push(wsc);
    wsc.on("close", ()=>{
        console.log("close connection");
    })
});

setInterval(sendImage, 100);






app.get("/video", async (req, res)=>{
    res.contentType('image/jpeg');
    let proces = child_process.spawn("ffmpeg", ["-f", "x11grab", '-i', ':99', "-q:v", "10","-preset", "veryfast" ,'-f', 'mjpeg', "-vframes", "1", '-']);
    proces.stdout.pipe(res);
});
app.listen(PORT, ()=>console.log("server listen on "+PORT))
