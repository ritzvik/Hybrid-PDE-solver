This README is intended for the next developer to understand what we have been able to gather in our project work. The reader is highly encouraged to read the EndSem report to understand the motivation of this document and basics of Singularity. These steps are only meant to help the reader overcome the tedious configuration and running issues faced by us.

Required : Singularity


## petscbase.singularity

This file contains the steps to create a singularity image to run relevant PETSC code on CPU

To create singularity image with the name `petscbase.simg` run:
```
$ sudo singularity build petscbase.simg petscbase.singularity
```

## ex3.c
This file contains a basic MPI supported code for solving linear equations for a tridiagonal matrix using KSP method. The file is adequately commented.

## makefile
This is used to compile C codes into executables files. This makefile has been taken from the official samples provided by the authors of PETSC. Though the final commands are displayed when make is executed, some underline code in this makefile carries out some required (still unidentified) executions or declarations to successfully compile the C codes. Thus we resorted to use this makefile only.


## Steps to run the code

```
$ # ignore this step if done previously
$ sudo singularity build petscbase.simg petscbase.singularity
```

```
$ # make executable for ex3.c using the makefile inside the singularity environment
$ singularity exec petscbase.simg make ex3
```
The present working directory of the system is by mounted automatically inside the singularity environment (at `/home` by default).

```
$ # running the final code on 4 cores parallely for a matrix of size 1000 X 1000
$ singularity exec petscbase.simg mpiexec -n 4 ./ex3 -n 1000
```

## NOTE :

  - The executable created inside the singularity environment can't be run directly as it has linked binaries which exist only inside the singularity image.
  - An interactive shell can be opened inside the singularity environment using `singulairty shell petscbase.simg`. Inside the shell `singularity exec petscbase.simg` can be ommited while running commands. It would behave as a conventional ubuntu with command line interface.
