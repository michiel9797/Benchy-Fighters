SOURCE		 = action_list.cc \
		   engine.cc \
		   main.cc \
		   yojimbo_layer.cc
OBJS		 = action_list.o \
		   engine.o \
		   main.o \
		   network_layer.o
OUT	 	 = BenchyFighters
CC	 	 = c++
FLAGS		 = -Wall -Weffc++ -std=c++17 -O2 -DYOJIMBO_DEBUG -DYOJIMBO_SERVER=1 -DYOJIMBO_CLIENT=1
LDLFLAGS 	 = yojimbo/yojimbo.a \
		   yojimbo/sodium/sodium.a \
		   yojimbo/netcode/netcode.a \
		   yojimbo/reliable/reliable.a \
		   -lsodium
INCLUDES 	 = -Iyojimbo \
		   -Iyojimbo/source \
		   -Iyojimbo/include \
		   -Iyojimbo/serialize 

all: $(OBJS)
	$(CC) $(OBJS) -o $(OUT) $(LDLFLAGS)

%.o: %.cc
	$(CC) $(FLAGS) $(INCLUDES) -c $< -o $@

network_layer.o: yojimbo_layer.cc
	$(CC) $(FLAGS) $(INCLUDES) -c yojimbo_layer.cc -o network_layer.o

clean:
	rm -f $(OBJS) $(OUT)