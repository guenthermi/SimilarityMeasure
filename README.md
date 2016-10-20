# SimilarityMeasure
SimilarityMeasure - compute for one node in a graph the most similar one

## Build
To build SimilarityMeasure you need a C++11 Compiler and the development package for libcurl. For me this is [libcurl4-openssl-dev](https://packages.debian.org/de/sid/libcurl4-openssl-dev).
Please use the g++ Compiler for C++ Version 11.

## Run

````
similarityMeasure indexFileLocation/IndexFileName [LogFile] [TrackFile]
````

If you set `logFile=stdout` the output is printet to stdout as usual. If you define a `TrackFile` you will be asked later to give a comma seperated list of Item Ids for that you want to track the similarity aspects.

## Index Files
You need an index out of a .map and a .data file (examples are available below). You have to store the files in the same folder with the same name.

You can download sample index files from Wikidata under following urls:

data file:
https://drive.google.com/open?id=0BwLpj_RT842oZ1AzSEVkeVp5dkE

map file:
https://drive.google.com/open?id=0BwLpj_RT842oTnlPZFo3eEI4V1U
