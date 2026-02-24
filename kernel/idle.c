#include <n7OS/cpu.h>
#include <n7OS/idle.h>

void idle(){
    for(;;){
        hlt(); // Met le CPU en pause jusqu'à la prochaine interruption (timer, clavier, etc.)
    }
}