#OBJS specifies which files to compile as part of the project
OBJS = main_test.c SDLFUNC.c RNGTEXGEN.c RNGTILES.c VECMATHS.c qdbmp/qdbmp.c

#CC specifies which compiler
CC = gcc

#COMPILER FLAGS specifies the additional compilation options
#-w supresses all warnings
COMPILER_FLAGS = -w

#LINKER FLAGS specifies the libraries we're linking
LINKER_FLAGS = -lm -lSDL2

#OBJ_NAME specifies the name of the executable
OBJ_NAME = RNGTEXLin

#Target to compile executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
