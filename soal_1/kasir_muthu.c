#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Fungsi bantuan untuk mengecek status keberhasilan setiap child process
void check_status(int status) {
    // Jika proses tidak exit dengan normal ATAU exit statusnya bukan 0 (error)
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(EXIT_FAILURE);
    }
}

int main() {
    pid_t pid;
    int status;

    // ---------------------------------------------------------
    // Langkah 1: Membuat folder 'brankas_kedai'
    // ---------------------------------------------------------
    pid = fork();
    if (pid == 0) {
        // Child Process 1
        execlp("mkdir", "mkdir", "brankas_kedai", NULL);
        exit(EXIT_FAILURE); // Hanya tereksekusi jika execlp gagal
    } else if (pid > 0) {
        // Parent Process (Upin) menunggu Ipin
        waitpid(pid, &status, 0);
        check_status(status);
    }

    // ---------------------------------------------------------
    // Langkah 2: Menyalin file 'buku_hutang.csv' ke 'brankas_kedai'
    // ---------------------------------------------------------
    pid = fork();
    if (pid == 0) {
        // Child Process 2
        execlp("cp", "cp", "buku_hutang.csv", "brankas_kedai/", NULL);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process (Upin) menunggu Ipin
        waitpid(pid, &status, 0);
        check_status(status);
    }

    // ---------------------------------------------------------
    // Langkah 3: Mencari "Belum Lunas" dan menyimpannya ke file txt
    // ---------------------------------------------------------
    pid = fork();
    if (pid == 0) {
        // Child Process 3
        // Menggunakan trik bash -c untuk mengeksekusi redirection (>)
        execlp("bash", "bash", "-c", "grep \"Belum Lunas\" brankas_kedai/buku_hutang.csv > brankas_kedai/daftar_penunggak.txt", NULL);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process (Upin) menunggu Ipin
        waitpid(pid, &status, 0);
        check_status(status);
    }

    // ---------------------------------------------------------
    // Langkah 4: Mengompres folder menjadi 'rahasia_muthu.zip'
    // ---------------------------------------------------------
    pid = fork();
    if (pid == 0) {
        // Child Process 4
        execlp("zip", "zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent Process (Upin) menunggu Ipin
        waitpid(pid, &status, 0);
        check_status(status);
    }

    // ---------------------------------------------------------
    // Langkah 5: Cetak pesan sukses jika semua berhasil
    // ---------------------------------------------------------
    printf("[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.\n");

    return 0;
}
