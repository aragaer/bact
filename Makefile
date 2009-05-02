CFLAGS += -O1
all: bact
spec: bact.c nb.c
	$(CC) bact.c -o bact -O1 -DONLINE_JUDGE
	$(CC) nb.c -o nb -O1 -DONLINE_JUDGE
