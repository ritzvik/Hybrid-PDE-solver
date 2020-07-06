  program main
#include <petsc/finclude/petscksp.h>
  use petscksp
  implicit none

  Vec              x, b, u
  Mat              A
  PetscReal        norm, tol
  PetscInt         i,n,its,rstart,rend,nlocal
  PetscScalar      one, ksprtol, kspdefault
  PetscBool        flg
  PetscMPIInt      rank
  PetscScalar,pointer,dimension(:) :: value
  PetscInt,pointer,dimension(:) :: col

  KSP              ksp
  PC               pc
  PetscErrorCode   ierr

  ! set values
  n=10; one=1.0; tol=1000.*PETSC_MACHINE_EPSILON
  ksprtol = 1.0e-7
  kspdefault = -2.0

  call PetscInitialize(PETSC_NULL_CHARACTER,ierr)
  if (ierr .ne. 0) then
    print*,'Unable to initialize PETSc'
    stop
  endif

  call PetscOptionsGetInt(PETSC_NULL_OPTIONS,PETSC_NULL_CHARACTER,'-n',n,flg,ierr)
  call MPI_Comm_rank(PETSC_COMM_WORLD,rank,ierr)



  call VecCreate(PETSC_COMM_WORLD,x,ierr)
  call VecSetSizes(x,PETSC_DECIDE,n,ierr)
  call VecSetFromOptions(x,ierr)


  call VecDuplicate(x,b,ierr)
  call VecDuplicate(x,u,ierr)

  call VecGetOwnershipRange(x,rstart,rend,ierr)
  call VecGetLocalSize(x,nlocal,ierr)

  ! print*,rstart , rend, nlocal


  call MatCreate(PETSC_COMM_WORLD,A,ierr)
  call MatSetSizes(A,nlocal,nlocal,n,n,ierr)
  call MatSetFromOptions(A,ierr)
  call MatSetUp(A,ierr)


  allocate(col(0:2))
  allocate(value(0:2))

  if (rstart.eq.0) then
    rstart=1
    i=0    

    !col(0)=0; col(1)=1; value(0)=2.0; value(1)=-1.0;
    col = [0,1]
    value = [2.0,-1.0]
    call MatSetValues(A,1, i, 2, col, value, INSERT_VALUES, ierr)
  endif
  
  if (rend.eq.n) then
    rend=n-1
    i=n-1
    col = [n-2,n-1]
    value = [-1.0,2.0]
    call MatSetValues(A, 1, i, 2, col, value, INSERT_VALUES, ierr)
  endif

  deallocate(col)
  deallocate(value)

  allocate(value(0:3))
  allocate(col(0:3))
  value=[-1.0,2.0,-1.0]

  do 10 i = rstart, rend-1
    col = [i-1, i, i+1]
    call MatSetValues(A,1,i,3,col,value,INSERT_VALUES, ierr)
  10  continue

  deallocate(col)
  deallocate(value)

  call MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY,ierr);  call MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY,ierr)

  call VecSet(u,one, ierr)
  call MatMult(A,u,b, ierr)

  call KSPCreate(PETSC_COMM_WORLD,ksp,ierr)
  call KSPSetOperators(ksp,A,A, ierr)

  call KSPGetPC(ksp,pc,ierr)
  call PCSetType(pc,PCJACOBI,ierr)
  call KSPSetTolerances(ksp,ksprtol,kspdefault,kspdefault,-2,ierr)

  call KSPSetFromOptions(ksp,ierr)

  ! time this single line
  call KSPSolve(ksp,b,x,ierr)

  call KSPView(ksp,PETSC_VIEWER_STDOUT_WORLD, ierr)

  call VecAXPY(x,DBLE(-1.0),u,ierr)
  call VecNorm(x,NORM_2,norm,ierr)
  call KSPGetIterationNumber(ksp,its,ierr)

  call VecDestroy(x, ierr)
  call VecDestroy(u, ierr)
  call VecDestroy(b, ierr)
  call MatDestroy(A, ierr)
  call KSPDestroy(ksp, ierr)


  call PetscFinalize(ierr)

end