
COMPILING INSTRUCTIONS
Note that you may have to modify the Makefile appropriately depending on the 
location of the opengl library/header files on your system.

cd graphiz_1_0
gmake 

The graphiz_1_0/parser directory contains the executable 'ucc'. Some example 
source files are present in the 'demo' directory. For eg. you can run the 'pawn' demo by 

cd demo
../parser/ucc pawn.gc


This distribution contains the sweep generator tool in the sweep_gen directory. You can
 specify the outline of an object, and the tool 'sweeps' this outline about the 
y axis to create an object. For eg. change to sweep_gen directory and run ./sweep.
 Click on the screen to create your outline and when you are done hit the enter key.
 The object would be created about the origin, so you may have to move back quite a bit
 using the 's' key. Hit escape at any time to exit. Press enter to store this outline
 into 'object01.dat'. Then change to the demo directory and run

../parser/ucc showOb.gc

You can generate programmed textures using the genimage tool in the genimage directory.
This is the outline of a simple C program which you can modify to create your own custom
textures. Usage is 

genimage <width> <height> <output file name>
