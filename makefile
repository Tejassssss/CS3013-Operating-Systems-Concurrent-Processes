all: shuemake

shuemake: Shueworld.c
	gcc -o shuemake Shueworld.c -o Shueworld

clean:
	rm -f shuemake