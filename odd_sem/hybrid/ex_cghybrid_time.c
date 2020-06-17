/*

Same code as that of cg_hybrid_thread.c.
Each function is seperately timed.

*/


static char help[] = "Implementation of conjugate Gradient\n\n";

#include <petscksp.h>
#include <time.h>
#include <stdio.h>


int set_mat_values(PetscInt rstart, PetscInt rend, PetscInt n, Mat A){
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

    return 0;
}

clock_t MatMult_split(Mat A11, Mat A12, Mat A21, Mat A22, Vec x1, Vec x2, Vec b1, Vec b2, Vec b1_, Vec b2_){

    clock_t begin, end;
    clock_t a,b,r;

    begin = clock();
    MatMult(A11,x1,b1);
    MatMult(A21,x1,b2);
    end = clock();
    a = (end - begin);


    begin = clock();
    MatMult(A22,x2,b2_);
    MatMult(A12,x2,b1_);
    end = clock();
    b = (end - begin);

    if( a > b ) r = a ; else r = b;

    begin = clock();
    VecAXPY(b1,(PetscScalar)1.0,b1_);
    VecAXPY(b2,(PetscScalar)1.0,b2_);
    end = clock();
    r += (end - begin);

    return r;
}

clock_t VecAXPY_split(Vec y1, Vec y2, PetscReal alpha, Vec x1, Vec x2){
    clock_t begin, end;
    clock_t a,b,r;

    begin = clock();
    VecAXPY(y1, alpha, x1);
    end = clock();
    a = (end - begin);


    begin = clock();
    VecAXPY(y2, alpha, x2);
    end = clock();
    b = (end - begin);

    if( a > b ) r = a ; else r = b;

    return r;
}

clock_t VecSet_split(Vec v1, Vec v2, PetscReal value){

    clock_t begin, end;
    clock_t a,b,r;


    begin = clock();
    VecSet(v1, value);
    end = clock();
    a = (end - begin);


    begin = clock();
    VecSet(v2, value);
    end = clock();
    b = (end - begin);

    if( a > b ) r = a ; else r = b;

    return r;
}

clock_t VecDot_split(Vec x1, Vec x2, Vec y1, Vec y2, PetscReal *value){
    PetscReal a, b;

    clock_t begin, end;
    clock_t a1,b1,r;


    begin = clock();
    VecDot(x1, y1, &a);
    end = clock();
    a1 = (end - begin);

    begin = clock();
    VecDot(x2, y2, &b);
    b1 = (end - begin);

    if( a1 > b1 ) r = a1 ; else r = b1;

    *value = (a+b);
    return r;
}

clock_t VecAXPBY_split(Vec y1, Vec y2, PetscReal alpha, PetscReal beta, Vec x1, Vec x2){
    clock_t begin, end;
    clock_t a,b,r;

    begin = clock();
    VecAXPBY(y1, alpha, beta, x1);
    end = clock();
    a = (end - begin);

    begin = clock();
    VecAXPBY(y2, alpha, beta, x2);
    b = (end - begin);

    if( a > b ) r = a ; else r = b;

    return r;
}

int VecView_split(Vec x1, Vec x2){
    VecView(x1, PETSC_VIEWER_STDOUT_WORLD );
    VecView(x2, PETSC_VIEWER_STDOUT_WORLD );
    return 0;
}

int MatView_spit(Mat A11, Mat A12, Mat A21, Mat A22){
    MatView(A11, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A12, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A21, PETSC_VIEWER_STDOUT_WORLD );
    MatView(A22, PETSC_VIEWER_STDOUT_WORLD );
    return 0;
}
int main(int argc, char **args){

    clock_t cb , ce;
    Vec x1, x2, b1, aux1, b2, aux2, u1, u2, r1, r2, p1, p2, Ap1, Ap2;
    Mat A11, A12, A21, A22;
    PetscReal alpha, beta, rdot, pAp, val,norm1,norm2, CGdiv = 0.2;
    PetscInt n=100,n1,n2, col[3], rstart1, rend1, nlocal1, rstart2, rend2, nlocal2, i, iters = 100;
    PetscScalar one=1.0, minusone=-1.0, zero=0.0, value[3];
    PetscBool printflg;

    PetscInitialize(&argc,&args,(char*)0,help);
    PetscOptionsGetInt(NULL,NULL,"-n",&n,NULL);
    PetscOptionsGetInt(NULL,NULL,"-iters",&iters,NULL);
    PetscOptionsGetReal(NULL,NULL,"-div",&CGdiv,NULL);
    PetscOptionsHasName(NULL,NULL,"-print",&printflg);

    n1 = (PetscInt)(CGdiv*n);
    n2 = n - n1;

    // printf("n = %d , n1 = %d , n2 = %d",n,n1,n2);


    VecCreate(PETSC_COMM_WORLD,&x1);
    VecSetSizes(x1,PETSC_DECIDE,n1);
    VecSetType(x1, VECCUDA);

    VecCreate(PETSC_COMM_WORLD,&x2);
    VecSetSizes(x2,PETSC_DECIDE,n2);
    VecSetType(x2, VECCUDA);

    VecDuplicate(x1,&b1);
    // VecDuplicate(x2,&aux2);
    VecDuplicate(x1,&u1);
    VecDuplicate(x1,&r1);
    VecDuplicate(x1,&p1);
    VecDuplicate(x1,&Ap1);

    VecDuplicate(x2,&b2);
    // VecDuplicate(x2,&aux1);
    VecDuplicate(x2,&u2);
    VecDuplicate(x2,&r2);
    VecDuplicate(x2,&p2);
    VecDuplicate(x2,&Ap2);

    VecCreate(PETSC_COMM_WORLD,&aux1);
    VecSetSizes(aux1,PETSC_DECIDE,n1);
    VecSetType(aux1, VECCUDA);

    VecCreate(PETSC_COMM_WORLD,&aux2);
    VecSetSizes(aux2,PETSC_DECIDE,n2);
    VecSetType(aux2, VECCUDA);



    VecGetOwnershipRange(x1,&rstart1,&rend1);
    VecGetLocalSize(x1,&nlocal1);

    VecGetOwnershipRange(x2,&rstart2,&rend2);
    VecGetLocalSize(x2,&nlocal2);

    // printf("%n %n \n", (int)nlocal1, (int)nlocal2);

    MatCreate(PETSC_COMM_WORLD,&A11);
    MatSetSizes(A11,PETSC_DECIDE,PETSC_DECIDE,n1,n1);
    MatSetType(A11, MATAIJCUSPARSE);
    MatSetUp(A11);

    MatCreate(PETSC_COMM_WORLD,&A22);
    MatSetSizes(A22,PETSC_DECIDE,PETSC_DECIDE,n2,n2);
    MatSetType(A22, MATAIJCUSPARSE);
    MatSetUp(A22);

    MatCreate(PETSC_COMM_WORLD,&A12);
    MatSetSizes(A12,PETSC_DECIDE,PETSC_DECIDE,n1,n2);
    MatSetType(A12, MATAIJCUSPARSE);
    MatSetUp(A12);

    MatCreate(PETSC_COMM_WORLD,&A21);
    MatSetSizes(A21,PETSC_DECIDE,PETSC_DECIDE,n2,n1);
    MatSetType(A21,MATAIJCUSPARSE);
    MatSetUp(A21);

    set_mat_values(rstart1, rend1, n1, A11);
    set_mat_values(rstart2, rend2, n2, A22);

    if (rstart2==0){
        col[0] = n1-1; value[0] = -1.0;
        i=0;
        MatSetValues(A21,1,&i,1,col,value,INSERT_VALUES);
    }
    MatAssemblyBegin(A21,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(A21,MAT_FINAL_ASSEMBLY);

    if (rstart1==0){
        col[0] = 0; value[0] = -1.0;
        i = n1-1;
        MatSetValues(A12,1,&i,1,col,value,INSERT_VALUES);
    }
    MatAssemblyBegin(A12,MAT_FINAL_ASSEMBLY); MatAssemblyEnd(A12,MAT_FINAL_ASSEMBLY);

    // MatView(A12,   PETSC_VIEWER_STDOUT_WORLD );
    // MatView_spit(A11, A12, A21, A22);


    // PetscFinalize();
    // return 0 ;

    clock_t t, t2 = 0;

    clock_t bt,et;
    cb = clock();
    
    // get the b vector
    t= VecSet_split(u1, u2, one);
    t+= MatMult_split(A11,A12,A21,A22,u1,u2,b1,b2,aux1,aux2);

    if(printflg) VecView_split(b1,b2);

    VecSetRandom(x1, NULL);
    VecSetRandom(x2, NULL);

    t+= MatMult_split(A11,A12,A21,A22,x1,x2,r1,r2,aux1,aux2);
    t+= VecAXPY_split(r1, r2, minusone, b1, b2);

    t+= VecSet_split(p1, p2, zero);
    t+= VecAXPY_split(p1, p2, minusone, r1, r2);

    for (i=0;i<iters;++i) {
        t+= VecDot_split(r1, r2, r1, r2, &rdot);
        t+= MatMult_split(A11,A12,A21,A22,p1,p2,Ap1,Ap2,aux1,aux2);
        t+= VecDot_split(p1,p2,Ap1,Ap2,&pAp);

        bt = clock();
        alpha = rdot/pAp;
        et = clock();
        t2+= (et-bt);


        t+= VecAXPY_split(x1, x2, alpha, p1, p2);
        t+= VecAXPY_split(r1, r2, alpha, Ap1, Ap2);

        t+= VecDot_split(r1, r2, r1, r2, &val);

        bt = clock();
        beta = val/rdot;
        et = clock();
        t2+= (int)(et-bt);


        t+= VecAXPBY_split(p1, p2, minusone, beta, r1, r2);
    }

    ce = clock();


    VecAXPY(x1,-1.0,u1);
    VecNorm(x1,NORM_2,&norm1);

    VecAXPY(x2,-1.0,u2);
    VecNorm(x2,NORM_2,&norm2);

    norm1 = norm1*norm1 + norm2*norm2;

    double total1 = (double)(ce - cb) / CLOCKS_PER_SEC;
    double maxTime = (double)t/CLOCKS_PER_SEC;
    double smallMaxTime = (double)t2/CLOCKS_PER_SEC;

    double finalTime = maxTime + smallMaxTime;
    PetscPrintf(PETSC_COMM_WORLD,"Time Elapsed : %lf, t = %lf , t2= %lf , total =  %lf", (double)total1,  maxTime , smallMaxTime, finalTime);
    if(printflg) VecView_split(x1, x2);

    return 0;

}