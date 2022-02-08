const express = require("express");
const child_process = require("child_process")
// const fs = require("fs");
const PORT = process.env.PORT || 8000;

const app=express();

app.use(express.static("public"));

app.get("/video", async (req, res)=>{
    res.contentType('image/jpeg');

    let proces = child_process.spawn("ffmpeg", ["-f", "x11grab", '-i', ':99', "-q:v", "10","-preset", "veryfast" ,'-f', 'mjpeg', "-vframes", "1", '-']);
    proces.stdout.pipe(res);
});
app.listen(PORT, ()=>console.log("server listen on "+PORT))