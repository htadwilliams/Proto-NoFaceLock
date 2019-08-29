# Proto-NoFaceLock
Prototype security tool written in C++ for Windows that locks workstation if face not detected via camera

# Dependencies
* MFC (visual studio 2019) (may be built with or without spectre mitigation libraries)
* Windows SDK (used 10.0.18362.0)
* OpenCV 2.4.13.6 (https://opencv.org/releases/).

# To build
* Set include directory for OpenCV: project properties -> C/C++ -> Additional Include Directories -> D:\dev\opencv-2.4.13.6\build\include
* Set library directory for OpenCV: project properties -> Linker -> Additional Library Directories -> D:\dev\opencv-2.4.13.6\build\x86\vc14\lib

# When updating to new version of OpenCV
* Update paths for include and library directories per build instructions 
* Update library lists for release and debug builds: project properties -> Linker -> Input -> Additional Dependencies -> 
  Add all libraries for each release type 
* If linking with libraries in .lib directory (instead of staticlib), opencv .dlls will need to be copied to location of NoFaceLock.exe, or system path adjusted to include them.
