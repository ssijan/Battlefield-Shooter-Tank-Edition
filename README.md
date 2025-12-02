# 🛡️ Battlefield Shooter: Tank Edition

## 🎮 Overview
**Battlefield Shooter: Tank Edition** is a 2D real-time tank shooting game developed using **C++ and OpenGL (GLUT)**. Players control a tank navigating a racetrack, shooting targets while avoiding obstacles. The game features detailed tank graphics, trackside scenery, dynamic bullets, and a real-time scoring system.

---

## 🌟 Features

- **Player Tank Control**
  - Move forward/backward (`W` / `S`)
  - Rotate left/right (`←` / `→`)
  - Fire bullets (`SPACE`)
  - Stop tank (`F`)

- **Dynamic Targets**
  - Randomly spawning targets per wave
  - Destroy all targets to spawn a new wave
  - Score counter updates in real-time

- **Transform Effects**
  - Reflection toggle (`R`)
  - Shear/Skew toggle (`H`)

- **HUD & Game Timer**
  - Displays current score and remaining time
  - Game Over message with final score
  - Restart game (`N`) or exit (`ESC`)

- **Graphics & Scenery**
  - Detailed tank: turret, tracks, barrel, shadows
  - Track: asphalt, outer borders, inner curbs, dashed lane line
  - Scenery: stands, pit building, trees
  - Realistic color palette

---

## ⌨️ Controls

| Key | Action |
|-----|-------|
| W   | Accelerate forward |
| S   | Brake / Reverse |
| ← / → | Rotate tank |
| SPACE | Fire bullet |
| R   | Toggle reflection |
| H   | Toggle shear effect |
| F   | Stop tank movement |
| N   | Restart game |
| ESC | Exit game |

---

## 🗂️ Project Structure

.
├── main.cpp # Initializes GLUT, main loop, and game callbacks
├── game.h / game.cpp # Game logic: tank, bullets, targets, scoring
├── graphics.h / graphics.cpp # Drawing utilities, tank rendering, track & scenery
├── images/ # Screenshots for README
└── README.md

yaml
Copy code

---

## ⚙️ Compilation & Execution

**Windows (g++ / MinGW + freeglut)**:
```bash
g++ main.cpp game.cpp graphics.cpp -lfreeglut -lopengl32 -lglu32 -o TankGame.exe
./TankGame.exe
