OBJS	= engine.cc main.cc
SOURCE	= engine.o main.o
HEADER	=
OUT	= BenchyFighters
CC	= c++
FLAGS	= -Wall -Weffc++ -std=c++17 -O2 -Iinclude

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

engine.o: engine.cc
	$(CC) $(FLAGS) pkg-config nlohmann_json --cflags engine.cc

main.o: main.cc
	$(CC) $(FLAGS) main.cc

clean:
	rm -f $(SOURCE) $(OUT)