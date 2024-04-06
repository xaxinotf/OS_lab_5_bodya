#include <sys/msg.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// Оголошення структури повідомлень
struct msgbuf {
    long mtype;       // тип повідомлення
    int mvalue;       // значення, що передається в повідомленні
};

// Функція, що відправляє результат обчислення в чергу повідомлень
void send_result(int msgid, int result, int result_type) {
    struct msgbuf msg;
    msg.mtype = result_type;
    msg.mvalue = result;
    msgsnd(msgid, &msg, sizeof(msg.mvalue), 0);
}

// Функція f(x) перевіряє, чи є число простим
int f(int x) {
    sleep(1); // Імітація тривалої операції
    if (x < 2) return 0;
    for (int i = 2; i <= x / 2; i++) {
        if (x % i == 0) return 0; // Число не є простим
    }
    return 1; // Число є простим
}

// Функція g(x) визначає, чи число є степенем двійки
int g(int x) {
    sleep(3); // Імітація тривалої операції
    if (x < 1) return 0;
    return (x & (x - 1)) == 0 ? 1 : 0; // Перевірка на степінь двійки
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <integer value>\n", argv[0]);
        return -1;
    }

    int x = atoi(argv[1]);

    // Створення черги повідомлень
    int msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);

    if (fork() == 0) {
        // Дочірній процес для f(x)
        int result = f(x);
        send_result(msgid, result, 1);
        exit(0);
    }

    if (fork() == 0) {
        // Дочірній процес для g(x)
        int result = g(x);
        send_result(msgid, result, 2);
        exit(0);
    }

    // Очікування результатів від обох процесів
    struct msgbuf msg;
    int results[2] = {0}, count = 0;
    while (count < 2) {
        if (msgrcv(msgid, &msg, sizeof(msg.mvalue), 0, 0) > 0) {
            results[msg.mtype - 1] = msg.mvalue;
            count++;
        }
    }

    // Логічне І між результатами f(x) та g(x)
    int logic_and_result = results[0] && results[1];
    printf("f(x) && g(x) result: %d\n", logic_and_result);

    // Видалення черги повідомлень
    msgctl(msgid, IPC_RMID, NULL);

    return 0;
}
