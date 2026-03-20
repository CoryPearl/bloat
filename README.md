# BLOAT

[https://corypearl.itch.io/bloat](https://corypearl.itch.io/bloat)

An **arcade survival** game where your score isn’t a number — it’s a **physical body** that drags behind you, ruins your turning, and gives enemies more to hit.  
The better you do, the worse it gets.

- **Jam theme**: Growing  
- **Made by**: Cory Pearl  

### How to play
- **Turn**: `A/D` or `←/→`
- **Shed**: `Space` (4s cooldown, requires **length ≥ 6**)
- **Pause**: `Esc`
- **Restart run**: `R`
- **From pause / game over**: `Space` returns to main menu

### The core mechanic (why it’s fun)
- **Collect pickups** to gain segments.
- **Your length is your multiplier**: every pickup scores \(10 ×\) your current length.
- **More length = worse handling**: turning gets slower as you grow.
- **Enemies target your body**: they chase the closest part of you (head or any segment).
- **Shed to survive**: when ready, `Space` drops **about half** your body to regain speed; dropped segments linger briefly as fading obstacles before dissolving.

### Win / lose
- **Lose** if you collide with an enemy, your own body, or recently-shed segments.
- **Goal**: survive and keep clearing waves (the game counts up; wave 100 is treated as the “you made it” milestone).

### Running the game

#### macOS (build from source)
This project uses **raylib** and plain C.

1) Install raylib (Homebrew):

```bash
brew install raylib
```

2) Build + run using the included script:

```bash
./run.sh
```

If you prefer, the command it runs is:

```bash
gcc bloat.c -o bloat -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib
./bloat
```

#### Windows (prebuilt)
- Run `bloat.exe` from the project root.
- The `raylib-win/` folder contains raylib headers/libs used for Windows builds.

#### Web (WASM)
The WebAssembly build artifacts live in `bloat-wasm/`:
- `index.html`, `index.js`, `index.wasm`, `index.data`
- A packaged build: `bloat-wasm/bloat_wasm.zip`

To run locally, serve the folder (browser security blocks `file://` WASM loads):

```bash
cd bloat-wasm
python3 -m http.server 8080
```

Then open `http://localhost:8080` in your browser.

### Project layout
- `bloat.c`: the full game source (gameplay, UI screens, audio, entities)
- `run.sh`: quick build/run for macOS (raylib via Homebrew)
- `bloat-wasm/`: WebAssembly build output
- `design/`: jam/itch page mockups and UI design HTML
- `menu.mp3`, `game.mp3`, `pickup.wav`, `shed.wav`, `sound.png`: game audio + UI asset

### Tech
- **Language**: C (raylib)
- **Rendering / input / audio**: [raylib](https://www.raylib.com/)

### Credits & third‑party
- **Game / code / design**: Cory Pearl
- **Engine/library**: raylib — Copyright (c) 2013–2024 Ramon Santamaria

`raylib-win/LICENSE` contains raylib’s zlib/libpng license.

### License
No explicit license is included for the game code/assets in this repo.  
By default, assume **all rights reserved** for the game content, and refer to raylib’s license for the bundled `raylib-win/` distribution.

