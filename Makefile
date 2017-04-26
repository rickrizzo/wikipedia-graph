################################################################################
# Makefile for Wikipedia Graph
# Kiana McNellis, Rob Russo, Ryan Manske
# Parallel Progrmaming
################################################################################

################################################################################
# Usage
################################################################################

################################################################################
# Variables
################################################################################
n ?= 10

################################################################################
# Commands
################################################################################
all: compile

compile: clean
	mpic++ -Wall main.cpp -o main.out

clean:
	rm -f main.out
	rm -f parse.out

run: compile
	mpirun -n $(n) ./main.out

download:
	echo "WARNING ABOUT TO DOWNLOAD 13GBs"
	curl -O https://dumps.wikimedia.org/enwiki/20170101/enwiki-20170101-pages-articles-multistream.xml.bz2

parse: clean
	rm -f article/article_*
	rmdir article
	g++ parseFiles.cpp -o parse.out
	./parse.out
