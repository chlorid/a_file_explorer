# a(kai)_file_explorer
This is a web gui based tool to handle Akai S900/950, S1000,S1100/S3000 file System containers as well as harddisks formatted with one of the machines.
After a request I added support for importing hfe images of S3000 disks made for gotek drives. To make this work, you need a library. This is described further below.

The program pretty much gives akaiutil by Klaus Michael Indlekofer (Available here: https://sourceforge.net/projects/akaiutil/) a web based GUI.
I distribute a slightly modified version of the program to create a library from it. The Program is published under the GNU 2.0 public license.
The program is still under heavy development, so let me know if you miss a feture or find a bug.

Things, that work:
-browse through images,partitions, volumes and files
-create disk container 
-format disk container 
-Create Partitions 
-Wipe Partitions
-Copy Partitions
-Create Volumes
-Delete Volumes
-Copy Volumes
-Wipe Volumes
-Copy files
-Delete files
-Play a sample on double click
-Browse S1000 Programs/ Keygroup settings (TODO: editing)
all above functionalities apply to S1000/S3000 file system. Anyway there is not mucch modification needed to enable more formats. Let me know what you are missing.

Yet to come:
-Ability to mount/unount disk containers provided by rascsi (https://github.com/akuker/RASCSI/)
-drag and drop feature to copy files
-remove bugs
-clean up code as it was derived from an example project

Contributions are welcome.

# Operating system
The code I created has been proven working on Ubuntu 18.04. and a rather recent version of MacOsX as well as on Debian Buster (RaspberryPi). Other operating systems might need modifications, but it should be possible.

# Installation

To be able to compile and run the project, you need Wt, a c++ framework to create web based apps.

For Linux:

Download it at www.webtoolkit.eu and follow the installation instructions provided. For older Linux versions you might find Wt in the packet sources. The packet is named witty.
Download the project a_file explorer to a local directory and compile with "make all".



Complete instructions for MacOs (Thanks to δέλτα άλφα):

1) Install Xcode (free download from the Apple App Store)

2) Install Xcode Command-Line Tools
   a. Run  

			xcode-select –install on Terminal, or

		b. If the above does’t work you can install them through Xcode

        i. Start Xcode on the Mac.
        ii. Choose Preferences from the Xcode menu.
        iii. In the General panel, click Downloads.
        iv. On the Downloads window, choose the Components tab.
        v. Click the Install button next to Command Line Tools.

3) Install Homebrew. On a Terminal window, run

	/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

4) Run the following lines one by one on Terminal to install wt framework.

	brew install cmake

	brew install boost

	brew install fcgi

[brew install openssl]

[brew install mysql-connector-c]

brew install libpng

[brew install libtiff]

[brew install libharu]

brew install pango

[brew install GraphicsMagick]

*The lines in brackets [   ] Are not needed to compile a_file_explorer. If you dont want the packet for anything else, omit these lines.

git clone git://github.com/kdeforche/wt.git

cd wt

mkdir build

cd build

cmake \
-DCMAKE_CXX_FLAGS='-stdlib=libc++' \
-DCMAKE_EXE_LINKER_FLAGS='-stdlib=libc++' \
-DCMAKE_MODULE_LINKER_FLAGS='-stdlib=libc++' \
-DWT_CPP_11_MODE='-std=c++11' \
-DGM_PREFIX=/usr/local [-DMYSQL_LIBRARY=mysqlclient \]
[-DMYSQL_PREFIX=/usr/local/Cellar/mysql-connector-c/6.1.6 \]
[-DWT_WRASTERIMAGE_IMPLEMENTATION=GraphicsMagick \
[-DSSL_PREFIX=/usr/local/Cellar/openssl/1.0.2h_1 ../]

*The lines in brackets [   ] Are not needed to compile a_file_explorer. If you dont want the packet for anything else, omit these lines. If you decide
 to compile the full package, make sure, you get the path to your openSSL,GraphicsMagic, etc installation right.

make -j4 # set -jN to your number of CPU cores for a faster parallel build

sudo make install

    5) Run on Terminal to clone a_file_explorer

git clone https://github.com/chlorid/a_file_explorer.git

cd a_file_explorer

make all APPLEPC=true


References

https://redmine.webtoolkit.eu/projects/wt/wiki/Installing_Wt_on_Mac_OS_X_Yosemite

https://stackoverflow.com/questions/9329243/how-to-install-xcode-command-line-tools

https://www.embarcadero.com/starthere/xe5/mobdevsetup/ios/en/installing_the_commandline_tools.html


# Running the Program

Create a symbolic link from the resources folder that came with your Wt framework to your document root folder. For example ".", the directory where you binary is. It provides the web stuff to the project.

You can achieve this in Linux and MacOs easily by changing in a Konsole window to the a_file_manager folder (the folder where your a_file_explorer.wt binary is) and entering:

ln -s /path/to/wt/resources/  or ln -s /Users/yourusername/wt/resources

create a folder named images in the a_file_explorer directory. All images in this folder are being opened on start

execute the binary as follows:

./afe.wt --docroot . --http-address 0.0.0.0 --http-port 9091 --config wt_config.xml

--http-address serves to the entire network. By choosing te localhost IP 127.0.0.1, you limit access to the local computer.
--http-port is the port the program is serving the web gui to. you can also choose 80, if you dont serve other webpage from the PC the program is run on. Then you can omit the port 			when opening the user interface in the browser.
--config The config file for WT. Settings regarding the web interface can be changed there. Make sure the right config file is loaded, as file upload will not work with the stanard one.

After executing the command, you should see a message saying the program is listening on Ip xxx and Por yyy.


Now open a browser and enter http://hostname:port (on a local installation for example : http://localhost:9091).

In order to stop the app press control+c on the Terminal window that you used to run it. If you closed that Terminal window and the program is still running,
just open a new one and run the command: sudo killall afe.wt.

# Using HXC floppy Emulator to load gotek hfe images

Download a snapshot from the repository:
https://sourceforge.net/p/hxcfloppyemu/code/HEAD/tree/HxCFloppyEmulator/
I used version r2479.
I couldnt make the library compile standalone, so I downloaded the entire repository. Maybe someone figures it out and sends me a solution?
Enter the build directory on a console window and execute Make all.

Copy the /build/libhxcfe.so file and the ../libhxcfe/.../.../.../sources/libhxcfe.h file to a_file_explorer/hxc/

That's it. You should now be able to import your hfe images successfully. Please note, that they need a hfe or HFE file extension!

# Disclaimer

I'm not associated in any way with Akai.

This program comes without any warranty and is still under development! Dont make me responsible if you lose data. If you locate the problem, you are more than welcome to contribute a solution.


