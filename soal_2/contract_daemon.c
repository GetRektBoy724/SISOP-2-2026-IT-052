#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

char expected_content[512];
const char *contract_filename = "contract.txt";
const char *log_filename = "work.log";

// Fungsi untuk mendapatkan timestamp saat ini
void get_timestamp(char *buffer, size_t size) {
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", info);
}

// Fungsi untuk menulis atau memulihkan contract.txt
void write_contract(int is_restored) {
    FILE *fp = fopen(contract_filename, "w");
    if (fp == NULL) return;

    char timestamp[64];
    get_timestamp(timestamp, sizeof(timestamp));

    if (is_restored) {
        snprintf(expected_content, sizeof(expected_content), 
                 "\"A promise to keep going, even when unseen.\"\nrestored at: %s\n", timestamp);
    } else {
        snprintf(expected_content, sizeof(expected_content), 
                 "\"A promise to keep going, even when unseen.\"\ncreated at: %s\n", timestamp);
    }

    fprintf(fp, "%s", expected_content);
    fclose(fp);
}

// Fungsi untuk menulis peringatan pelanggaran ke log
void write_violation_log() {
    FILE *fp = fopen(log_filename, "a");
    if (fp) {
        fprintf(fp, "contract violated.\n");
        fclose(fp);
    }
}

// Signal handler untuk menangkap saat daemon dihentikan (kill)
void handle_signal(int sig) {
    FILE *fp = fopen(log_filename, "a");
    if (fp) {
        fprintf(fp, "We really weren't meant to be together\n");
        fclose(fp);
    }
    exit(0);
}

// Thread khusus untuk memonitor file contract.txt
void *monitor_thread(void *arg) {
    while (1) {
        sleep(1); // Cek file setiap 1 detik

        FILE *fp = fopen(contract_filename, "r");
        if (fp == NULL) {
            // Skenario 4: File dihapus (tidak ditemukan)
            write_contract(1); 
        } else {
            // Membaca isi file saat ini
            char current_content[512] = {0};
            size_t len = fread(current_content, 1, sizeof(current_content) - 1, fp);
            current_content[len] = '\0';
            fclose(fp);

            // Skenario 5: Jika isi file berbeda dari yang seharusnya (diubah)
            if (strcmp(current_content, expected_content) != 0) {
                write_violation_log();
                write_contract(1); // Restore file
            }
        }
    }
    return NULL;
}

// Fungsi untuk mengubah program menjadi Daemon
void daemonize() {
    pid_t pid;

    // Fork pertama
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); // Parent keluar

    // Buat session baru
    if (setsid() < 0) exit(EXIT_FAILURE);

    // Abaikan sinyal yang dapat menghentikan proses dari terminal
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // Fork kedua
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // Atur perizinan file baru
    umask(0);

    // Tutup standard file descriptors agar tidak terikat ke terminal
    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--) {
        close(x);
    }

    // Arahkan stdin, stdout, stderr ke /dev/null
    int fd0 = open("/dev/null", O_RDWR);
    if (fd0 != -1) {
        dup2(fd0, STDIN_FILENO);
        dup2(fd0, STDOUT_FILENO);
        dup2(fd0, STDERR_FILENO);
        if (fd0 > 2) close(fd0);
    }
}

int main() {
    // 1. Ubah proses menjadi Daemon
    daemonize();

    // 2. Tangkap sinyal terminasi (untuk poin ke-6)
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);

    // Seed untuk random generator
    srand(time(NULL));

    // 3. Pembuatan awal contract.txt (Poin ke-3)
    write_contract(0);

    // 4. Jalankan thread monitor contract.txt
    pthread_t tid;
    pthread_create(&tid, NULL, monitor_thread, NULL);

    // 5. Loop utama untuk menulis log setiap 5 detik (Poin ke-2)
    const char *statuses[] = {"[awake]", "[drifting]", "[numbness]"};
    while (1) {
        FILE *fp = fopen(log_filename, "a");
        if (fp) {
            int status_idx = rand() % 3;
            fprintf(fp, "still working... %s\n", statuses[status_idx]);
            fclose(fp);
        }
        sleep(5);
    }

    return 0;
}