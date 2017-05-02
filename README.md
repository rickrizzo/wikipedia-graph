# Wikipedia Graph
##### Kiana McNellis, Ryan Manski, Rob Russo

## Description
This project set out to create a graph containing all of Wikipedia's articles. Articles are stored along with the links they contain. These references are then sent to their respective articles such that each article is aware of all of the article it references, as well as all of the articles which reference it.

## Parallel Algorithm
To create this graph, the Wikipedia XML dump is first read in to the parser program. This breaks out the articles into individual files and places them into directories based upon their title name. Following the completion of the parsing, the main file reads in the these files and creates a number of article objects by rank. Ranks are ordered so that they each cover a certain group of directories. Once in memory, articles are send to the correct ranks in order to update the articles they reference. Once this communication is complete, the graph is created. For more detailed information, check out the formal report!

## Design Considerations
Written using C++ to take advantage of objects.
