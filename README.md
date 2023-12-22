# CSAP January Project


## Summary
The task is to create a multi-process log server, where each client is served by a "worker" process spawned by the main server.
After a connection has been established, the client sends messages to the assigned worker, which in turn writes the message received on a common file that is modified by other clients.


## Prerequisiti
* Versione modificata di SymCC, disponibile al seguente indirizzo: "https://github.com/ZioSaba/symcc"
* Versione modificata di SymQEMU, disponibile al seguente indirizzo: "https://github.com/ZioSaba/symqemu"

Il simbolo "**" scrive in grassetto
**Sono in grassetto**

Il simbolo "##" crea un titolo piccolo con divisorio
## Sono un titolo piccolo

Il simbolo "*" crea un elemento di una lista
* Elem_1
* Elem_2

Il simbolo "_" scrive in corsivo
_Sono in corsivo_

## Funzionamento

Il simbolo "```sh" consente di scrivere testo di shell
```sh
$make 
```

Il simbolo "###" crea un titolo piccolo senza divisorio
### Sono un titolo piccolissimo



# Signals in disastrOS - SO Project

## Summary
The goal of this project is to implement a simple signals system within disastrOS, a program provided by our professors. <br/>
Given the PCB of a generic process, a system call defined by us will be invoked at a specific time and will set a signal number for the aforementioned process.


## How?
An array of contexts has been added to the process PCB, in which each cell will contain the context associated with the same number as the received signal, along with a common stack that all the signal contexts will share. <br/>
We used two bitmaps to handle the signals, one for the received signals and one for the signal being served.
<br/>
We have defined a new syscall, called _internal_signal_, which will run at fixed intervals (defined in signal.h) and the function works as follows:
1. it analyzes which signal must be sent in that instant of time
2. we scan the ready_list to check if it's not empty
    * if not empty
        - we find the PCB of the last process in the ready_list
        - we check if that process is _init_, in which case the function will return
        - we check if the last signal received is the same as the one we are trying to send or the same signal is served, in which case the function will print a message
        - otherwise, we create a new context which will be used by the signal_handler
        - if the signals_mask is not set, we set the bit representing that signal we just sent
    * otherwise, we go to 3
3. we scan the waiting_list to check if it's not empty
    * if not empty
        - we find the PCB of the last process in the waiting_list
        - we check if that process is _init_, in which case the function will return
        - we check if the last signal received is the same as the one we are trying to send or the same signal
is served, in which case the function will print a message
        - otherwise, we create a new context which will be used by the signal_handler
        - if the signals_mask is not set, we set the bit representing that signal we just sent 
4. if 2 and 3 fail, the program will crash because **at least one process must be active**


Before each process starts running, we check if any of the signals are active by checking the _signals_ value in the PCB: if the value is not zero, we jump to the _signal main context_, otherwise the process proceeds in its execution. <br/>
Assuming that we are in the _signal main context_, we begin to scan the mask of the signals received until we find a cell with its bit set, then we move on to the context inside the cell of the array of contexts with the same index if the mask of the signal served is not set. 


## Signals defined
We have defined two signals:
1. _MovUp_: print an informative message and set a variable on the PCB to "true", which will be used the next time the process performs a wait
2. _Kill_: print an informative message meaning the process would be killed

## How to execute
Type in shell the following:
```sh
$ make
$ ./disastrOS_test
```


### Source files that have been changed:
- disastrOS_constants.h
- disastrOS_pcb.c
- disastrOS_pcb.h
- disastrOS_sleep.c
- disastrOS_spawn.c
- disastrOS_syscalls.h
- disastrOS.c
- disastrOS.h

### Added files:
- disastrOS_sendSignal.c
- sigKill.c
- sigMovUp.c
- signalMakeContext.c
- signals.h