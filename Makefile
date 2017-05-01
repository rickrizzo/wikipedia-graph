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
n ?= 9

################################################################################
# Commands
################################################################################
all: compile run

main.out: main.cpp article.cpp article.h
	mpic++ -Wall main.cpp article.cpp -o main.out -lpthread -g -O0 -fno-inline

compile: main.out

clean:
	rm -f main.out
	rm -f parse.out
	rm -f makeDirs.out

run: compile
	mpirun -n $(n) ./main.out

download:
	echo "WARNING ABOUT TO DOWNLOAD 13GBs"
	curl -O https://dumps.wikimedia.org/enwiki/20170101/enwiki-20170101-pages-articles-multistream.xml.bz2

unzip:
	bzip2 -kd enwiki-20170101-pages-articles-multistream.xml.bz2

parse.out: parseFiles.cpp helpers.h
	g++ parseFiles.cpp -o parse.out -std=c++98 -g

makeDirs.out: makeDirs.cpp
	g++ makeDirs.cpp -o makeDirs.out -std=c++98 -g

parse: makeDirs.out parse.out
	rm -rf article
	./makeDirs.out
	./parse.out

blue:
	# mpic++ -O5 main.cpp article.cpp -o main.out
	mpixlcxx -O5 main.cpp article.cpp -o main.out -qflag=w
	mpixlcxx -O5 parseFiles.cpp -o parse.out -qflag=w
	mpixlcxx -O5 makeDirs.cpp -o makeDirs.out -qflag=w
