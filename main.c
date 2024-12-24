#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

volatile sig_atomic_t sigusr1_received = 0;
volatile sig_atomic_t sigusr2_received = 0;

const int iterations = 42;

void sigusr1_handler(int signum) {
    sigusr1_received = 1;
}

void sigusr2_handler(int signum) {
    sigusr2_received = 1;
}

int main(void) {
    pid_t pid;
    struct sigaction sa1, sa2;

    sa1.sa_handler = sigusr1_handler;
    sa1.sa_flags = 0;
    sigemptyset(&sa1.sa_mask);
    if (sigaction(SIGUSR1, &sa1, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    sa2.sa_handler = sigusr2_handler;
    sa2.sa_flags = 0;
    sigemptyset(&sa2.sa_mask);
    if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        for (int i = 0; i < iterations; i++) {
            while (!sigusr1_received) {
                pause();
            }
            sigusr1_received = 0;
            printf("Child: Received Ping %d\n", i + 1);

            kill(getppid(), SIGUSR2);
        }
    } else {
        for (int i = 0; i < iterations; i++) {
            kill(pid, SIGUSR1);

            while (!sigusr2_received) {
                pause();
            }
            sigusr2_received = 0;
            printf("Parent: Received Pong %d\n", i + 1);
        }

        wait(NULL);
    }

    return 0;
}
