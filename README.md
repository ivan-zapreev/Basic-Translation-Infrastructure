**The Basic Phrase-Based Statistical Machine Translation Tool**

**Author: [Dr. Ivan S. Zapreev](https://nl.linkedin.com/in/zapreevis)**

**Project pages: [Git-Hub-Project](https://github.com/ivan-zapreev/Back-Off-Language-Model-SMT)**

# Introduction
This is a fork project from the Back Off Language Model(s) for SMT project aimed at creating the entire phrase-based SMT translation infrastructure. This project follows a client/server atchitecture based on WebSockets for C++ and consists of the three main applications:

+ **bpbd-client** - is a thin client to send the translation job requests to the translation server and obtain results
+ **bpbd-server** - the the translation server consisting of the following main components:
    - *Decoder* - the decoder component responsible for translating text from one language into another
    - *LM* - the language model implementation allowing for seven different trie implementations and responsible for estimating the target language phrase probabilities.
    - *TM* - the translation model implementation required for providing source to target language phrase translation and the probailities thereof.
    - *RM* - the reordering model implementation required for providing the possible translation order changes and the probabilities thereof
+ **lm-query** - a stand-alone language model query tool that allows to perform labguage model queries and estimate the joint phrase probabilities.

##Decoding
_ToDo: Extend_

##Translatin model
_ToDo: Extend_

##Reordering model
_ToDo: Extend_

##Language model
For machine translation it is important to estimate and compare the fluency of different possible translation outputs for the same source (i.e., foreign) sentence. This is commonly achieved by using a language model, which measures the probability of a string (which is commonly a sentence). Since entire sentences are unlikely to occur more than once, this is often approximated by using sliding windows of words (n-grams) occurring in some training data.

### Language Models background
An *n-gram* refers to a continuous sequence of n tokens. For instance, given the following sentence: our neighbor , who moved in recently , came by . If n = 3, then the possible n-grams of
this sentence include: 
```
"our neighbor ,"
"neighbor , who"
", who moved"
...
", came by"
"came by ."
```

Note that punctuation marks such as comma and full stop are treated just like any ‘real’ word and that all words are lower cased.

#Project structure
This is a Netbeans 8.0.2 project, based on cmake, and its' top-level structure is as follows:

* **[Project-Folder]**/
    * **doc/** - contains the project-related documents including the Doxygen-generated code documentation
    * **ext/** - stores the external header only libraries used in the project
    * **inc/** - stores the C++ header files of the implementation
    * **src/** - stores the C++ source files of the implementation
    * **nbproject/** - stores the Netbeans project data, such as makefiles
    * **data/** - stores the test-related data such as test models and query intput files, as well as some experimental results.
    * LICENSE - the code license (GPL 2.0)
    * CMakeLists.txt - the cmake build script for generating the project's make files
    * README.md - this document
    * Doxyfile - the Doxygen configuration file

#Supported platforms
This project supports two major platforms: Linux and Mac Os X. It has been successfully build and tested on:

* **Centos 6.6 64-bit** - Complete functionality.
* **Ubuntu 15.04 64-bit** - Complete functionality.
* **Mac OS X Yosemite 10.10 64-bit** - Limited by inability to collect memory-usage statistics.

**Notes:**

1. There was only a limited testing performed on 32-bit systems.
2. The project must be possible to build on Windows platform under [Cygwin](https://www.cygwin.com/).

#External libraries
_ToDo: Write this section_

#Building the project
Building this project requires **gcc** version >= *4.9.1* and **cmake** version >= 2.8.12.2. The project can be build in two ways:

+ From the Netbeans environment by running Build in the IDE
    - Perform `mkdir build` in the project folder.
    - In Netbeans menu: *Tools/Options/"C/C++"* make sure that the cmake executable is properly set.
    - Netbeans will always run cmake for the DEBUG version of the project
    - To build project in RELEASE version use building from Linux console
+ From the Linux command-line console perform the following steps
    - `cd [Project-Folder]`
    - `mkdir build`
    - `cd build`
    - `cmake -DCMAKE_BUILD_TYPE=Release ..` OR `cmake -DCMAKE_BUILD_TYPE=DEBUG ..`
    - `make -j [NUMBER-OF-THREADS]` add `VERBOSE=1` to make the compile-time options visible

The binaries will be generated and placed into *./build/* folder. In order to clean the project from the command line run `make clean`.

##Project compile-time parameters
_ToDo: make up to date_

###General
One can limit the debug-level printing of the code by changing the value of the *LOGER_MAX_LEVEL* constant in the *./inc/Configuration.hpp*. The possible range of values, with increasing logging level is: ERROR, WARNING, USAGE, RESULT, INFO, INFO1, INFO2, INFO3, DEBUG, DEBUG1, DEBUG2, DEBUG3, DEBUG4. It is also possible to vary the information level output by the program during its execution by specifying the command line flag, see the next section.

###bpbd-client
_ToDo: Add text_
###bpbd-server
_ToDo: Add text_
###lm-query
_ToDo: Add text_

#Code documentation
_ToDo: Extend with more details_

At present the documentation is done in the Java-Doc style that is successfully accepted by Doxygen with the Doxygen option *JAVADOC_AUTOBRIEF* set to *YES*. The generated documentation is located in the **./docs/** folder of the project.

#Literature and references

This project is originally based on the followin literature:

_ToDo: Put the BibText entries into linked files_

>        @inproceedings{DBLP:conf/acl/PaulsK11,
>        author    = {Adam Pauls and
>                       Dan Klein},
>          title     = {Faster and Smaller N-Gram Language Models},
>          booktitle = {The 49th Annual Meeting of the Association for Computational Linguistics:
>                       Human Language Technologies, Proceedings of the Conference, 19-24
>                       June, 2011, Portland, Oregon, {USA}},
>          pages     = {258--267},
>          year      = {2011},
>          crossref  = {DBLP:conf/acl/2011},
>          url       = {http://www.aclweb.org/anthology/P11-1027},
>          timestamp = {Fri, 02 Dec 2011 14:17:37 +0100},
>          biburl    = {http://dblp.uni-trier.de/rec/bib/conf/acl/PaulsK11},
>          bibsource = {dblp computer science bibliography, http://dblp.org}
>        }

and

>        @inproceedings{DBLP:conf/dateso/RobenekPS13,
>          author    = {Daniel Robenek and
>                       Jan Platos and
>                       V{\'{a}}clav Sn{\'{a}}sel},
>          title     = {Efficient In-memory Data Structures for n-grams Indexing},
>          booktitle = {Proceedings of the Dateso 2013 Annual International Workshop on DAtabases,
>                       TExts, Specifications and Objects, Pisek, Czech Republic, April 17,
>                       2013},
>          pages     = {48--58},
>          year      = {2013},
>          crossref  = {DBLP:conf/dateso/2013},
>          url       = {http://ceur-ws.org/Vol-971/paper21.pdf},
>          timestamp = {Mon, 22 Jul 2013 15:19:57 +0200},
>          biburl    = {http://dblp.uni-trier.de/rec/bib/conf/dateso/RobenekPS13},
>          bibsource = {dblp computer science bibliography, http://dblp.org}
>        }

_ToDo: Add the paper of Ken LM_
_ToDo: Add the SMT book_

The first paper discusses optimal Trie structures for storing the learned text corpus and the second indicates that using *std::unordered_map* of C++ delivers one of the best time and space performances, compared to other data structures, when using for Trie implementations

_ToDo: Add more details about the papers and books_

#General design

#Using software

## _bpbd-server_ - translation server
## _bpbd-client_ - translation client
## _lm-query_ - LM query tool
In order to get the program usage information please run *./lm-query* from the command line, the output of the program is supposed to be as follows:

``` 
vpn-stud-146-50-150-5:build zapreevis$ lm-query 
USAGE:  ------------------------------------------------------------------ 
USAGE: |                 Back Off Language Model(s) for SMT     :)\___/(: |
USAGE: |                       Software version 1.1             {(@)v(@)} |
USAGE: |                         The Owl release.               {|~- -~|} |
USAGE: |            Copyright (C) Dr. Ivan S Zapreev, 2015-2016 {/^'^'^\} |
USAGE: |  ═════════════════════════════════════════════════════════m-m══  |
USAGE: |        This software is distributed under GPL 2.0 license        |
USAGE: |          (GPL stands for GNU General Public License)             |
USAGE: |          The product comes with ABSOLUTELY NO WARRANTY.          |
USAGE: |   This is a free software, you are welcome to redistribute it.   |
USAGE: |                     Running in 64 bit mode!                      |
USAGE: |                 Build on: Mar 10 2016 17:11:35                   |
USAGE:  ------------------------------------------------------------------ 
PARSE ERROR:  
             Required arguments missing: query, model

Brief USAGE: 
   lm-query  [-l <lm lambda weight>] [-d <error|warn|usage|result|info
             |info1|info2|info3>] -q <query file name> -m <model file name>
             [--] [--version] [-h]

For complete USAGE and HELP type: 
   lm-query --help
```

#Software details
## _bpbd-client_
_ToDo: Add details on how the client works including requirements and structure_
## _bpbd-server_
_ToDo: Add details on how the server works including requirements and structure_
## _lm-query_
_ToDo: Update details on how the query tool works including requirements and structure_

In this section we mention a few implementation details, for more details see the source code documentation. The code contains the following important source files:

* **main.cpp** - contains the entry point of the program
* **Executor.cpp** -  contains some utility functions including the one reading the test document and performing the queries on a filled in Trie instance.
* **ARPATrieBuilder.hpp / ARPATrieBuilder.cpp** - contains the class responsible for reading the ARPA file format and building up the trie model using the ARPAGramBuilder.
* **TrieDriver.hpp** - is the driver for all trie implementations - allows to execute queries to the tries.
* **LayeredTrieDriver.hpp** - is a wrapper driver for all the layered trie implementations - allows to retrieve N-gram probabilities and back-off weights.
* **C2DHashMapTrie.hpp / C2DHashMapTrie.cpp** - contains the Context-to-Data mapping trie implementation based on unordered_map.
* **C2DMapArrayTrie.hpp / C2DMapArrayTrie.cpp** - contains the Context-to-Data mapping trie implementation based  on unordered_map and ordered arrays.
* **C2WOrderedArrayTrie.hpp / C2WOrderedArrayTrie.cpp** - contains the Context-to-Word mapping trie implementation based on ordered arrays.
* **G2DHashMapTrie.hpp / G2DHashMapTrie.cpp** - contains the M-Gram-to-Data mapping trie implementation based on self-made hash maps.
* **W2CHybridMemoryTrie.hpp / W2CHybridMemoryTrie.cpp** - contains the Word-to-Context mapping trie implementation based on unordered_map and ordered arrays.
* **W2COrderedArrayTrie.hpp / W2COrderedArrayTrie.cpp** - contains the Word-to-Context mapping trie implementation based on ordered arrays.
* **Configuration.hpp** - contains configuration parameter for the word index and trie and memory management entities.
* **Exceptions.hpp** - stores the implementations of the used exception classes.
* **HashingUtils.hpp** - stores the hashing utility functions.
* ** ARPAGramBuilder.hpp / ARPAGramBuilder.cpp** - contains the class responsible for building n-grams from a line of text and storing it into Trie.
* **StatisticsMonitor.hpp / StatisticsMonitor.cpp** - contains a class responsible for gathering memory and CPU usage statistics
* **Logger.hpp/Logger.cpp** - contains a basic logging facility class

#Licensing
This is a free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.

#History
* **21.04.2015** - Created
* **27.07.2015** - Changed project name and some to-do's
* **21.09.2015** - Updated with the latest developments preparing for the version 1, Owl release. 
* **11.03.2016** - Updated Updated to reflect the project status. 

https://github.com/adam-p/markdown-here/wiki/Markdown-Cheatsheet
