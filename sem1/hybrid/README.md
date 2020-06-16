Hybrid PDE solver : It offloads some part of the compuatation to CPU and mostly to GPU.

The same singularity image created in `GPU` section for running `ex_cg.c` is used here.
Please copy `cudapetsc.simg` in this directory.


## ex_cghybrid_var.c

This code facilitates division of the vector into two parts, each handled by CPU and GPU respectively. 
The code is not commented as relevant comments are there is the similar code `ex_cghybrid_thread.c`.

## ex_cghybrid_thread.c

Hybrid implementation as discussed in the presentation(not in the report)
The A matrix is divided into four parts, 2 handled by CPU and 2 by GPU and vector b is divided into 2 parts.
each handled by a CPU and GPU. 
Threads are created to place calls conccurrently to both GPU and GPU so that neither of them would have to wait for other to finish.

The file is commented in relevant sections are is similar to `ex_cg.c` in structure.


## ex_cghybird_time.c

Same as `ex_cghybrid_thread.c` but each function is seperately timed.

## makefile
This is used to compile C codes into executables files. 
This makefile has been taken from the official samples provided by the authors of PETSC. 
Though the final commands are displayed when make is executed, 
some underline code in this makefile carries out some required 
(still unidentified) executions or declarations to successfully compile the C codes. 
Thus we resorted to use this makefile only.


## Compiling the codes

```
$ singularity exec --nv ./cudapetsc.simg make ex_cghybrid_thread
$ singularity exec --nv ./cudapetsc.simg make ex_cghybrid_time
```

## Running the codes

For running the code on 4 CPU cores and a singe GPU simultaneously, with size 10000000, iterations 1000 and
workload divided to CPU and GPU with a ratio of 0.3.
```
singularity exec --nv ./cudapetsc.simg mpiexec -n 4 ./ex_cghybrid_time -n 10000000 -iter 1000 -div 0.3
singularity exec --nv ./cudapetsc.simg mpiexec -n 4 ./ex_cghybrid_thread -n 10000000 -iter 1000 -div 0.3
```

The flags are explained in `ex_cghybrid_thread.c` in detail.
