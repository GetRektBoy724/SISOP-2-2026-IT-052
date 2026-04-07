#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/prctl.h>
#include <dirent.h> 
#include <ctype.h>

// ==========================================
// BAGIAN 1: HELPER FUNCTIONS (Base64 & Log)
// ==========================================

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char *b64_encode(const unsigned char *in, size_t len) {
    char *out;
    size_t elen;
    size_t i, j, v;

    if (in == NULL || len == 0) return NULL;

    elen = 4 * ((len + 2) / 3);
    out = malloc(elen + 1);
    out[elen] = '\0';

    for (i=0, j=0; i<len; i+=3, j+=4) {
        v = in[i];
        v = i+1 < len ? v << 8 | in[i+1] : v << 8;
        v = i+2 < len ? v << 8 | in[i+2] : v << 8;

        out[j]   = b64chars[(v >> 18) & 0x3F];
        out[j+1] = b64chars[(v >> 12) & 0x3F];
        if (i+1 < len) out[j+2] = b64chars[(v >> 6) & 0x3F];
        else out[j+2] = '=';
        if (i+2 < len) out[j+3] = b64chars[v & 0x3F];
        else out[j+3] = '=';
    }
    return out;
}

unsigned char *b64_decode(const char *in, size_t *out_len) {
    size_t len = strlen(in);
    if (len % 4 != 0) return NULL;
    
    size_t padding = 0;
    if (len > 0 && in[len-1] == '=') padding++;
    if (len > 1 && in[len-2] == '=') padding++;
    
    *out_len = (len / 4) * 3 - padding;
    unsigned char *out = malloc(*out_len + 1);
    if (!out) return NULL;
    
    int b[256];
    for (int i=0; i<256; i++) b[i] = -1;
    for (int i=0; i<64; i++) b[(unsigned char)b64chars[i]] = i;
    
    size_t i, j;
    for (i=0, j=0; i<len; i+=4, j+=3) {
        int v = b[(unsigned char)in[i]] << 18 | 
                b[(unsigned char)in[i+1]] << 12 | 
                (in[i+2] == '=' ? 0 : b[(unsigned char)in[i+2]]) << 6 | 
                (in[i+3] == '=' ? 0 : b[(unsigned char)in[i+3]]);
                
        out[j] = (v >> 16) & 0xFF;
        if (in[i+2] != '=') out[j+1] = (v >> 8) & 0xFF;
        if (in[i+3] != '=') out[j+2] = v & 0xFF;
    }
    out[*out_len] = '\0';
    return out;
}

void write_log(const char *process, const char *status) {
    FILE *f = fopen("ethereal.log", "a");
    if (!f) return;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(f, "[%02d:%02d:%04d]-[%02d:%02d:%02d]_%s_%s\n",
            tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900,
            tm.tm_hour, tm.tm_min, tm.tm_sec,
            process, status);
    fclose(f);
}

void print_usage() {
    printf("Penggunaan:\n");
    printf("  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)\n");
    printf("  ./angel -decrypt : decrypt LoveLetter.txt\n");
    printf("  ./angel -kill    : kill proses\n");
}

// ==========================================
// BAGIAN 2: FUNGSI FITUR UTAMA
// ==========================================

// Fungsi untuk menjalankan program sebagai daemon (-daemon)
void run_daemon(char *cwd, int argc, char *argv[]) {    // --- Inisialisasi Daemon ---
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 
    
    if (setsid() < 0) exit(EXIT_FAILURE);
    
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS); 
    
    umask(0);
    chdir(cwd); // Tetap di direktori pengeksekusian awal
    
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    // Ubah nama proses yang tampil di ps aux menjadi "maya"
    prctl(PR_SET_NAME, "maya", 0, 0, 0);
    
    // Bersihkan seluruh argumen bawaan ('-daemon') dari memori
    for (int i = 0; i < argc; i++) {
        memset(argv[i], 0, strlen(argv[i]));
    }
    // Set ulang argv[0] menjadi maya
    strcpy(argv[0], "maya");
    
    const char *sentences[] = {
        "aku akan fokus pada diriku sendiri",
        "aku mencintaimu dari sekarang hingga selamanya",
        "aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.",
        "kalau aku dilahirkan kembali, aku tetap akan terus menyayangimu"
    };
    srand(time(NULL));

    // Loop Utama Daemon
    while (1) {
        // --- Fitur: secret ---
        write_log("secret", "RUNNING");
        FILE *f = fopen("LoveLetter.txt", "w");
        if (f) {
            int r = rand() % 4;
            fprintf(f, "%s", sentences[r]);
            fclose(f);
            write_log("secret", "SUCCESS");
        } else {
            write_log("secret", "ERROR");
        }

        // --- Fitur: surprise ---
        write_log("surprise", "RUNNING");
        f = fopen("LoveLetter.txt", "r");
        if (f) {
            fseek(f, 0, SEEK_END);
            long fsize = ftell(f);
            fseek(f, 0, SEEK_SET);
            
            unsigned char *content = malloc(fsize + 1);
            fread(content, 1, fsize, f);
            content[fsize] = 0;
            fclose(f);
            
            // Enkripsi Base64
            char *encoded = b64_encode(content, fsize);
            free(content);
            
            if (encoded) {
                f = fopen("LoveLetter.txt", "w");
                fprintf(f, "%s", encoded);
                fclose(f);
                free(encoded);
                write_log("surprise", "SUCCESS");
            } else {
                write_log("surprise", "ERROR");
            }
        } else {
            write_log("surprise", "ERROR");
        }
        
        sleep(10); // Ulangi setiap 10 detik
    }
}

// Fungsi untuk melakukan dekripsi manual (-decrypt)
void run_decrypt() {
    write_log("decrypt", "RUNNING");
    FILE *f = fopen("LoveLetter.txt", "r");
    if (!f) {
        printf("Error: File LoveLetter.txt tidak ada.\n");
        write_log("decrypt", "ERROR");
        return;
    }
    
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = malloc(fsize + 1);
    fread(content, 1, fsize, f);
    content[fsize] = 0;
    fclose(f);
    
    size_t out_len;
    unsigned char *decoded = b64_decode(content, &out_len);
    free(content);
    
    if (decoded) {
        f = fopen("LoveLetter.txt", "w");
        fwrite(decoded, 1, out_len, f);
        fclose(f);
        free(decoded);
        printf("Isi file LoveLetter.txt berhasil dikembalikan (decrypt).\n");
        write_log("decrypt", "SUCCESS");
    } else {
        printf("Error: Gagal mendekripsi file (format Base64 mungkin tidak valid).\n");
        write_log("decrypt", "ERROR");
    }
}

// Fungsi untuk menghentikan daemon (-kill) dengan meng-iterasi directory /proc untuk mencari proses dengan nama "maya" dan mengirim sinyal SIGTERM
void run_kill() {
    write_log("kill", "RUNNING");
    DIR *dir;
    struct dirent *ent;
    pid_t target_pid = -1;

    // Buka direktori /proc
    if ((dir = opendir("/proc")) != NULL) {
        // Iterasi semua entri di dalam /proc
        while ((ent = readdir(dir)) != NULL) {
            // Cek apakah nama direktori sepenuhnya angka (berarti itu direktori PID)
            int is_pid = 1;
            for (int i = 0; ent->d_name[i] != '\0'; i++) {
                if (!isdigit(ent->d_name[i])) {
                    is_pid = 0;
                    break;
                }
            }

            if (is_pid) {
                char path[256];
                // File 'comm' berisi nama command dari proses tersebut
                snprintf(path, sizeof(path), "/proc/%s/comm", ent->d_name);
                
                FILE *f = fopen(path, "r");
                if (f) {
                    char process_name[256];
                    if (fgets(process_name, sizeof(process_name), f) != NULL) {
                        // Hapus karakter newline (\n) di akhir string
                        process_name[strcspn(process_name, "\n")] = 0;
                        
                        // Cek apakah namanya adalah "maya"
                        if (strcmp(process_name, "maya") == 0) {
                            target_pid = atoi(ent->d_name);
                            fclose(f);
                            break; // Ketemu! Keluar dari loop readdir
                        }
                    }
                    fclose(f);
                }
            }
        }
        closedir(dir);
    } else {
        printf("Error: Tidak dapat membuka direktori /proc.\n");
        write_log("kill", "ERROR");
        return;
    }

    // Eksekusi Kill jika PID ditemukan
    if (target_pid != -1) {
        if (kill(target_pid, SIGTERM) == 0) {
            printf("Proses daemon 'maya' (PID: %d) berhasil dihentikan.\n", target_pid);
            write_log("kill", "SUCCESS");
        } else {
            printf("Error: Gagal menghentikan proses.\n");
            write_log("kill", "ERROR");
        }
    } else {
        printf("Error: Program (daemon) 'maya' tidak ditemukan sedang berjalan.\n");
        write_log("kill", "ERROR");
    }
}

// ==========================================
// BAGIAN 3: MAIN DRIVER
// ==========================================

int main(int argc, char *argv[]) {
    // Simpan working directory agar daemon tahu letak file yang harus dikelola
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd error");
        return 1;
    }

    if (argc != 2) {
        print_usage();
        return 1;
    }

    // Routing ke fungsi masing-masing berdasarkan argumen
    if (strcmp(argv[1], "-daemon") == 0) {
        run_daemon(cwd, argc, argv); // Kirim argc dan argv secara utuh
    }
    else if (strcmp(argv[1], "-decrypt") == 0) {
        run_decrypt();
    } 
    else if (strcmp(argv[1], "-kill") == 0) {
        run_kill();
    } 
    else {
        print_usage();
    }

    return 0;
}