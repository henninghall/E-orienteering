# set this variable to the director in which you saved the common files
commondir = ../common/

all : lab

lab : lab.c $(commondir)GL_utilities.c helpers.c $(commondir)VectorUtils3.c $(commondir)loadobj.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c
	gcc -Wall -o lab -I$(commondir) -I../common/Linux -DGL_GLEXT_PROTOTYPES lab.c $(commondir)GL_utilities.c helpers.c $(commondir)loadobj.c $(commondir)VectorUtils3.c $(commondir)LoadTGA.c $(commondir)Linux/MicroGlut.c -lXt -lX11 -lGL -lm

clean :
	rm lab
