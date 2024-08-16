all:
	g++ main.cpp -o main  `pkg-config --cflags --libs gtkmm-4.0`	
run: all
	./main
clean:
	rm -rf *.o main
python:
	cython3 --embed -o app-cython.c main.py
	gcc -Os -I /usr/include/python3.12 -o app-cython app-cython.c -lpython3.12 -lpthread -lm -lutil -ldl
	rm app-cython.c
python-run: python
	./app-cython
