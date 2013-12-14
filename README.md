SearchServer
============

SearchServer provides an HTTP-Service to find Websites/Wikipedia-Articles from
a database which are relevant for certain key-words. This is basically an
experiment with tf-idf-ranking. And it is a basic file server ;)

TODO: write readme ;) ("./SearchServerMain --help" is quiet verbose)

in short:

1. git clone ...
2. make
3. unzip example/wikipedia-sentences.csv.zip
4. build the index (may take some seconds) and start serving queries:
   `./SearchServerMain example/wikipedia-sentences.csv 8080`
5. open [http://localhost:8080/](http://localhost:8080)
6. Type a scientific key-word (the example contains wikipedia-articles about
   some scientists with names early in the alphabet).
