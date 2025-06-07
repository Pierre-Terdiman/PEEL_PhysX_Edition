# PEEL (PhysX Edition)
Physics Engine Evaluation Lab

![alt text](peel.jpg?raw=true)

PEEL was a tool designed to evaluate, compare and benchmark physics engines. See PEEL 1.1 Github repo here: https://github.com/Pierre-Terdiman/PEEL

This is PEEL 2.0, dedicated to PhysX.

For now this is mainly a private backup of the code online. Feel free to look at it but there is no documentation, it is not easy to compile, it is unsupported, you're on your own.

Because the project supports both very old and very new engines, building everything with the same compiler is basically impossible. Some old engines whose source code has never been made public require e.g. Visual Studio 2010, while brand new engines only support Visual Studio 2019 and C++17. I have no good build solution for this project. Included here on GitHub are various manually maintained Visual Studio solutions. I tend to use VS2008 for Win32 builds, VS2017 for x64 builds, and VS2019 for new engines like Jolt (included here as an example).

This "PhysX edition" currently only includes plugins for PhysX 4.1.2, PhysX 5.1.0, PhysX 5.4.2, PhysX 5.5.0, PhysX 5.6.0. The older versions are not included so far. Due to the size limits enforced by Github on binary files, the GPU DLLs have been zipped. Just unzip them locally if you need them. You should still be able to run the precompiled binaries on the CPU.

Videos:

[![Watch the video](https://img.youtube.com/vi/0r4i5UjNOQo/maxresdefault.jpg)](https://www.youtube.com/watch?v=0r4i5UjNOQo)

And more:

https://www.youtube.com/watch?v=jHWBD8P58dg
(You can run that one locally using Run_PhysX4_Showreel2.bat in the Build64 folder)

https://www.youtube.com/watch?v=6dATi4-wb3o
(You can run that one locally using Run_PhysX4_Showreel.bat in the Build64 folder)

Screenshots:

![alt text](/Screenshots/AnymalC.jpg?raw=true)
![alt text](/Screenshots/LegoTechnicBuggy.jpg?raw=true)
![alt text](/Screenshots/LegoTechnicKart.jpg?raw=true)
![alt text](/Screenshots/JacobsLadder.jpg?raw=true)
![alt text](/Screenshots/NutAndBolt.jpg?raw=true)

![alt text](/Screenshots/Screenshot00.jpg?raw=true) ![alt text](/Screenshots/Screenshot01.jpg?raw=true) ![alt text](/Screenshots/Screenshot02.jpg?raw=true)
![alt text](/Screenshots/Screenshot03.jpg?raw=true) ![alt text](/Screenshots/Screenshot04.jpg?raw=true) ![alt text](/Screenshots/Screenshot05.jpg?raw=true)
![alt text](/Screenshots/Screenshot06.jpg?raw=true) ![alt text](/Screenshots/Screenshot07.jpg?raw=true) ![alt text](/Screenshots/Screenshot08.jpg?raw=true)
![alt text](/Screenshots/Screenshot09.jpg?raw=true) ![alt text](/Screenshots/Screenshot10.jpg?raw=true) ![alt text](/Screenshots/Screenshot11.jpg?raw=true)
![alt text](/Screenshots/Screenshot12.jpg?raw=true) ![alt text](/Screenshots/Screenshot13.jpg?raw=true) ![alt text](/Screenshots/Screenshot14.jpg?raw=true)
![alt text](/Screenshots/Screenshot15.jpg?raw=true) ![alt text](/Screenshots/Screenshot16.jpg?raw=true) ![alt text](/Screenshots/Screenshot17.jpg?raw=true)
![alt text](/Screenshots/Screenshot18.jpg?raw=true) ![alt text](/Screenshots/Screenshot19.jpg?raw=true) ![alt text](/Screenshots/Screenshot20.jpg?raw=true)

