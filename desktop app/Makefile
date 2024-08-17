all:
	g++ main.cpp -o build/main  `pkg-config --cflags --libs gtkmm-4.0`	
run: all
	build/main
clean:
	rm -rf *.o main
python:
	cython3 --embed -o app-cython.c main.py
	gcc -Os -I /usr/include/python3.12 -o build/app-cython app-cython.c -lpython3.12 -lpthread -lm -lutil -ldl
	rm app-cython.c
python-build-run: python
	build/app-cython
python-run:
	python3 main.py
