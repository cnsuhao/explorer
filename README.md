Pulse Physiology Explorer
=========================

The Pulse Explorere is an application developed to analyze and
visualize the results produced by the Pulse Physiology Engine using ParaView.
The Pulse Physiology Engine is a powerful open-source, multi-platform tool for
modeling and simulation of the adult human physiology during healthy and disease states. 
ParaView is a powerful open-source, multi-platform data analysis and 
visualization tool widely used in the scientific visualization research community. 
Creating an extension to Paraview to run scenarios and visualize the results 
produced by the physiology engine is key to creating a useful tool for researchers and clinicians.

## Build Environment

The code provided utilizes C++11, here is a list of popular compilers and their initial version to implement all of C++11 :
- GCC 4.8.1 and later
- Clang 3.3 and later
- MSVC 2015 and later

If you have any questions\comments, don't hesitate to email physiology@kitware.com.

While the provided cmake superbuild automatically pulls many libraries it needs to compile, 
you will still need to have the following tools installed (along with your choice of C++ compiler) :

### CMake
Currently, the code requires CMake 3.7 or greater to properly build
Go to the cmake website, `https://cmake.org/download`, and download the appropriate distribution.
Ensure that cmake bin is on your PATH and available in your cmd/bash shell.

Note on Linux systems, make sure to have curl before you build/install cmake for it to behave properly when downloading 3rd party libraries.
~~~~~~~~~~~~~~~~~~~~~~~~
sudo apt-get install curl
~~~~~~~~~~~~~~~~~~~~~~~~

### Qt

It currently does not pull Qt. You will need to download and install Qtv5 on your own.
You can get it from <a href="https://www.qt.io/download-open-source/">here</a>

### Java JDK

While there is no dependency on Java by the Explorer application, 
it is currently required by the engine to generate required data files.

Add a JAVA_HOME environment variable to point to the Java installation and add it to the system PATH.<br>
There are many ways to do this, here is a simple walk through to get you going with a JDK.

#### Windows
- Download the Windows x64 JDK <a href="http://www.oracle.com/technetwork/java/javase/downloads/index.html">here.</a>
- Run the installer.
- Goto your Control Panel->System and click Advanced system settings on the left. <br>
- Click the 'Environment Variables' button and add JAVA_HOME as a new 'System variables'.<br>
- Set the Value to something like: C:\Program Files\Java\jdk1.8.0_121<br>
    - It's a good idea to add the JDK to the system PATH by adding this to the beginning: %JAVA_HOME%/bin;
- Make sure to start a new cmd window.<br>

#### Linux
- You can find where java is by running `update-alternatives --list java <br>
 - If you don't have a Java SDK, I recommend using an installer like Synaptic
 - Search for 'jdk' by name and install the 'openjdk-8-jdk' 
- You can then add the JAVA_HOME variable to a bash shell by typing
    - export JAVA_HOME=(a path listed by a call to updata-alternatives --list java)
    - For example : export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64
- You can also add it to your ~/.bash_profile, or related file (.bashrc, .zshrc, .cshrc, setenv.sh), to get the path in all shells

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~bash
JAVA_HOME='/usr/lib/jvm/java-8-openjdk-amd64'
export JAVA_HOME
PATH="$JAVA_HOME/bin:$PATH"
export PATH
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

## Building

The build is directed by CMake to ensure it can be built on various platforms. 
The code is build by a CMake 'superbuild', meaning as part of the build, CMake will download any
dependent libraries and compile those before it builds, specifically the Engine and ParaView.
The build is also out of source, meaning the code base is seperated from the build files.
This means you will need two folders, one for the source code and one for the build files.
Generally, I create a single directory to house these two folders.
Here is the quickest way to pull and build via a cmd/bash shell:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~bash
mkdir physiology
cd physiology
git clone https://gitlab.kitware.com/physiology/explorer src
mkdir builds
cd builds
# Feel free to make subfolders here, like msvc2017x64 or release or something
# Generate a make file/msvc solution for the external dependencies
# Note you need to provide cmake the source directory at the end (relative or absolute)
# Run CMake (it will use the system default compiler if you don't provide options or use the CMake GUI)
cmake -DCMAKE_BUILD_TYPE:STRING=Release ../src
# Note you will need to provide the path to Qt if it is not on the PATH
cmake -DCMAKE_BUILD_TYPE:STRING=Release -DQt5_DIR=C:/Programming/Tools/Qt/5.9.1/msvc2017_64/lib/cmake/Qt5 ../src

# Build the install target/project
# On Linux/OSX/MinGW 
make  
# For MSVC
# Open the OuterBuild.sln and build the project (It will build everything!)
# When the build is complete, MSVC users can close the OuterBuild solution, and open the PhysiologyExplorer.sln located in the PhysiologyExplorer directory.
# Unix based systems should also cd into this directory for building any changes to the PhysiologyExplorer code base
cd PhysiologyExplorer
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The application will be build in the <build dir>/install

## Notes

!! The Explorer is still under development and may be unstable !!

As the GUI is based on Qt, if the GUI is missing any widgets, such as the graphing views,
you will need to remove the Qt PhysiologyExplorer0.1.0.ini file from your system.

It can be found in your user directory ./configs
Or on windows:
C:\Users\your_login\AppData\Roaming\Kitware, Inc

If you find any other issues, please do not hesitate to log any issue in our repository.





