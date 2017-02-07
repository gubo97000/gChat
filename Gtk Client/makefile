all:  gcl

gcl: main.c gcl.c fcl.c func.h
	gcc  main.c gcl.c fcl.c -o gcl -g `pkg-config --cflags --libs libgnome-2.0 libgnomeui-2.0`

clean:
	rm -f gcl
