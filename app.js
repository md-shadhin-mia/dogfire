const express = require("express");
const child_process = require("child_process");

const PORT = process.env.PORT || 8000;

const CHAGE_MONITOR_CONFIG = {
    gridSize: process.env.CHAGE_GRID_SIZE || 4,
    threshold: process.env.CHAGE_THRESHOLD || 10,
    minChanges: process.env.CHAGE_MIN_CHANGES || 1,
    sleepTime: process.env.CHAGE_SLEEP || 40000,
};

function takeSnap(x =0 , y = 0)
{
    return new Promise((resolve, reject)=>{
        let _buf = []
        let proces = child_process.spawn('./takepng', ['-o', '-', '-r', '3']);
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
        proces.stdout.on("data", (chunk)=>_buf.push(chunk));
        proces.stdout.on("end", ()=>resolve(Buffer.concat(_buf)));
        proces.stderr.setEncoding('utf8');
        proces.stderr.on("error", (error)=>reject(error));
    })
}


var chages = [];
var monitorProcess = null;
var isMonitorRunning = false;

function startChangeMonitor(){
    if (isMonitorRunning) return;
    isMonitorRunning = true;

    const args = [
        '-g', CHAGE_MONITOR_CONFIG.gridSize.toString(),
        '-t', CHAGE_MONITOR_CONFIG.threshold.toString(),
        '-m', CHAGE_MONITOR_CONFIG.minChanges.toString(),
        '-s', CHAGE_MONITOR_CONFIG.sleepTime.toString()
    ];

    monitorProcess = child_process.spawn('./chage-monitor', args);
    monitorProcess.stdout.setEncoding('utf-8');
    monitorProcess.stdout.on("data", (data)=>{
        const lines = data.trim().split('\n');
        for (const line of lines) {
            if (line.length > 0) {
                try {
                    chages.push(JSON.parse(line));
                } catch (e) {
                    console.error('Failed to parse change data:', line);
                }
            }
        }
    });

    monitorProcess.stderr.on("data", (data)=>{
        console.error('[chage-monitor]', data.toString());
    });

    monitorProcess.on("error", (error)=>{
        console.error('chage-monitor process error:', error.message);
        isMonitorRunning = false;
    });

    monitorProcess.on("exit", (code, signal)=>{
        console.log(`chage-monitor exited with code ${code}`);
        isMonitorRunning = false;
        if (code !== 0 && code !== null) {
            setTimeout(() => startChangeMonitor(), 1000);
        }
    });

    console.log(`Started chage-monitor with config:`, CHAGE_MONITOR_CONFIG);
}

function stopChangeMonitor(){
    if (monitorProcess) {
        monitorProcess.kill('SIGTERM');
        monitorProcess = null;
        isMonitorRunning = false;
    }
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
    res.contentType('image/png');
    res.send(image);
});

app.get("/imagepart/:x/:y/:w/:h", async (req, res)=>{
    res.header('img-size', '1024x1600');
    let image = await takeSnapPart(req.params.x, req.params.y, req.params.w, req.params.h);
    res.contentType('image/png');
    res.send(image);
})


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

app.get("/monitor/status", (req, res)=>{
    res.json({
        running: isMonitorRunning,
        changesCount: chages.length,
        config: CHAGE_MONITOR_CONFIG
    });
});

app.get("/monitor/config", (req, res)=>{
    const newConfig = {
        gridSize: req.query.gridSize,
        threshold: req.query.threshold,
        minChanges: req.query.minChanges,
        sleepTime: req.query.sleepTime
    };

    if (newConfig.gridSize) CHAGE_MONITOR_CONFIG.gridSize = parseInt(newConfig.gridSize);
    if (newConfig.threshold) CHAGE_MONITOR_CONFIG.threshold = parseInt(newConfig.threshold);
    if (newConfig.minChanges) CHAGE_MONITOR_CONFIG.minChanges = parseInt(newConfig.minChanges);
    if (newConfig.sleepTime) CHAGE_MONITOR_CONFIG.sleepTime = parseInt(newConfig.sleepTime);

    stopChangeMonitor();
    startChangeMonitor();

    res.json({ success: true, config: CHAGE_MONITOR_CONFIG });
});

startChangeMonitor();


app.listen(PORT, ()=>console.log("server listen on " + PORT))