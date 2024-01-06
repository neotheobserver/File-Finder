if [ $# -lt 1 ]
then
	echo "Usage "./buid.sh x" x=w for windows, m for mac, and l for linux"
	exit 1
fi

CFLAGS="-Wall -Wextra -Wno-unused-parameter "

if [ "$1" == "w" ]
then
	CFLAGS+="-DWINDOW "
	CLIBS="-lraylib -lopengl32 -lgdi32 -lwinmm"
elif [ "$1" == "l" ]
then
	CLIBS="-lGL -lm -lpthread -ldl -lrt -L. -lraylib "
elif [ "$1" == "m" ]
then
	CLIBS="-framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo -L. -lraylib "
else
	echo "OS not supported"
	exit 1

fi
CSRCS="main.c resultlist.c "
gcc $CFLAGS -o finder $CSRCS $CLIBS
