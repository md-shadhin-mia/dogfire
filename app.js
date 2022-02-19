const express = require("express");
const child_process = require("child_process");

const PORT = process.env.PORT || 8000;

//require takepng takepng.c compiled x11 and png
function takeSnap(x =0 , y = 0)
{
    return new Promise((resolve, reject)=>{
        let _buf = []
        let proces = child_process.spawn('./takepng', ['-o', '-', '-r', '3']);
        // let proces = child_process.spawn("ffmpeg", ['-f', 'x11grab', '-i', `${process.env.DISPLAY}`,'-vframes','1', '-f', 'mjpeg', '-']);
        proces.stdout.on("data", (chunk)=>_buf.push(chunk));
        proces.stdout.on("end", ()=>resolve(Buffer.concat(_buf)));
        proces.stderr.setEncoding('utf8');
        proces.stderr.on("error", (error)=>reject(error));
    })
}

function takeSnapPart(x , y , w, h)
{
    return new Promise((resolve, reject)=>{
        let _buf = []
        let proces = child_process.spawn('./takepng', ['-o', '-', '-r', '3', '-x', x.toString(), '-y', y.toString(), '-w', w.toString(), '-h', h.toString()]);
        // let proces = child_process.spawn("ffmpeg", ['-f', 'x11grab', '-i', `${process.env.DISPLAY}`,'-vframes','1', '-f', 'mjpeg', '-']);
        proces.stdout.on("data", (chunk)=>_buf.push(chunk));
        proces.stdout.on("end", ()=>resolve(Buffer.concat(_buf)));
        proces.stderr.setEncoding('utf8');
        proces.stderr.on("error", (error)=>reject(error));
    })
}


var chages = [];
//require chage-monitor chageMonitor.c compiled x11 and png
async function check(){
    const proces = child_process.spawn('./chage-monitor');
    proces.stdout.setEncoding('utf-8');
    proces.stdout.on("data", (data)=>{
        chages.push(JSON.parse(data));

    })
}

const app=express();
app.use(express.static("public"));

app.get("/video", async (req, res)=>{
    res.contentType('image/jpeg');
    let proces = child_process.spawn("ffmpeg", ["-f", "x11grab", '-i', ':99', "-q:v", "10","-preset", "veryfast" ,'-f', 'mjpeg', "-vframes", "1", '-']);
    proces.stdout.pipe(res);
});


app.get("/image", async (req, res)=>{
    res.header('img-size', '1024x1600');
    let image = await takeSnap();
    // let proces = child_process.spawn("ffmpeg", ['-f', 'x11grab', '-i', `${process.env.DISPLAY}`,'-vframes','1', '-f', 'apng', '-']);
    res.contentType('image/png');
    res.send(image);
    // proces.stdout.pipe(res);
});

app.get("/imagepart/:x/:y/:w/:h", async (req, res)=>{
    res.header('img-size', '1024x1600');
    let image = await takeSnapPart(req.params.x, req.params.y, req.params.w, req.params.h);
    // let proces = child_process.spawn("ffmpeg", ['-f', 'x11grab', '-i', `${process.env.DISPLAY}`,'-vframes','1', '-f', 'apng', '-']);
    res.contentType('image/png');
    res.send(image);
    // proces.stdout.pipe(res);
})


//requared xdotools for those
app.get("/click/:x/:y", (req, res)=>{
    child_process.exec(`xdotool mousemove ${req.params.x} ${req.params.y} click 1`);
    res.send(true);
})
app.get("/rightclick/:x/:y", (req, res)=>{
    child_process.exec(`xdotool mousemove ${req.params.x} ${req.params.y} click 3`);
    res.send(true);
})

app.get('/typekey/:text', (req, res)=>{
    child_process.exec(`xdotool type ${req.params.text}`);
    res.send(true);
})

app.get("/ischage", (req,res)=>{
    res.json(chages);
    chages = [];
})

check();


app.listen(PORT, ()=>console.log("server listen on " + PORT))

/*


const express = require('express');
const child_process = require('child_process');
const { application } = require('express');
const PORT = process.env.PORT | 8000;
const app = express();

let impSize = {x:100, y:100}

// function takeSnap(x, y)
// {
//     return new Promise((resolve, rejects)=>{
//         let gest = child_process.spawn('ffmpeg', ['-video_size', `${impSize.x}x${impSize.y}`, '-f', 'x11grab', '-i', `${process.env.DISPLAY}+${x},${y}`,'-vframes','1', '-f', 'mjpeg', '-']);
//         gest.stdout.pipe(createWriteStream(`scr/images-${x}-${y}.jpg`));
//         gest.stdout.on('end',()=>resolve())
//         gest.stderr.setEncoding('utf-8');
//         gest.stderr.on('error',data=>{
//             rejects(data);
//         });
//     });
// }

app.use(express.static("public"));




app.listen(PORT, ()=>{
    console.log("app listen on Port: "+PORT);
}) 

*/