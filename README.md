###Start###
Use make file ( command $ make)  to generate the following Executable:
-"mikaboo.core.uarm"
-"mikaboo.stab.uarm"

Start the emulator uarm using command:$ uarm

Search the top bar for the machine configs button (the first one on
the left) and choose the core file you just created "mikaboo.core.uarm", and the Symbol Table "mikaboo.stab.uarm"

Save the configurations by clicking "Ok" 

Press the power button to switch on the machine, and if you want to view the code
in action, bring up Terminal by clicking 'Terminals' button then choose Terminal.

Press the play button to run the machine.

###P2P###

We have created a new test that has the abily to test each thread parallelly to each terminal:
To do this you have to use the command( $ make parallel)

###Links###
uArm Emulator http://mellotanica.github.io/uARM/

Documentazione Phase1 MiKKABoO  http://www.cs.unibo.it/~renzo/so/mikaboo/phase1.pdf

###Notes###
-we assume that uArm is in the following directory: /usr/include/
-we have added include stint.h in mikabooq.h to avoid problems made by uintptr_t
