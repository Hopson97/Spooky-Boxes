# Bullet Terrain

Project to just learn a bit more about terrain rendering, and to learn how bullet phyiscs works.

## Building and Running

### Windows (Visual Studio)

The easiest way to build is to use [vcpkg](https://vcpkg.io/en/index.html) and install libraries through this.

```bash
vcpkg install sfml
vcpkg install imgui
vcpkg install assimp
vcpkg install bullet3
vcpkg install glm
vcpkg integrate install
```

Create a new visual studio C++ empty project, git clone the files, and copy them into the project directory.

Select the "Show All Files" options in Solution Explorer, and right-click on the src/ and deps/ directory, and choose the "include in project options"

Go into the project properies and under `C/C++ > General`, add the deps/ directory as an additional include directy.

Finally, under `Linker > Input`, add OpenGL32.lib as an additional dependancy.

Under "Project -> Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions" add `_CRT_SECURE_NO_WARNINGS`z

### Linux

#### Pre-requisites

Install Vcpkg and other required packages using your distribution's package manager:

```sh
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh

# These are required to build some packages
sudo apt install cmake make autoconf libtool pkg-config

# The following are required for SFML
sudo apt install libx11-dev xorg-dev freeglut3-dev libudev-dev
```

Ensure paths are set correctly:

```sh
export VCPKG_ROOT=/path/to/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

RECOMMENDED: Add the above lines to your `.bashrc` or `.zshrc` file:

```sh
echo 'export VCPKG_ROOT=/path/to/vcpkg' >> ~/.bashrc
echo 'export PATH=$VCPKG_ROOT:$PATH' >> ~/.bashrc
```

#### Build and Run

To build, at the root of the project:

```sh
vcpkg install # First time only
sh scripts/build.sh
```

To run, at the root of the project:

```sh
sh scripts/run.sh
```

To build and run in release mode, simply add the `release` suffix:

```sh
sh scripts/build.sh release
sh scripts/run.sh release
```

### Credits

#### Models

House model by Vinrax -

https://opengameart.org/content/small-old-house

#### Textures

Grass textures by Cethiel
https://opengameart.org/content/tileable-grass-textures-set-1

---

Crate texture from learnopengl.com
https://learnopengl.com/Getting-started/Textures

---

Mud/Cliff texture from ThinMatrix - unsure if he made it himself or not, but it's from his tutorial series

[Dropbox](https://www.dropbox.com/sh/m8y3g1bh1l64hy8/AAAx9UhizogiLIRDNyWAZ72da?dl=0)
https://www.youtube.com/user/ThinMatrix

---

"""Person""" texture by me

---

Skybox texture from ThinMatrix tutorial, also from here: https://opengameart.org/content/miramar-skybox

https://www.dropbox.com/sh/phslacd8v9i17wb/AABui_-C-yhKvZ1H2wb3NykIa?dl=0

https://www.youtube.com/watch?v=_Ix5oN8eC1E

#### Sounds

sfx_step_grass_l.ogg, sfx_step_grass_r.ogg
https://opengameart.org/content/grass-foot-step-sounds-yo-frankie

---

crickets.ogg
https://opengameart.org/content/crickets-ambient-noise-loopable

---

Atmosphere_003(Loop).wav
www.blackleafstudios.net
contact@blackleafstudios.net
https://opengameart.org/content/horror-atmosphere-lite

---
