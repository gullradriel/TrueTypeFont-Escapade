```
 _______           _______               ______          _   
|__   __|         |__   __|             |  ____|        | |  
   | |_ __ _   _  ___| |_   _ _ __   ___| |__ ___  _ __ | |_ 
   | | '__| | | |/ _ \ | | | | '_ \ / _ \  __/ _ \| '_ \| __|
   | | |  | |_| |  __/ | |_| | |_) |  __/ | | (_) | | | | |_ 
 __|_|_|   \__,_|\___|_|\__, | .__/ \___|_|  \___/|_| |_|\__|
|  ____|                 __/ | |     | |                     
| |__   ___  ___ __ _ _ |___/|_|_  __| | ___                 
|  __| / __|/ __/ _` | '_ \ / _` |/ _` |/ _ \                
| |____\__ \ (_| (_| | |_) | (_| | (_| |  __/                
|______|___/\___\__,_| .__/ \__,_|\__,_|\___|                
                     | |                                     
                     |_|
```

# TrueTypeFont Escapade

My entry for Krampus Hack 2025, a game jam hosted by https://tins.amarillion.org/

# Secret Santa

I'm the Secret Santa for dlharmon911

# Recipient wishlist

```
Two weeks away!!! What to wish for? As some of you might know, I'm a big fan of font manipulation. So I think I'll will wish for something font related.

1. Font as a major component. You can use it any way you want, but it the game must have a feel that the font is important.
2. Procedurally generated .... something procedurally generated (example maze, terrain, etc)
3. Purple - my favorite color (0x36013f)

And as always the most important rule:

4. Have fun!!!!!
```

# The entry, TrueTypeFont Escapade

The game is a multi level 3D game.

Mazes are generated from each letters of the sentences in the 'DATA/level' config file, using the config specified font.

Use the keyboard and mouse to reach the exit letter at the end of the sentence without falling and before running out of time.

Fire the surprise boxes to reclaim some life/speed/score upgrade bonuses.

# Controls

- W/S/A/D or arrows or ZQSD : move
- Mouse : look
- F1    : pause/unpause (unlocks/locks mouse, shows PAUSE)
- F11   : toggle fullscreen
- SPACE : jump. When hearing the slip sound, you can also trigger a 'save jump'
- Left mouse button : shoot projectiles
- ENTER : start / advance to next level / get onto final screen once levels are finished
- ESC   : quit, everywhere ESC key is quit, be carefull ! 
  WEB VERSION: press ESC two times, first time to ungrab the mouse, second time to exit

# WASM/Enscripten version online

If you can not build the entry of if the binaries are not working for you, just click here:

https://www.nilorea.net/Games/TrueTypeFont_Escapade/

# How to build

## prerequisites

### Linux/Windows build
- make
- gcc
- allegro development libraries installed

### WebAssembly (wasm) build
- Emscripten SDK installed and sourced (`source /path/to/emsdk/emsdk_env.sh`)
- make
- git (for cloning dependencies)

## procedure

```
# clone the repo
git clone https://github.com/gullradriel/TrueTypeFont-Escapade.git
cd TrueTypeFont-Escapade

###############
# Compilation #
###############

# Linux/Windows build
make

# wasm/Emscripten build
make wasm

############
# CLEANING #
############

# clean Linux/Windows build
make clean

# clean wasm/Emscripten build (game and libs)
make wasm-clean

# clean all builds
make clean-all
```

# Resources used

## songs and sound effects

All taken are from https://pixabay.com/

### bonus: shell used to convert to ogg

```
for file in `ls *.mp3`
do 
    base_name="${file%.mp3}"
    ffmpeg -i "$file" -c:a libvorbis -qscale:a 6 "${base_name}.ogg"
done
```

## References / Articles for the 3D stuff

### 3D from Allegro 5

- Allegro 5 3D camera and transforms

- Official example ex_camera.c: how to set up a 3D camera with ALLEGRO_TRANSFORM, perspective projection and a free-flying view.

- al_perspective_transform man page: explains the parameters (left, top, near, right, bottom, far) used to build the perspective projection matrix.

- al_build_camera_transform docs: the function we mirror conceptually for al_build_camera_transform(&view, eye, at, up) to create a look-at camera matrix.

- ALLEGRO_TRANSFORM documentation: describes Allegro’s 4×4 matrix type and how it’s used for 2D/3D transforms and camera/view matrices.

- ALLEGRO_VERTEX documentation: how 3D vertices (position, color, texture coords) are defined and drawn with al_draw_prim, which is exactly what the voxelized letters and boxes use.

### 3D collision & capsule / AABB articles

- "Capsule Collision Detection" : https://wickedengine.net/2020/04/capsule-collision-detection
- "Real Time Collision Detection" : http://www.r-5.org/files/books/computers/algo-list/realtime-3d/Christer_Ericson-Real-Time_Collision_Detection-EN.pdf

# Additional TWEAKS

You can have a look at app_config.json to see how to tweak the internals (speed, gravity, fonts used, ...)

You can also use in games keyboard shortcut I used to help me test the game, see below.

## CHEAT keyboard shortcuts

- F3 : toggle gravity aka fly mode
- 1  : toggle COLOR_CYCLE_GOAL (goal rainbow)
- 2  : toggle PULSE_TEXT (pulsing glow)
- 3  : toggle BLEND_TEXT (additive glow)
- t  : add 30s of time
- v  : add SPEED-BONUS-INCREMENT to maximum player's velocity

## Command line arguments

TTF_Escapade can be started with:

```
-h => print help
-v => print version
-V LOGLEVEL => choose log level (NOTICE, INFO, ERR, DEBUG, default ERR)
-L logfile  => log to file 'logfile'
-f level_font_file => use 'level_font_file' as level font file
-g gui_font_file   => use 'gui_font_file' as gui font
-l levels_file     => use 'levels_file' as levels file
```

To ease the testings, I used these flags to start the game with different fonts and levels, like the single level levels1.txt file.
