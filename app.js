const express = require("express");
const child_process = require("child_process");

const PORT = process.env.PORT || 8000;
const app=express();

app.get("/video", async (req, res)=>{
    res.contentType('image/jpeg');
    let proces = child_process.spawn("ffmpeg", ["-f", "x11grab", '-i', ':99', "-q:v", "10","-preset", "veryfast" ,'-f', 'mjpeg', "-vframes", "1", '-']);
    proces.stdout.pipe(res);
});

app.listen(PORT, ()=>console.log("server listen on " + PORT))
