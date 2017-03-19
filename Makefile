FLAGS=-O2

# uncomment the following line to compile using OpenMP
#FLAGS=-fopenmp -O2

adrinas: *.c *.h
	${CC} ${FLAGS} -o AudioCorruptDetect main.c AudioCorruptDetect.c sound.c -lsndfile -lm

test:
	./AudioCorruptDetect example.wav restored.wav
	@echo
	diff example_restored.wav restored.wav
	@echo
	@echo Test successful!

clean:
	rm -f AudioCorruptDetect restored.wav
