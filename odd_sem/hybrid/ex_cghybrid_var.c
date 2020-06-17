static char help[] = "Implementation of conjugate Gradient\n\n";

#include <petscksp.h>
#include <time.h>
#include <stdio.h>


void set_mat_values_tridiag(PetscInt rstart, PetscInt rend, PetscInt n, Mat A){
    PetscInt col[3], i;
    PetscScalar value[3];

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

    return;
}

void MatMult_split(Mat A11, Mat A12, Mat A21, Mat A22, Vec x1, Vec x2, Vec b1, Vec b2, Vec b1_, Vec b2_){
    MatMult(A11,x1,b1);
    MatMult(A12,x2,b1_);
    MatMult(A21,x1,b2);
    MatMult(A22,x2,b2_);
    VecAXPY(b1,(PetscScalar)1.0,b1_);
    VecAXPY(b2,(PetscScalar)1.0,b2_);
    return;
}

void VecAXPY_split(Vec y1, Vec y2, PetscReal alpha, Vec x1, Vec x2){
    VecAXPY(y1, alpha, x1);
    VecAXPY(y2, alpha, x2);
    return;
}

void VecSet_split(Vec v1, Vec v2, PetscReal value){
    VecSet(v1, value);
    VecSet(v2, value);
    return;
}

void VecDot_split(Vec x1, Vec x2, Vec y1, Vec y2, PetscReal *value){
    PetscReal a, b;
    VecDot(x1, y1, &a);
    VecDot(x2, y2, &b);
    *value = (a+b);
    return;
}

void VecAXPBY_split(Vec y1, Vec y2, PetscReal alpha, PetscReal beta, Vec x1, Vec x2){
    VecAXPBY(y1, alpha, beta, x1);
    VecAXPBY(y2, alpha, beta, x2);
    return;
}

void VecView_split(Vec x1, Vec x2){
    VecView(x1, PETSC_VIEWER_STDOUT_WORLD );
    VecView(x2, PETSC_VIEWER_STDOUT_WORLD );
    return;
}

void MatView_spit(Mat A11, Mat A12, Mat A21, Mat A22){
    MatView(A11, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A12, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A21, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A22, PETSC_VIEWER_STDOUT_WORLD );
}
int main(int argc, char **args){
    Vec x1, x2, b1, aux1, b2, aux2, u1, u2, r1, r2, p1, p2, Ap1, Ap2;
    Mat A11, A12, A21, A22;
    PetscReal alpha, beta, rdot, pAp, val;
    PetscInt n=100, nC, nG, col[3], rstart1, rend1, nlocal1, rstart2, rend2, nlocal2, i, iters = 100;
    PetscScalar one=1.0, minusone=-1.0, zero=0.0, value[3];
    PetscReal CGdiv = 0.2;
    PetscBool printflg;

    PetscInitialize(&argc,&args,(char*)0,help);
    PetscOptionsGetInt(NULL,NULL,"-n",&n,NULL);
    PetscOptionsGetInt(NULL,NULL,"-iters",&iters,NULL);
    PetscOptionsHasName(NULL,NULL,"-print",&printflg);
    PetscOptionsGetReal(NULL,NULL,"-div",&CGdiv,NULL);


    nC = (PetscInt)(n*CGdiv);
    nG = n - nC;


    VecCreate(PETSC_COMM_WORLD,&x1);
    VecSetSizes(x1,PETSC_DECIDE,nC);
    VecSetType(x1, VECSTANDARD);

    VecCreate(PETSC_COMM_WORLD,&x2);
    VecSetSizes(x2,PETSC_DECIDE,nG);
    VecSetType(x2, VECCUDA);

    VecDuplicate(x1,&b1);
    VecDuplicate(x1,&u1);
    VecDuplicate(x1,&r1);
    VecDuplicate(x1,&p1);
    VecDuplicate(x1,&Ap1);

    VecDuplicate(x2,&b2);
    VecDuplicate(x2,&u2);
    VecDuplicate(x2,&r2);
    VecDuplicate(x2,&p2);
    VecDuplicate(x2,&Ap2);

    VecCreate(PETSC_COMM_WORLD,&aux1);
    VecSetSizes(aux1,PETSC_DECIDE,nC);
    VecSetType(aux1, VECCUDA);

    VecCreate(PETSC_COMM_WORLD,&aux2);
    VecSetSizes(aux2,PETSC_DECIDE,nG);
    VecSetType(aux2, VECCUDA);


    VecGetOwnershipRange(x1,&rstart1,&rend1);
    VecGetLocalSize(x1,&nlocal1);

    VecGetOwnershipRange(x2,&rstart2,&rend2);
    VecGetLocalSize(x2,&nlocal2);

    // printf("%n %n \n", (int)nlocal1, (int)nlocal2);

    MatCreate(PETSC_COMM_WORLD,&A11);
    MatSetSizes(A11,nlocal1,nlocal1,nC,nC);
    MatSetType(A11, MATAIJ);
    MatSetUp(A11);

    MatCreate(PETSC_COMM_WORLD,&A22);
    MatSetSizes(A22,nlocal2,nlocal2,nG,nG);
    MatSetType(A22, MATAIJCUSPARSE);
    MatSetUp(A22);

    MatCreate(PETSC_COMM_WORLD,&A12);
    MatSetSizes(A12,nlocal1,nlocal2,nC,nG);
    MatSetType(A12, MATAIJCUSPARSE);
    MatSetUp(A12);

    MatCreate(PETSC_COMM_WORLD,&A21);
    MatSetSizes(A21,nlocal2,nlocal1,nG,nC);
    MatSetType(A21,MATAIJ);
    MatSetUp(A21);

    set_mat_values_tridiag(rstart1, rend1, nC, A11);
    set_mat_values_tridiag(rstart2, rend2, nG, A22);

    if (rstart2==0){
        col[0] = nC-1; value[0] = -1.0;
        i=0;
        MatSetValues(A21,1,&i,1,col,value,INSERT_VALUES);
    }
    MatAssemblyBegin(A21,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(A21,MAT_FINAL_ASSEMBLY);

    if (rstart1==0){
        col[0] = 0; value[0] = -1.0;
        i = nC-1;
        MatSetValues(A12,1,&i,1,col,value,INSERT_VALUES);
    }
    MatAssemblyBegin(A12,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(A12,MAT_FINAL_ASSEMBLY);

    // MatView_spit(A11, A12, A21, A22);
    
    // get the b vector
    VecSet_split(u1, u2, one);
    MatMult_split(A11,A12,A21,A22,u1,u2,b1,b2,aux1,aux2);
    if(printflg) VecView_split(b1,b2);

    VecSetRandom(x1, NULL);
    VecSetRandom(x2, NULL);
    MatMult_split(A11,A12,A21,A22,x1,x2,r1,r2,aux1,aux2);
    VecAXPY_split(r1, r2, minusone, b1, b2);

    VecSet_split(p1, p2, zero);
    VecAXPY_split(p1, p2, minusone, r1, r2);

    for (i=0;i<iters;++i) {
        VecDot_split(r1, r2, r1, r2, &rdot);
        MatMult_split(A11,A12,A21,A22,p1,p2,Ap1,Ap2,aux1,aux2);
        VecDot_split(p1,p2,Ap1,Ap2,&pAp);

        alpha = rdot/pAp;

        VecAXPY_split(x1, x2, alpha, p1, p2);
        VecAXPY_split(r1, r2, alpha, Ap1, Ap2);

        VecDot_split(r1, r2, r1, r2, &val);
        beta = val/rdot;

        VecAXPBY_split(p1, p2, minusone, beta, r1, r2);
    }

    if(printflg) VecView_split(x1, x2);

    return 0;

}
