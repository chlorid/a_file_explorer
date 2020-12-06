# akai_file_explorer
This is a web gui based tool to handle Akai S900/950, S1000,S1100/S3000 file System containers as well as harddisks formatted with one of the machines.

The program pretty much gives akaiutil by Klaus Michael Indlekofer. (Available here: https://sourceforge.net/projects/akaiutil/) a web based GUI.

Things, that already work:
-create disk container
-format disk container 
-Create Partitions (needs some work)
-Wipe Partitions
-Copy Partitions
-Create Volumes
-Delete Volumes
-Copy Volumes
-Wipe Volumes
-Copy files
-delete files
all above functionalities apply to S1000/S3000 file system. Anyway there is not mucch modification needed to enable more formats.

Yet to come:
-Ability to mount/unount disk containers provided by rascsi (https://github.com/akuker/RASCSI/)
-drag and drop feature to copy files
-remove bugs
-clean up code as it was derived from an example project

Contributions are welcome.

# Operating system
The code I created has ben proven working on Ubuntu 18.04.Other operating systems might need modifications, but it should be possible as the libraries used are also compatible with windows and mac

# Installation

To be able to compile and run the project, you need Wt, a c++ framework to create web based apps. Download it at www.webtoolkit.eu and follow the installation instructions provided.

Download the project to a local directory and compile with "make all".

Create a symbolic link from the resources folder that came with your Wt framework to your document root folder. For example ".", the directory where you binary is. It provides the web stuff to the project.

execute the binary as follows:

./augui --docroot . --http-address 0.0.0.0 --http-port 9091

This starts a webserver providing the program for everyone in the network on all IP Adresses of the computer (0.0.0.0)on the given Port(9091).

Now open a browser and enter http://hostname:port (on a local installation for example : http://localhost:9091).

# Disclaimer

This program comes without any warranty and is still under development! Dont make me responsible if you lose data. Better commit a solution for the problem.
