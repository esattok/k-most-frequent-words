# K Most Frequent Words

- Application that finds the K most frequently occurring words (top-K words) for a given data-set
- The application is developed on Linux operating system using C programming language
- Processes, Threads, and IPC (Inter-Process Communication) is used in the implementation

## Contents

- Project1.pdf (Project Description)
- proctopk.c (Source File)
- threadtopk.c (Source File)
- Makefile (Makefile to Compile the Project)
- report.pdf (Experimental Results)

## How to Run

- cd to the project directory

##### Compilation and linking

```
$ make
```

##### Recompile

```
$ make clean
$ make
```

##### Running the proctopk program

```
$ ./proctopk <K> <outfile> <N> <infile1> .... <infileN>
```

##### Example proctopk run

```
$ make
$ ./proctopk 10 output.txt 3 input-01.txt input-02.txt input-03.txt
```

##### Running the threadtopk program

```
$ ./threadtopk <K> <outfile> <N> <infile1> .... <infileN>
```

##### Example threadtopk run

```
$ make
$ ./threadtopk 10 output.txt 3 input-01.txt input-02.txt input-03.txt
```
