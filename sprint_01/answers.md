ex1.
a. && b.
Step 2. x = -2
Step 3. 2908; x = -2
Step 1. x = 2
Step 3. 0; x = 2
fork returns the PID of the child, thus the first condition that runs is the else for the parent, meaning step 2 followed by the parents's step 3
then the child process runs with the fork result 0 so step 1 and 3 run.

2.
a. Not counting the parent process, 6, counting the parent process 7
b. 
Parent
Child_1 | Child_2 | Child_3
Child_1_1 | Child_1_2
Child_1_1_1 |
c. 2 times, the parent process doesn't wait for the children.

3.
a.
a=5, b=3200, c=3199, d=2882 -> parent
a=5, b=0, c=3200, d=3199 -> child

a is 0 + 5
b is the fork result, for the parent its the child's PID, for the child its 0 
c is the PID of the process 3199 for the parent and 3199+1 for the child
d is the parent PID, for the parent its another process, for the child its the parents PID

4.
a. 2^n where n is the number of forks, 2^3 = 8
b. see ex4
c. see ex4
d. see ex4

5. see ex5
a. In the same priority level, the more processes that exist for a program the more cpu its given

6. see ex6
7. see ex7
8. see ex8, don't get the difference between this one and 7
9. see ex9
10. see ex10
11. see ex11
12. ex12.c -> unbounded, producer doesn't block, consumer blocks if buffer is empty
    ex12_2.c -> bounded, producer blocks if buffer if full, consumer blocks if buffer is empty
13. see ex13, macos dispatch used for easier testing
