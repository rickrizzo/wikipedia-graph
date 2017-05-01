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

compile:
	mpic++ -Wall main.cpp article.cpp -o main.out -lpthread

clean:
	rm -f main.out
	rm -f parse.out

run:
	mpirun -n $(n) ./main.out

download:
	echo "WARNING ABOUT TO DOWNLOAD 13GBs"
	curl -O https://dumps.wikimedia.org/enwiki/20170101/enwiki-20170101-pages-articles-multistream.xml.bz2

parse:
	rm -rf article
	g++ parseFiles.cpp -o parse.out -std=c++98
	./parse.out


blue:
	# mpic++ -O5 main.cpp article.cpp -o main.out
	mpixlcxx -O5 main.cpp article.cpp -o main.out -qflag=w
