MAKE=gmake

all: sweep_mod parser_mod

sweep_mod:
	gmake -C sweep_gen
parser_mod:
	gmake -C parser

clean:
	gmake -C sweep_gen clean
	gmake -C parser    clean
