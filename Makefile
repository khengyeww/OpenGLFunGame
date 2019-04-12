#  
#  Makefile :  OpenGLで動作するプログラム "TARGET".c をコンパイル 
#  
#    ** TARGETをコマンドラインで上書きすることができます ** 
#     > make TARGET=c1-1 
#     > make TARGET=c1-1 clean
#    

TARGET = c1-1

RM = rm -rf
CFLAGS = -O2
CC = /usr/bin/gcc
LIBS = -L$(ROOT)/usr/X11R6/lib

all : $(TARGET)

$(TARGET) : $(TARGET).c
	$(RM) $@
	$(CC) $(CFLAGS) -o $@ $(TARGET).c $(LIBS) -lglut -lGLU -lXmu -lGL -lX11 -lm

clean :
	$(RM) core $(TARGET)
