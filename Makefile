################################################################################
# Makefile for Wikipedia Graph
# Kiana McNellis, Rob Russo, Ryan Manske
# Parallel Progrmaming
################################################################################

################################################################################
# Usage
################################################################################

all: compile

compile: clean
	mpic++ -Wall main.cpp -o main.out

clean:
	rm -f main.out

n ?= 16
run: compile
	mpirun -n $(n) ./main.out

download:
	echo "WARNING ABOUT TO DOWNLOAD 13GBs"
	# curl -O https://dumps.wikimedia.org/enwiki/20170101/enwiki-20170101-pages-meta-history1.xml-p000000010p000002245.7z
	curl -o https://dumps.wikimedia.org/enwiki/20170101/enwiki-20170101-pages-articles-multistream.xml.bz2
