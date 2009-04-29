CFLAGS += -O1
all: bact
spec: bact.c
	$(CC) bact.c -o bact -O1 -DONLINE_JUDGE
