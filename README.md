# CSAP_project


# Esecuzione simbolica di computazioni in virgola mobile  


## Sommario
L’esecuzione simbolica è una tecnica di program analysis molto nota ed utilizzata per generare automaticamente input in grado di esercitare diversi percorsi di esecuzione di un programma dove, analizzando istruzione per istruzione il codice del programma, è possibile costruire delle formule che rappresentano il flusso di esecuzione e che saranno poi elaborate da un Satisfiability Modulo Theories (SMT) solver.

La componente principale di questa tecnica sono i simboli, ovvero input non concreti di cui non è noto il valore iniziale, che costituiscono le espressioni e su cui vengono effettuate delle operazioni o imposte delle condizioni a seconda della parte di codice che è stata analizzata.
Studiando tutte le possibili combinazioni di valori che questi simboli possono assumere in un determinato punto del programma, è possibile scoprire comportamenti interessanti nell’esecuzione in risposta a specifici input, che potranno poi condurre alla scoperta di nuovi bug e vulnerabilità all’interno di un software.

In questa tesi, consideriamo l’esecuzione concolica: tale tecnica affianca all’esecuzione simbolica anche un’esecuzione concreta, potendo in ogni momento sostituire un valore concreto ad una formula simbolica quando quest’ultima risulta troppo complessa da risolvere per il solver.

Una limitazione comune a diversi esecutori concolici esistenti è il mancato supporto al ragionamento simbolico su operazioni in virgola mobile: nostro obiettivo sarà analizzare in dettaglio il funzionamento delle operazioni in virgola mobile per architettura x86 e x86_64 e, successivamente, discutere come integrare il supporto ad una parte di queste operazioni all’interno dell’esecutore concolico binario SymQEMU.


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