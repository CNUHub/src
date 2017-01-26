c ******************** file: gammaz.f ********************
	program gammaz

c       .1 Header has 2 lines of descriptive text
c      
c       .2 Results not printed to lineprinter.
c          +/- searches are not in seperate files.
c          Only 1 data file.
c
c          Commented out the stop command after the 50/25 checks
c
c	History: Originally written by Lewis Geer; July 17, 1987
c	         Reworked by Wayne Peterson; January 26, 1993
c
c	Intent:  To statistically analyze maxima generated by a search
c                program and to test the significance of the maxima.
c
c	integer*4 auxlu		! auxlu not used

	integer*4 userin        ! Unit number of the input device
                                ! [set to unit 5 = keyboard]

	integer*4 userout       ! Unit number of the output device
                                ! [set to unit 6 = screen]

	integer*4 disk          ! Unit number of the output file
                                ! [set to unit 8 = fname]

	integer*4 printer

	common /usrio/ userin, userout
c
c	auxlu not used
c
c	data auxlu, userin, userout, disk, printer /1, 2, 3, 8, 9/

	data userin, userout, disk, printer /5, 6, 8, 9/

	character*80 q          ! Holds the questions that are passed to
                                ! yesno and gtstring

	character*80 fname      ! The name of the input file associated 
                                ! with unit "disk".
                                ! [user input]

	real p1(3)

	real p2(3)

	real prob(3)

	real tmp                ! Used to hold intermediate values,
                                ! swap values etc.

	real cutoff             ! The significance level to stop
                                ! outputting points.
                                ! [user input]

	real s1

	real s2

	real mean               ! The mean of the magnitude values
                                ! of the (x,y,z,mag) data values.

	real minus              ! Used to indicate if working with
                                ! positive or negative search files.

	real ar(4,200000)        ! The rows of ar hold the (x,y,z,mag)
                                ! data values:
                                !    ar(1, ) holds the x position
                                !    ar(2, ) holds the y position
                                !    ar(3, ) holds the z position
                                !    ar(4, ) holds the magnitude at
				!            (x,y,z)

	real moment(3,3)

	real qrab

	real q1

	real q2

	real mid                ! Indicates where the magnitude
                                ! distribution is split:
                                !    0 => split at 0
                                !    1 => split at the mean of the
                                !         magnitude values.

	real kurt(3,27)         ! Holds a table for testing kurtosis.
                                ! Derived from table 34c of Tables for
                                ! Statisticians and Biometricians by
                                ! E.S. Pearson
                                !    kurt(1, ) holds the sample size
                                !    kurt(2, ) holds the 1% significance
                                !    kurt(3, ) holds the 5% significance

	real skew(3,21)         ! Holds a table of significance for
                                ! gamma 1. Derived from table 34b of
                                ! Tables for Statisticians and
                                ! Biometricians by E.S. Pearson
                                !    skew(1, ) holds the sample size
                                !    skew(2, ) holds the 1% significance
                                !    skew(3, ) holds the 5% significance 

	real g2(3)		! gamma 2 statistics
                                !    g2(1): bottom of curve
                                !    g2(2): all points
                                !    g2(3): top of curve

	real t2(3)		! gamma 2 t-statistics
                                !    t2(1): bottom of curve
                                !    t2(2): all points
                                !    t2(3): top of curve

	real sd(3)		! standard deviation of magnitudes
                                !    sd(1): bottom of curve
                                !    sd(2): all of curve
                                !    sd(3): top of curve

	real g1			! Gamma 1 statistic of all points

	real t1			! Gamma 1 t-staistic of all points

	integer i		! Loop index

	integer j		! Loop index

	integer k		! Loop index

	integer n(3)		! Number of data points
                                !    n(1): below mean
				!    n(2): total
				!    n(3): above mean

	integer npos		! Number of positive mag data points

	integer nneg		! Number of negative mag data points

	integer temp		! Temporary variable

	integer out             ! Set to printer, disk, or user out

	logical*1 lpt           ! Print out the results on a 
                                ! lineprinter? (true/false)
                                ! [user input]

	logical*1 ans		! Holds the answer to yes/no questions

	logical*1 split		! Are positive and negative searches
				! in seperate files? (true/false)
				! [user input]

	logical*1 dsk           ! Do you wish to put the output in a
                                ! file? (true/false)
				! [user input]

	common /buf/ ar, n, q, minus

	data n /3*0/
	data moment /9*0.0/
c
c	Formats
c

7	format(a, e10.4)
8	format(a, e10.4, a, e10.4)
9	format(6(e10.4,1x))
	
c
c	Table for testing kurtosis.  Derived from table 34c of
c       Tables for Statisticians and Biometricians by E.S. Pearson
c
c	   kurt(1,n) = sample size
c	   kurt(2,n) = 1% significance
c	   kurt(3,n) = 5% significance
c
	data kurt /
     :		50.,	4.88,	3.99,	75.,	4.59,	3.87,
     : 		100.,	4.39,	3.77,	125.,	4.24,	3.71,
     :		150.,	4.13, 	3.65, 	200.,	3.98,	3.57,
     :		250.,	3.87,	3.52,	300.,	3.79,	3.47, 
     :		350.,	3.72,	3.44,	400.,	3.67,	3.41,
     :		450.,	3.63,	3.39, 	500.,	3.60,	3.37, 
     :		550.,	3.57,	3.35, 	600.,	3.54,	3.34,
     :		650.,	3.52,	3.33, 	700.,	3.50,	3.31,
     :		750.,	3.48,	3.30, 	800.,	3.46,	3.29, 
     :		850.,	3.45,	3.28,	900.,	3.43,	3.28,
     :		950.,	3.42,	3.27, 	1000.,	3.41,	3.26,
     :		1200.,	3.37,	3.24,	1400.,	3.34,	3.22, 
     :		1600.,	3.32,	3.21,	1800.,	3.30,	3.20, 
     :		2000.,	3.28,	3.18	/
c
c	Table of significance for gamma 1. Derived from table 34b of
c       Tables for Statisticians and Biometricians by E.S. Pearson
c
c	   skew(1,n) = sample size
c	   skew(2,n) = 1% significance
c	   skew(3,n) = 5% significance
c
	data skew /
     :		25.,	1.061,	0.711,	30.,	0.982,	0.661, 
     :		35.,	0.921,	0.621, 	40.,	0.869,	0.587, 
     :		45.,	0.825,	0.558, 	50.,	0.787,	0.533, 
     :		60.,	0.723,	0.492, 	70.,	0.673,	0.459, 
     :		80.,	0.631,	0.432, 	90.,	0.596,	0.409, 
     :		100.,	0.567,	0.389, 	125.,	0.508,	0.350, 
     :		150.,	0.464,	0.321, 	175.,	0.430,	0.298, 
     :		200.,	0.403,	0.280, 	250.,	0.360,	0.251, 
     :		300.,	0.329,	0.230, 	350.,	0.305,	0.213, 
     :		400.,	0.285,	0.200, 	450.,	0.269,	0.188, 
     :		500.,	0.255,	0.179	/
c
c	This call to termset has been commented out. It is not known
c	how this was used to set up terminals.
c
c	call termset(auxlu)
c
	write (userout, *)
	write (userout, *) '                    GammaZ version 1.0.1'
	write (userout, *)
c	n(1)=0;
c	n(2)=0;
c	n(3)=0;


c
c	Prompt the user:
c       Do you wish to print out the results on the lineprinter?
c       [Default:Yes]
c
c	q = 'Do you wish to print out the results on the lineprinter?'
c	lpt = .true.
c	call yesno(q, lpt)
c
c	Prompt the user:
c	Do you wish to put the output in a file? [Default: No]
c
	q = 'Do you wish to put the output in a file?'
	dsk = .false.	
	call yesno(q, dsk)
c
c	If a output file is requested then prompt the user for a file
c       name and open the file.
c
	if (dsk) then
820	   continue
	   q = 'What is the filename?'
	   fname = ' '
	   call gtstring(q, fname)
	   open (disk, file = fname, form = 'formatted', status = 'new',
     :                 err = 800)
	   goto 810
800	   continue
	   write(userout, *)
     :          'Error: file name is not legal or file exists!'
	   goto 820
810        continue
	endif
c
c	Prompt the user about where they wish to split the data.
c       They can split it at 0 or at the mean. [Default: split at 0]
c
	write(userout, *)
	q='Do you wish to split the data at 0 (as opposed to the mean)?'
	ans = .true.	
	call yesno(q, ans)
	
	if (ans) then
	   mid = 0.
	else
	   mid = 1. 
	endif

c	write (userout, *)
c	q = 'Are positive and negative searches in seperate files?'
	split = .false.		! Error in 87 version corrected.
c	call yesno(q, split)

c	write (userout, *) 'Enter the name(s) of the data file(s) '//
c    :                     '[return to end]:'
c	write (userout, *)
c	write (userout, *) 'Enter the name of the data file:'
	write (userout, *)

100	continue
	if (split) then
	   q = 'What is the name of the positive search file?'
	   minus = 1.
	   call rfile
	   if (minus .eq. 0) goto 111
	   q = 'What is the name of the negative search file?'
	   minus = -1.
	   call rfile
	else
	   q = 'What is the name of the search file?'
	   minus = 1.
	   call rfile
	   if (minus .eq. 0.) goto 111
	endif
c	goto 100

111	continue
	if (n(2) .eq. 0) then
	   write (userout, *) 'Error: no data points were read in!'
	   stop
	endif
c
c	Data Analysis
c
	mean = 0.
	npos = 0
	nneg = 0
	
	do 200 i = 1,n(2)
	   mean = mean+ar(4,i)
	   if (ar(4,i) .gt. 0) then
	      npos = npos+1
	   elseif (ar(4,i) .lt. 0.) then
	      nneg = nneg+1
	   endif
200	continue
	
	mean = mean/n(2)
	
	do 250 i = 1,n(2)
  	   temp = 2+sign(1.,ar(4,i)-mid*mean)
	   n(temp) = n(temp)+1
	   do 275 j = 1,3
	      moment(2,j) = moment(2,j)+(ar(4,i)-mid*mean)**(j+1)
	      moment(temp,j) = moment(temp,j)+(ar(4,i)-mid*mean)**(j+1)
275	   continue
250	continue
	
	do 290 i = 1,3
	   do 295 j = 1,3
	      moment(i,j) = moment(i,j)/n(i)
295	   continue
290	continue

	do 300 i = 1,3
	   sd(i) = sqrt(moment(i,1))
	   g2(i) = (moment(i,3)/sd(i)**4)-3.0
	   tmp = 24.*n(i)*(n(i)-1.)**2
	   tmp = tmp/((n(i)-3.)*(n(i)-2.)*(n(i)+3.)*(n(i)+5.))
	   tmp = sqrt(tmp)
	   t2(i) = g2(i)/tmp
300	continue

	g1 = moment(2,2)/sd(2)**3
	t1 = g1/sqrt((6.*n(2)*(n(2)-1.))/((n(2)-2.)*(n(2)+1.)*(n(2)+3.)))

	do 666 j = 1,3
	   if (n(j) .lt. 50) then
	      write (userout, *) 'There are less than 50 points.'
	      write (userout, *) 'This is not enough points to do a'//
     :                           ' proper analysis.'
c	      stop
	   endif

	   temp = 1
	   if (n(j) .gt. 2000) then
	      prob(j) = 0.
	      goto 390
	   endif

	   do 310 i = 2,27
	      if (kurt(1,i) .ge. n(j)) goto 320
310	   continue

320	   continue

	   s2 = kurt(1,i)-n(j)
	   s1 = n(j)-kurt(1,i-1)
           p1(j) = (s1*kurt(2,i)+s2*kurt(2,i-1))/(kurt(1,i)-kurt(1,i-1))
	   p2(j) = (s1*kurt(3,i)+s2*kurt(3,i-1))/(kurt(1,i)-kurt(1,i-1))
	   p1(j) = p1(j)-3.
	   p2(j) = p2(j)-3.
	   if (g2(j) .ge. p1(j)) then
	      prob(j) = 1.
	   else if (g2(j) .ge. p2(j)) then
	      prob(j)  = 5.
	   else
	      prob(j) = 100.
	   endif
666	continue

	if (n(2) .lt. 25) then
	   write(userout, *) 'There are less than 25 points.'
	   write(userout, *) 'This is not enough points to do a proper'//
     :                       ' analysis.'
c	   stop
	endif

	temp = 1
	if (n(2) .gt. 500) then
	   qrab = 0.
	   goto 390
	endif

	do 373 i = 2,21
	   if (skew(1,i) .ge. n(2)) goto 374
373	continue

374	continue
	s2 = skew(1,i)-n(2)
	s1 = n(2)-skew(1,i-1)
	q1 = (s1*skew(2,i)+s2*skew(2,i-1))/(skew(1,i)-skew(1,i-1))
	q2 = (s1*skew(3,i)+s2*skew(3,i-1))/(skew(1,i)-skew(1,i-1))
	if (g1.ge. q1) then
	   qrab = 1.
	else if (g1 .ge. q2) then
	   qrab = 5.
	else
	   qrab = 100.
	endif

390	continue
	out = userout

400	continue
	write (out, *)
	write (out, *)
	write (out, *) '                    GammaZ version 1.0.1'
	write (out, *)
	write (out, *) 'Number of data points:		          ', n(2)
	write (out, *) 'Number of positive data points:	          ', npos
	write (out, *) 'Number of negative data points:	          ', nneg
	write (out, *)
	write (out, 7) 'Mean:				          ', mean

	if (mid .eq. 1) then
	   write (out, *) 'Number of points above the mean:       ', n(3)
	   write (out, *) 'Number of points below the mean:       ', n(1)
	   write (out, *)
	   write (out, *) 'Distribution is split at the mean.'
	else
	   write (out, *)
	   write (out, *) 'Distribution is split at 0.'
	endif

	write (out, *)
	write (out, 7) 'Standard deviation of all points:         ',
     :                 sd(2)
	write (out, 7) 'Stnd. dev. of the top of the curve:       ',
     :                 sd(3)
	write (out, 7) 'Stnd. dev. of the bot. of the curve:      ',
     :                 sd(1)
	write (out, *)
	write (out, 7) 'Gamma 1 statistic of all points:          ', g1
	write (out, 7) 'Gamma 1 t-statistic for all points:       ', t1
	write (out, *)
	
	if (qrab .eq. 0.0) then
	   write (out, *) 'Too many points to test skew.'
	else
	   if (qrab .gt. 5.0) then
	      write (out, *) 'There is no significant skew.'
	   else
	      write (out, *) 'There is a better than ', qrab, '% '//
     :                       'significance of skew.'
	   endif
	   write (out, *) 'For ',n(2),' points significance levels are:'
	   write (out, 8) '  .01 : ', q1,'   .05 : ', q2
	endif

	write (out, *)
	write (out, 7) 'Gamma 2 statistic of all points:           ',
     :                 g2(2)
	write (out, 7) 'Gamma 2 of the top of the curve:           ',
     :                 g2(3)
	write (out, 7) 'Gamma 2 of the bot. of the curve:          ',
     :                 g2(1)
	write (out, *)
	write (out, 7) 'Gamma 2 t-statistic for all points:        ',
     :                 t2(2)
	write (out, 7) 'Gamma 2 t-statistic for the top points:    ',
     :                 t2(3)
	write (out, 7) 'Gamma 2 t-statistic for the bottom points: ',
     :                 t2(1)
	write (out, *) 'Nota bene: Do NOT use the t-statistic for '//
     :                 'less than 2000 points.'
	write (out, *)
	write (out, *) 'Kurtosis:'
	write (out, *)
	
	do 888 j = 1,3
	   if (j .eq. 1) then
	      write (out, *) 'Bottom of curve:'
	   else if (j .eq. 2) then
	      write (out, *) 'All points:'
	   else
	      write (out, *) 'Top of curve:'
	   endif

	   if (prob(j) .eq. 0.0) then
	      write (out, *) 'Too many points to test kurtosis, '//
     :                       'please use t-statistic.'
	   else
	      if (prob(j) .gt. 5) then
	         write (out, *) 'There is no significant leptokurtosis.'
	      else
	         write (out, *) 'There is a better than ', prob(j), 
     :                          '% significance of leptokurtosis.'
	      endif

	      write (out, *) 'For ', n(j),
     :                       ' points significance levels are:'
	      write (out, 8) '   .01 : ', p1(j), '   .05 : ', p2(j)
	   endif
	   write (out, *)
888	continue

	if (lpt .and. out .ne. printer .and. out .ne. disk) then
	   out = printer
	   goto 400
	else if (dsk .and. out .ne. disk) then
	   out = disk
	   goto 400
	endif

	q = 'Do you wish to do a z test on the data?'
	if (prob(3) .eq. 0. .or. prob(3) .gt. 5.) then
	   ans = .false.
	else
	   ans = .true.
	endif
	call yesno(q, ans)
	if (.not. ans) then
	   stop
	endif
c
c	z test
c
	do 600 i = 1,n(2)
	   do 610 j = 2,n(2)
	      if (ar(4,j) .gt. ar(4,j-1)) then
	         do 650 k = 1,4
	            tmp = ar(k,j)
		    ar(k,j) = ar(k,j-1)
	            ar(k,j-1) = tmp
650	         continue
	      endif
610	   continue
600	continue

	cutoff = .05
	q = 'At what significance level do you wish to stop '//
     :       'output of points?'
        call getreal(q, cutoff, 0., 100.)
	out = userout

910	write (out, *)
	write (out, 7) 'Cutoff at ', cutoff
	write (out, *)
	write (out, *) 'x,     y,     z,     max,     z-score,'//
     :                 '     significance level'
	ans = .true.

	do 999 i = 1,n(2)
	   tmp = (1.-agauss(ar(4,i), mean*mid, sd(2)))
	   if (tmp .gt. cutoff) then
	      if (ans) then
	         write (out, *)
	         ans = .false.
	      endif
	      goto 999
	   endif
	   write (out, 9) (ar(j,i),j = 1,4),(ar(4,i)-mid*mean)/sd(2),tmp
999	continue

	if (lpt .and. out .ne. printer .and. out .ne. disk) then
	   out = printer
	   goto 910
	else if (dsk .and. out .ne. disk) then
	   out = disk
	   goto 910
	endif

	stop
	end
	      
c *************************
c
	subroutine rfile
c
c	Intent: To read in a f3d file
c
	real ar(4,200000)	! The rows of ar hold the (x,y,z,mag)
         			! data values:
				!    ar(1, ) holds the x position
				!    ar(2, ) holds the y position
				!    ar(3, ) holds the z position
				!    ar(4, ) holds the magnitude at
                                !            (x,y,z)

	real minus		! Used to indicate if working with
				! positive or negative search files.
	
	integer n(3)            ! Number of data points
				!    n(1): below mean
				!    n(2): total
				!    n(3): above mean

	integer i		! Loop index

	integer xx		! The first value on line 2 of the 
 				! f3d file.

	integer yy		! The second value on line 2 of the
				! f3d file.

	integer j		! Loop index

	character*80 q		! Holds the questions that are passed to
				! yesno and gtstring

	character*80 fname	! The name of the f3d input file.
 				! [user input]

	character*80 header	! Holds the line 1 header of the f3d
				! file named fname

	common /buf/ ar, n, q, minus

	integer*4 userin	! Unit number of the input device
				! [set to unit 5 = keyboard]

	integer*4 userout	! Unit number of the output device
				! [set to unit 6 = screen]

	integer*4 disk		! Unit number of the input device
				! [set to unit 7 = fname]

	common /usrio/ userin, userout

	data disk /7/

	fname = ' '
	call gtstring(q, fname)

	if (fname .eq. ' ' .and. minus .ne. -1.) then
	   minus = 0.
	   return
	endif

	open (disk, err = 2001, file = fname, status = 'old')
	read (disk, '(a80)') header
	write (userout, *) header
	read (disk, '(a80)') header
	write (userout, *) header
	read (disk, *) xx, yy
	if (yy .ne. 4 .and. yy .ne. 1) goto 2002

	write (userout, *)'reading from input file'
	do 2500 i = n(2)+1,n(2)+xx
	   if (yy .ne. 1) then
	      read (disk, *, err = 2002) (ar(j,i), j = 1,4)
	   else
	      read (disk, *, err = 2002) ar(4,i)
	      ar(1,i) = 0.
	      ar(2,i) = 0.
	      ar(3,i) = 0.
	   endif

	   if (minus .eq. -1. .and. ar(4,i) .gt. 0.) then
	      ar(4,i) = ar(4,i)*minus
	   endif

c	   if (ar(4,i) .eq. 0.) then
c	      write (userout, *)'Error: the data contains a peak at 0.0!'
c	      goto 2002
c	   endif
2500	continue

	n(2) = n(2)+xx
	close(disk)
	write (userout, *)'done reading from input file'
	return

2001	write (userout, *) 'File not found! File not read in.'
	return

2002	write (userout, *) 'Format of file incorrect! File not read in.'
	close (disk)
	return
	end

c *************************
c
	function agauss(x, averag, sigma)
c
c	Intent: To compute the integral of the Gaussian curve.
c	        Taken from Bevington, Data Reduction and Error Analysis
c	        for the Physical Sciences, pg. 48
c
	real x			! Range of integration.
				! The averag(+/-)z*sigma where
				! z = abs(x-averag)/sigma

	real averag		! The mean of the distribution

	real sigma		! The standard deviation of the
                                ! distribution

	double precision z
	double precision y2
	double precision term
	double precision sum
	double precision denom

	z = abs(x-averag)/sigma
	agauss = 0.

	if (z) 42, 42, 21

21	term = 0.7071067812*z
	sum = term
	y2 = (z**2)/2.
	denom = 1.

31	denom = denom+2.
	term = term*(y2*2./denom)
	sum = sum+term

	if (term/sum-1.d-10) 41, 41, 31

41	agauss = 1.128379167*sum*dexp(-y2)
42	return
	end

c *************************
c
	subroutine yesno(question, answer)
c	
c	Intent: To ask the yes/no question "question". 
c               Depending on the users response .true. or
c               .false. is returned. 
c
	character*80 question	! The yes/no question

	character*1  response	! The yes ('y' or 'Y')/no ('n' or 'N')
 				! answer [user input]

	logical*1 answer	! .true./.false. depending on response

	integer*4 userin	! Unit number of the input device
				! [set to unit 5 = keyboard]

	integer*4 userout	! unit number of the output device
				! [set to unit 6 = screen]

	common /usrio/ userin, userout

	write(userout, *) question
	read (userin, *) response

	if (response .eq. 'Y' .or. response .eq. 'y') then
	   answer = .true.
	else
c
c          Note: If the user enters anything but 'y' or 'Y'
c	         it is treated as a no answer.
c
	   answer = .false.
	endif

	return
	end

c *************************
c
	subroutine getreal(question, value, low, high)
c
c	Intent: The question "question" asks the user to supply a
c		real number answer.  If the user supplys an answer 
c		that does not lie in [low, high] then the question
c		is re-asked.
c
	character*80 question	! A question asking the user to supply 
				! a real number.
	
	integer*4 userin	! Unit number of the input device
				! [set to unit 5 = keyboard]

	integer*4 userout	! Unit number of the output device
				! [set to unit 6 = screen]

	common /usrio/ userin, userout

	real value		! The real value being asked for

	real low, high		! The legal range of values for 
				! value is [low, high]

	logical*1 valid         ! Is value holding a legal response?

	valid = .false.

100	continue
	write(userout, *) question
	read (userin, *) value
	if (low .le. value .and. value .le. high) goto 200
	write(userout, *) 'Warning: The value is out of range. '//
     :                    'Legal range is [', low, ',', high, ']'
	goto 100

200	continue
	return
	end

c *************************
c
	subroutine gtstring(question, value)
c
c	Intent: To ask the user a question that requires a
c		character string response.
c
	character*80 question	! The question

	character*80 value	! The character string response

	integer*4 userin	! Unit number of the input device
				! [set to unit 5 = keyboard]

	integer*4 userout	! Unit number of the output device
				! [set to unit 6 = screen]

	common /usrio/ userin, userout

	write (userout, *) question
	read  (userin, '(a80)') value
	return
	end