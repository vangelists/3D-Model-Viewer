3D Model Viewer
===============

This project was created in 2017 as a bonus assignment for the Computer Graphics course and is no longer maintained.

In case you are interested for a very brief overview of its features, you can take a look at the included [slide(s)](doc/cs358_bonus_assignment_slides.pdf).

---

## Dependencies

- glGA (OpenGL Geometric Application Framework - Educational Edition)
    - GLEW (OpenGL Extension Wrangler Library)
    - Assimp (Open Asset Import Library)
    - Dear ImGui
    - SOIL2 (Simple OpenGL Image Library)


## Project Structure

    3D Model Viewer
    ├── CMakeLists.txt
    ├── README.md
    ├── include
    │   ├── Common.h
    │   ├── Environment.h
    │   ├── GUI.h
    │   ├── Object.h
    │   ├── PolygonMesh.h
    │   ├── Utilities.h
    │   ├── fonts
    │   │   └── IconsFontAwesome.h
    │   └── lib
    │       └── tiny-file-dialogs
    │           └── tinyfiledialogs.h
    ├── res
    │   ├── fonts
    │   │   ├── FontAwesome.ttf
    │   │   └── RobotoMedium.ttf
    │   ├── media
    │   │   └── Dreamy.mp3
    │   └── models
    │       ├── Gear.obj
    │       └── Sphere.obj
    ├── shaders
    │   ├── BoundingBox.frag
    │   ├── BoundingBox.vert
    │   ├── Object.frag
    │   └── Object.vert
    └── src
        ├── CMakeLists.txt
        ├── Environment.cpp
        ├── GUI.cpp
        ├── Main.cpp
        ├── Object.cpp
        ├── PolygonMesh.cpp
        └── lib
            └── tiny-file-dialogs
                └── tinyfiledialogs.c


## Condiguring and Building

#### Configuring

1. Set `GLGA_PATH` in `./CMakeLists.txt` (Line 20) to the root directory of the glGA project.

2. Set `rootDirectory` in `./include/Common.h` to the location of the project root (this folder). Optionally, you may also choose the window dimensions.

#### Building and Running with Make

    $ mkdir <build_directory>
    $ cd <build_directory>
    $ cmake ..
    $ make
    $ make test

#### Generating Xcode project

    $ mkdir <build_directory>
    $ cd <build_directory>
    $ cmake -G Xcode ..
    $ open 3D_Model_Viewer.xcodeproj


## Notes

* Some directories (`./bin`, `./examples` and `./libraries`) may need manual removal.


## License

Licensed under the [Mozilla Public License 2.0](LICENSE).


## Contact

Vangelis Tsiatsianas - [contact@vangelists.com](mailto:contact@vangelists.com?subject=[GitHub]%203D%20Model%20Viewer)
