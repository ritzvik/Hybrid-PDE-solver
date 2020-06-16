static char help[] = "Implementation of conjugate Gradient\n\n";

#include <petscksp.h>
#include <petscsys.h>
#include <time.h>
#include <stdio.h>

int main(int argc, char **args){
    Vec x, b, u, r, p, Ap;
    Mat A;
    PetscReal alpha, beta, rdot, pAp, val;
    // PetscErrorCode ierr;
    PetscInt n=10, col[3], rstart, rend, nlocal, i, iters = 100;
    PetscScalar one=1.0, minusone=-1.0, zero=0, value[3];
    PetscBool printflg;

    PetscInitialize(&argc,&args,(char*)0,help);
    PetscOptionsGetInt(NULL,NULL,"-n",&n,NULL);
    PetscOptionsGetInt(NULL,NULL,"-iters",&iters,NULL);
    PetscOptionsHasName(NULL,NULL,"-print",&printflg);

    VecCreate(PETSC_COMM_WORLD,&x);
    VecSetSizes(x,PETSC_DECIDE,n);
    VecSetFromOptions(x);

    VecDuplicate(x,&b);
    VecDuplicate(x,&u);
    VecDuplicate(x,&r);
    VecDuplicate(x,&p);
    VecDuplicate(x,&Ap);

    VecGetOwnershipRange(x,&rstart,&rend);
    VecGetLocalSize(x,&nlocal);


    MatCreate(PETSC_COMM_WORLD,&A);
    MatSetSizes(A,nlocal,nlocal,n,n);
    MatSetFromOptions(A);
    MatSetUp(A);


    // Assigning values to tridiagonal matrix A
    if (rstart == 0){
        rstart=1;
        i=0;
        col[0]=0; col[1]=1; value[0]=2.0; value[1]=-1.0;
        MatSetValues(A, 1, &i, 2, col, value, INSERT_VALUES);
    }
    if (rend == n){
        rend=n-1;
        i=n-1;
        col[0]=n-2; col[1]=n-1; value[0]=-1.0; value[1]=2.0;
        MatSetValues(A, 1, &i, 2, col, value, INSERT_VALUES);
    }

    value[0]=-1.0; value[1]=2.0; value[2]=-1.0;
    for (i=rstart; i<rend; ++i){
        col[0] = i-1; col[1] = i; col[2] = i+1;
        MatSetValues(A,1,&i,3,col,value,INSERT_VALUES);
    }

    MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);


    // get the b vector
    VecSet(u, one);
    MatMult(A,u,b);
    if (printflg) VecView(b, PETSC_VIEWER_STDOUT_WORLD );


    clock_t cStart,cEnd;
    cStart = clock();

    // Initialization for loop
    // PetscRandomSeed((PetscRandom)50);
    VecSetRandom(x, NULL);
    MatMult(A,x,r);
    VecAXPY(r,minusone,b);
    

    //Initialize p=-r
    VecSet(p, zero);
    VecAXPY(p,minusone,r);

    for (i=0;i<iters;++i){
        VecNorm( r , NORM_2 , &rdot);
        rdot = rdot*rdot;
        MatMult(A,p,Ap);
        VecDot(p, Ap, &pAp);

        alpha = rdot/pAp;

        VecAXPY(x, alpha, p);

        VecAXPY(r,alpha,Ap);

        VecNorm(r,NORM_2,&val);
        val = val*val;
        beta = val/rdot;

        VecAXPBY(p,minusone, beta, r);
    }


    cEnd = clock();
    double total = (double)(cEnd - cStart) / CLOCKS_PER_SEC;

    if(printflg) VecView(x, PETSC_VIEWER_STDOUT_WORLD );

    PetscPrintf(PETSC_COMM_WORLD,"%lf", (double)total);

    return 0;
}
