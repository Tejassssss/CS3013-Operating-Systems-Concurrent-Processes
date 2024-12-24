Shueworld.c

    The program uses threads to represent individual shoes, with each shoe attempting to enter the stage. 
    A shoe type can only occupy the stage if there is space, and shoes of the same type must enter in groups of at least two. 
    Shoes can only enter if the stage is currently occupied by their distinct faction, or if the stage is empty.
    Synchronization is handled using mutex locks and condition variables. These ensure that shoes are added and removed from 
    the stage without conflicts. The stage_lock mutex guarantees mutual exclusion during stage operations, while condition 
    variables such as stage_open and need_friends manage the orderly arrival of shoes on the stage. 
    A record "count" records how many of each shoe have been on stage thus far. 
    When a shoe type has had its allotted time on stage, all shoes of that type are prevented from entering the stage. 
    Once all shoe types have filled their quota, a "free for all" round resets the counts, allowing all shoe types to re-enter. 
    The simulation runs continuously, with shoes randomly arriving and leaving the stage as the program progresses.

    Code was tested using a variety of seed.txt values with which we calculated the random values and made sure the outputs matched our results. 
    We used rigorous print statements to make sure the threads were working as intended.
