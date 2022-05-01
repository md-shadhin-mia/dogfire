# DogFire - Lightweight Remote Screen Monitor

A minimal resource remote screen monitoring solution optimized for **low bandwidth** and **low resource** environments. Built with C (X11) and Node.js, it provides real-time screen monitoring with intelligent change detection.

---

## Features

### High Performance
- **Grid-based downsampling** - Compares screen blocks instead of individual pixels (up to 16x faster)
- **Direct pixel access** - Bypasses X11 overhead for faster screen captures
- **Adaptive sleep** - Automatically adjusts polling rate based on screen activity
- **Memory efficient** - Uses contiguous arrays for better cache locality

### Smart Change Detection
- **Color threshold tolerance** - Ignores minor color differences (reduces false positives)
- **Minimum change threshold** - Only triggers when significant changes occur
- **Bounding box calculation** - Outputs exact changed region coordinates
- **JSON output** - Machine-readable change data for easy integration

### Remote Control
- **Screenshot capture** - Full screen or specific regions
- **Mouse control** - Left click, right click at any coordinate
- **Keyboard input** - Remote typing support via xdotool
- **REST API** - Simple HTTP endpoints for all operations

### Easy Deployment
- **Docker ready** - Pre-configured with Xvfb virtual display
- **One-command setup** - `docker build` and run
- **Environment configuration** - Customize via env variables

---

## Installation

### Prerequisites

**For local deployment:**
- GCC compiler
- X11 development libraries (`libX11-dev`)
- libpng development (`libpng-dev`)
- Node.js 14+
- xdotool (for remote control features)

**For Docker deployment:**
- Docker installed

### Method 1: Docker (Recommended)

```bash
# Clone repository
git clone https://github.com/muhammadshadhin/dogfire.git
cd dogfire

# Build image
docker build -t dogfire .

# Run container
docker run -d -p 8000:8000 dogfire
```

### Method 2: Manual Installation

```bash
# Install dependencies (Debian/Ubuntu)
sudo apt update
sudo apt install -y gcc make libx11-dev libpng-dev xdotool nodejs npm

# Clone and build
git clone https://github.com/muhammadshadhin/dogfire.git
cd dogfire
make

# Install Node.js dependencies
npm install

# Start server
node app.js
```

---

## Usage

### Web Interface

Open your browser and navigate to:

```
http://localhost:8000
```

The dashboard provides:
- Real-time screen display
- Change detection log
- Monitor configuration panel
- Click/typing interaction

### REST API Endpoints

#### Screen Capture

| Endpoint | Description |
|----------|-------------|
| `GET /image` | Capture full screen (PNG) |
| `GET /imagepart/:x/:y/:w/:h` | Capture specific region |
| `GET /video` | Capture as MJPEG |

#### Remote Control

| Endpoint | Description |
|----------|-------------|
| `GET /click/:x/:y` | Left click at coordinates |
| `GET /rightclick/:x/:y` | Right click at coordinates |
| `GET /typekey/:text` | Type text remotely |

#### Change Detection

| Endpoint | Description |
|----------|-------------|
| `GET /ischage` | Get pending screen changes |
| `GET /monitor/status` | Monitor status and config |
| `GET /monitor/config?gridSize=4&threshold=10` | Update monitor settings |

### Command Line Tools

**Capture screenshot:**
```bash
./takepng -o screenshot.png -r 3
```

**Run change monitor:**
```bash
./chage-monitor -g 4 -t 10 -m 1 -s 40000
```

---

## Configuration

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PORT` | `8000` | Server port |
| `CHAGE_GRID_SIZE` | `4` | Grid cell size for downsampling |
| `CHAGE_THRESHOLD` | `10` | Color difference threshold (0-100) |
| `CHAGE_MIN_CHANGES` | `1` | Min changed cells to trigger |
| `CHAGE_SLEEP` | `40000` | Base sleep time in microseconds |

### Change Monitor Flags

```bash
./chage-monitor [options]

Options:
  -g SIZE     Grid size for downsampling (default: 4)
  -t THRESH   Color difference threshold (default: 10)
  -m MIN      Min changed cells to trigger (default: 1)
  -s US       Base sleep time in microseconds (default: 40000)
  -n          Disable adaptive sleep
  -h          Show help
```

### Tuning for Performance

**High speed (low latency):**
```bash
./chage-monitor -g 2 -t 5 -s 20000
```

**Low bandwidth (minimal data):**
```bash
./chage-monitor -g 8 -t 20 -m 5 -s 60000
```

**Balanced (default):**
```bash
./chage-monitor -g 4 -t 10 -m 1 -s 40000
```

---

## Why DogFire?

### Perfect for Low Resource Environments

| Feature | Benefit |
|---------|---------|
| **Grid downsampling** | Reduces CPU usage by up to 16x vs pixel-by-pixel |
| **Change-only updates** | Only sends changed regions (saves bandwidth) |
| **Direct pixel access** | Eliminates X11 function call overhead |
| **Adaptive polling** | Slows down when screen is static |
| **Minimal dependencies** | No heavy frameworks or libraries |

### 📊 Bandwidth Comparison

**Traditional VNC:**
- Continuous full-screen streaming
- ~500KB - 2MB per frame
- Requires stable high-speed connection

**DogFire:**
- Initial full screen + incremental updates
- ~5KB - 50KB per change (region-only)
- Works on 2G/edge networks

### 💻 Resource Usage

| Metric | DogFire | VNC Server |
|--------|---------|------------|
| **Memory** | ~15-30MB | 50-150MB |
| **CPU (idle)** | <1% | 5-15% |
| **CPU (active)** | 5-20% | 20-50% |
| **Network** | 10-100KB/s | 500KB-2MB/s |

---

## Project Structure

```
dogfire/
├── chageMonitor.c      # C source: Screen change detector
├── takepng.c           # C source: Screenshot capture
├── app.js              # Node.js: HTTP server & API
├── Makefile            # Build configuration
├── script.sh           # Docker startup script
├── Dockerfile          # Docker image definition
├── package.json        # Node.js dependencies
└── public/
    └── index.html      # Web dashboard
```

---

## Architecture

```
┌─────────────────────────────────────────┐
│           Web Browser (UI)              │
└──────────────┬──────────────────────────┘
               │ HTTP/REST API
┌──────────────▼──────────────────────────┐
│         Node.js Server (app.js)         │
│  ┌───────────────────────────────────┐  │
│  │  REST API Endpoints               │  │
│  │  Change Monitor Manager           │  │
│  │  Screenshot Spawner               │  │
│  └───────────────────────────────────┘  │
└──────┬───────────────────────┬──────────┘
       │ spawn                 │ spawn
┌──────▼──────────┐   ┌────────▼─────────┐
│ chage-monitor   │   │   takepng        │
│ (C / X11)       │   │   (C / X11)      │
│                 │   │                  │
│ Grid comparison │   │ Screen capture   │
│ JSON output     │   │ PNG encoding     │
└─────────────────┘   └──────────────────┘
```

---

## Troubleshooting

### Display not found error
Ensure X server is running:
```bash
# For Docker (automatic)
docker run -d -p 8000:8000 dogfire

# For local
export DISPLAY=:0
Xvfb :99 -screen 0 1024x768x24 &
```

### Permission denied on click
Install and configure xdotool:
```bash
sudo apt install xdotool
```

### Port already in use
Change port via environment:
```bash
PORT=3000 node app.js
```

---

## Contributing

We welcome contributions! Areas we'd love help with:

### Priority Improvements

1. **Performance**
   - SIMD optimization for pixel comparison
   - Multi-threaded change detection
   - GPU acceleration support

2. **Features**
   - WebSocket support for real-time push
   - Motion vector tracking
   - Region of interest (ROI) configuration
   - Recording/playback functionality

3. **Security**
   - Authentication & authorization
   - HTTPS/TLS support
   - Input sanitization

4. **Testing**
   - Unit tests for C code
   - Integration tests for API
   - Performance benchmarks

5. **Documentation**
   - API documentation (OpenAPI/Swagger)
   - Video tutorials
   - Deployment guides (Heroku, AWS, etc.)

### How to Contribute

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## License

This project is licensed under the ISC License - see the [package.json](package.json) file for details.

---

## Acknowledgments

- X11/Xlib for screen capture capabilities
- Express.js for lightweight HTTP server
- libpng for PNG encoding
- xdotool for input simulation

---

## Contact

- **Repository:** [github.com/muhammadshadhin/dogfire](https://github.com/muhammadshadhin/dogfire)
- **Issues:** [Report a bug](https://github.com/muhammadshadhin/dogfire/issues)

---

Built with care for low-resource environments
