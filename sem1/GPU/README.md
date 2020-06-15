## cudapetscbase.singularity

This file contains the steps to create a base singularity image with GPU support

To create singularity image with the name `cudapetscbase.simg` run:
```
$ sudo singularity build cudapetscbase.simg cudapetscbase.singularity
```

## petsc-3.10.5.tar.gz
This is the petsc source code and used to build petsc inside singularity container.

## cudapetsctop.singularity

This file contains the steps to configure PETSC with GPU support and uses the previously-made 
singularity image file as a base.
This also uses `petsc-3.10.5.tar.gz`.

To create singularity image with the name `cudapetsc.simg` run:
```
$ sudo singularity build cudapetsc.simg cudapetsctop.singularity
```

This image would be the final environment we would be needing for running PETSC
on both CPU & GPU.

## ex_cg.c
The file contains a parallelized implementation of conjugate gradient method as described in the report.
The file is commented as requiered for explanation

## makefile
This is used to compile C codes into executables files. 
This makefile has been taken from the official samples provided by the authors of PETSC. 
Though the final commands are displayed when make is executed, 
some underline code in this makefile carries out some required 
(still unidentified) executions or declarations to successfully compile the C codes. 
Thus we resorted to use this makefile only.

## Compliling the code
```
singularity exec cudapetsc.simg make ex_cg
```

## Running the code on CPU

  - Single Core
    ```
    singularity exec --nv cudapetsc.simg ./ex_cg -n 100 -iter 100
    ```

  - Multi Core
    ```
    singularity exec --nv cudapetsc.simg mpiexec -n 4 ./ex_cg -n 100 -iter 100
    ```

## Running the code on GPU

  ```
  singularity exec --nv cudapetsc.simg ./ex_cg --vec_type cuda --mat_type aijcusparse -n 100 -iter 100
  ```


## Additional Notes

  - flags `-n` and `-iter` denotes the size of matrix and number of iterations respectively. They are thouroughly defined inside the code.
  - flag after `--nv` is required with singularity exec as it links the internally installed cuda with the external GPU driver.
