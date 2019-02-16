CFLAGS=-Wall -g
CC=clang

all: cs152coin_part2

cs152coin_part2: cs152coin.o parser.o addr_bal.o dbl_spends.o hmap.o deanon.o
	$(CC) $(CFLAGS) -o $@ $^
		
.PHONY clean:
	rm -f *.o
	rm -Rf *.dSYM
	rm -f cs152coin_part2