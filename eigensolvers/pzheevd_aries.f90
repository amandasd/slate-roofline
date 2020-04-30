module helper
  implicit none

  contains

    elemental subroutine convert_to_int(str, i, stat)

      character(len=*), intent(in) :: str
      integer, intent(out) :: i, stat

      read(str, *, iostat=stat) i

    end subroutine convert_to_int

end module helper

module ariescounters
interface
   !void InitAriesCounters(char *progname, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
   subroutine f_InitAriesCounters(progname, my_rank, reporting_rank_mod, AC_event_set, AC_events, AC_values, AC_event_count) &
   bind(C,name='InitAriesCounters')
   use, intrinsic :: iso_c_binding
      character(c_char) :: progname(*)
      integer(c_int), value :: my_rank, reporting_rank_mod
      integer(c_int) :: AC_event_set
      type(c_ptr), dimension(:), allocatable :: AC_events
      type(c_ptr) :: AC_values 
      integer(c_int) :: AC_event_count
   end subroutine f_InitAriesCounters

   !void StartRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
   subroutine f_StartRecordAriesCounters(my_rank, reporting_rank_mod, AC_event_set, AC_events, AC_values, AC_event_count) &
   bind(C,name='StartRecordAriesCounters')
   use, intrinsic :: iso_c_binding
      integer(c_int), value :: my_rank, reporting_rank_mod
      integer(c_int) :: AC_event_set
      type(c_ptr), dimension(:), allocatable :: AC_events
      type(c_ptr) :: AC_values
      integer(c_int) :: AC_event_count
   end subroutine f_StartRecordAriesCounters

   !void EndRecordAriesCounters(int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
   subroutine f_EndRecordAriesCounters(my_rank, reporting_rank_mod, AC_event_set, AC_events, AC_values, AC_event_count) &
   bind(C,name='EndRecordAriesCounters')
   use, intrinsic :: iso_c_binding
      integer(c_int), value :: my_rank, reporting_rank_mod
      integer(c_int) :: AC_event_set
      type(c_ptr), dimension(:), allocatable :: AC_events
      type(c_ptr) :: AC_values
      integer(c_int) :: AC_event_count
   end subroutine f_EndRecordAriesCounters

   !void FinalizeAriesCounters(MPI_Comm *mod16_comm, int my_rank, int reporting_rank_mod, int* AC_event_set, char*** AC_events, long long** AC_values, int* AC_event_count)
   subroutine f_FinalizeAriesCounters(f_mod_comm, my_rank, reporting_rank_mod, AC_event_set, AC_events, AC_values, AC_event_count) &
   bind(C,name='FinalizeAriesCounters')
   use, intrinsic :: iso_c_binding
   use mpi_f08
      type(MPI_Comm) :: f_mod_comm
      integer(c_int), value :: my_rank, reporting_rank_mod 
      integer(c_int) :: AC_event_set
      type(c_ptr), dimension(:), allocatable :: AC_events
      type(c_ptr) :: AC_values 
      integer(c_int) :: AC_event_count
  end subroutine f_FinalizeAriesCounters
end interface
end module ariescounters

program pzheevd_aries
   use mpi_f08
   use helper
   use ariescounters
   use, intrinsic :: iso_c_binding

   implicit none
   !modify for specific problem
   integer, parameter :: MY_NPROW      = 2
   integer, parameter :: MY_NPCOL      = 2
   integer, parameter :: MY_N          = 1024
   integer, parameter :: MY_NB         = 256
   integer, parameter :: MY_IL         = 1             ! lower bound for eigenval num
   integer, parameter :: MY_IU         = 1024          ! upper bound for eigenval num

   integer, parameter :: ELS_TO_PRINT  = 5             ! arbitrary els to print out
 
   integer   :: info = 0
   character :: jobz = 'V'
   character :: uplo = 'L'
   integer   :: ln = MY_N
   integer   :: lnbrow = MY_NB
   integer   :: lnbcol
   integer   :: m, nz
   integer   :: err, ierror
 
   integer   :: il = MY_IL, iu = MY_IU
 
   real(kind=8) :: dummyL, dummyU
   real(kind=8) :: t1, t2
  
   integer :: lnprow = MY_NPROW
   integer :: lnpcol = MY_NPCOL

   integer :: myrow, mycol
   integer :: ia=1, ja=1, iz=1, jz=1

   integer :: desca(15), descz(15)
  
   integer :: ctxt_sys
   integer :: moneI = -1, zeroI = 0, oneI = 1
   integer(c_int) :: rank, size
   integer :: llda, i

   character(len=9) :: procOrder = "Row-major"
   character(len=10), dimension(5) :: argc

   !ADDED DEFINITIONS
   integer :: ml, nl
   integer :: iseed(4)
   integer :: numEle

   complex(kind=8), allocatable :: a_ref(:,:), z(:,:)
   real(kind=8), allocatable :: w(:)
   complex(kind=8), allocatable ::  work(:)
   real(kind=8), allocatable :: rwork(:)
   integer,      allocatable :: iwork(:)
   real(kind=8) :: temp(2), rtemp(2)   ! I THINK THIS COULD ONLY BE rtemp(1)
   integer :: liwork, lwork, lrwork
   integer :: my_rank, j

   !.. External Functions ..
   integer, external ::   numroc

   !AriesNCL variables 
   integer(c_int) :: nodes
   integer :: r
   integer, allocatable :: members(:)
   type(MPI_Comm) :: mod_comm
   type(MPI_Group) :: mod_group, group_world
   
   character(kind=c_char,len=:), allocatable :: progname
   integer :: arglen

   character(len=4) :: str_nodes, str_rpn
   character(len=6) :: str_ln
   character(len=:), allocatable :: filename
   
   integer(c_int) :: AC_event_set !int in C
   type(c_ptr), dimension(:), allocatable :: AC_events !char ** in C 
   type(c_ptr) :: AC_values !long long * in C
   integer(c_int) :: AC_event_count !int in C

   ! Initialize MPI and BLACS
   call MPI_Init(err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if
   call MPI_Comm_size(MPI_COMM_WORLD, size, err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if
   call MPI_Comm_rank(MPI_COMM_WORLD, rank, err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if

   !print *, rank
   do i = 1, 5
      call get_command_argument(i, argc(i))
   end do

   call convert_to_int(argc(1), ln, err)
   call convert_to_int(argc(2), lnprow, err)
   call convert_to_int(argc(3), lnpcol, err)
   call convert_to_int(argc(4), lnbrow, err)
   call convert_to_int(argc(5), nodes, err)
   lnbcol = lnbrow

   !if (rank == 0) then
   !   print *, "ln =", ln
   !   print *, "lnprow =", lnprow
   !   print *, "lnpcol =", lnpcol
   !   print *, "lnbropw =", lnbrow
   !   print *, "nodes =", nodes
   !end if

   !AriesNCL 
   !Since we only want to do a gather on every n'th rank, we need to create a new MPI_Group
   allocate( members(nodes) )
   do r = 0, nodes-1
      members(r+1) = r*(size/nodes)
   end do
   call MPI_Comm_group(MPI_COMM_WORLD, group_world, err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if
   call MPI_Group_incl(group_world, nodes, members, mod_group, err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if
   call MPI_Comm_create(MPI_COMM_WORLD, mod_group, mod_comm, err)
   if (err .ne. MPI_SUCCESS) then
      print*, "There is a problem in the MPI_Init function."
   end if

   !call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, err)
   !if (err .ne. MPI_SUCCESS) then
   !   print*, "There is a problem in the MPI_Init function."
   !end if

   call get_command_argument(0,length=arglen)
   allocate(character(arglen) :: progname)
   call get_command_argument(0,value=progname)

   write(str_nodes,*) nodes 
   str_nodes = adjustl(str_nodes)
   write(str_rpn,*) size/nodes 
   str_rpn = adjustl(str_rpn)
   write(str_ln,*) ln 
   str_ln = adjustl(str_ln)
   allocate(character(len=len(progname)+len(str_nodes)+len(str_ln)+len(str_rpn)+3) :: filename)
   filename = progname//'-'//trim(str_nodes)//'-'//trim(str_ln)//'-'//trim(str_rpn)

   call blacs_get(moneI, zeroI, ctxt_sys)
   !print *, ctxt_sys
   call blacs_gridinit(ctxt_sys, procOrder, lnprow, lnpcol)
   call blacs_gridinfo(ctxt_sys, lnprow, lnpcol, myrow, mycol)
   !print *, myrow, mycol
  
   if (myrow .eq. -1) then
      !print *, ctxt_sys
      print *, "Failed to properly initialize MPI and/or BLACS!"
      call MPI_Finalize(err)
      stop
   end if

   !Allocate my matrices now
   ml = numroc(ln, lnbrow, myrow, zeroI, lnprow)
   nl = numroc(ln, lnbrow, mycol, zeroI, lnpcol)
   llda = ml

   call descinit(desca, ln, ln, lnbrow, lnbrow, zeroI, zeroI, ctxt_sys, llda, info)
   call descinit(descz, ln, ln, lnbrow, lnbrow, zeroI, zeroI, ctxt_sys, llda, info)
  
   allocate( a_ref(ml,nl) )
   allocate( z(ml,nl) )
   allocate( w(ln) )

   iseed(1) = myrow
   iseed(2) = mycol
   iseed(3) = mycol + myrow*lnpcol
   iseed(4) = 1
   if (iand(iseed(4), 2) == 0) then
      iseed(4) = iseed(4) + 1
   end if

   numEle = ml*nl

   !write(6,*) iseed, numEle
   call zlarnv(oneI, iseed, numEle, a_ref)
   !call zlarnv(oneI, iseed, numEle, z)      ! might not be necessary
  
   !do i=1, ml
   !   do j=1, ml
   !      write(6,*) "rank=", rank, "a_ref(",i,j,")=", a_ref(i,j)
   !   end do
   !end do

   !print *, ml, nl

   t1 = MPI_WTIME()

   !AriesNCL 
   call f_InitAriesCounters(filename, rank, size/nodes, AC_event_set, AC_events, AC_values, AC_event_count)
   call f_StartRecordAriesCounters(rank, size/nodes, AC_event_set, AC_events, AC_values, AC_event_count)

   call pzheevd(jobz, uplo, ln, a_ref, ia, ja, desca, w, z, iz, jz, descz, temp, moneI, rtemp, moneI, liwork, moneI, info)
   lwork     = temp(1)
   allocate( work(lwork) )
   lrwork    = rtemp(1)
   allocate( rwork(lrwork) )
   allocate( iwork(liwork) )
   !print *, lwork, lrwork, liwork
   call pzheevd(jobz, uplo, ln, a_ref, ia, ja, desca, w, z, iz, jz, descz, work, lwork, rwork, lrwork, iwork, liwork, info)
   if (info /= 0) then
      write(6,*) "PZHEEVD returned non-zero info val of ", info
   end if

   !AriesNCL 
   call f_EndRecordAriesCounters(rank, size/nodes, AC_event_set, AC_events, AC_values, AC_event_count)
   call f_FinalizeAriesCounters(mod_comm, rank, size/nodes, AC_event_set, AC_events, AC_values, AC_event_count)

   t2 = MPI_WTIME()

   call MPI_Comm_rank(MPI_COMM_WORLD, my_rank, ierror)
   call blacs_gridexit(ctxt_sys)
   call blacs_exit(zeroI)
   !print *, my_rank, err

   if (rank == 0) then
      print *, "PZHEEVD time: ", t2 - t1
   end if

   !if (rank == 0 ) then
   !   do j=1, ELS_TO_PRINT
   !      write(6,*) "w(", j, ")=", w(j), "z(", j, ")=", z(j,1)
   !   end do
   !end if

   deallocate( a_ref, z, w )
   deallocate( work, rwork, iwork )

   !call MPI_Finalize(err)

end program pzheevd_aries

