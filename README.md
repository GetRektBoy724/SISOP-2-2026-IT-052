# SISOP-2-2026-IT-052
daemon? more like demon, and also finally we actually touch some syscalls bruh.

- Nama : Muhammad Syihan Zhafiri
- NRP : 5027251052

# Soal 1 - Kasbon Warga Kampung Durian Runtuh

Program ini bertujuan untuk mengamankan data hutang dari kedai abang muthuuu dengan membuat backup secara otomatis dan berurutan (*sequential*). Terdapat beberapa langkah yang harus dieksekusi oleh *child process* secara berurutan, yaitu: membuat direktori, menyalin file, memfilter data spesifik menggunakan `grep`, dan mengompres folder tersebut. Syarat utama dari program ini adalah harus menggunakan kombinasi `fork()`, fungsi `exec()`, dan `waitpid()`, serta dilarang keras menggunakan fungsi `system()`.

## Poin 1 & 6: Proses Sekuensial, Larangan system(), dan Pengecekan Status

Sesuai instruksi pertama, program tidak menggunakan `system()` melainkan keluarga `exec()` (dalam hal ini `execlp`). Agar eksekusi berjalan berurutan, *Parent Process* (Upin) selalu memanggil `waitpid()` setelah melakukan `fork()` untuk menunggu *Child Process* (Ipin) selesai bekerja sebelum lanjut ke langkah berikutnya.

Untuk memenuhi Poin 6 (pengecekan status keberhasilan), dibuat sebuah fungsi khusus:

```c
void check_status(int status) {
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        printf("[ERROR] Aiyaa! Proses gagal, file atau folder tidak ditemukan.\n");
        exit(EXIT_FAILURE);
    }
}
```

Fungsi ini mengecek *exit status* dari *child process*. Jika perintah gagal (misalnya `mkdir` gagal karena folder sudah ada, atau `cp` gagal karena file tidak ditemukan), program akan langsung mencetak pesan error dan berhenti menggunakan `exit()`.

## Poin 2: Membuat Folder brankas_kedai

Child process pertama dibuat menggunakan `fork()`. Di dalam child, dijalankan perintah `execlp("mkdir", "mkdir", "brankas_kedai", NULL);`. Parent menunggu dengan `waitpid()` lalu mengecek statusnya.

```c
...snip...
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
...snip...
```

## Poin 3: Menyalin File buku_hutang.csv

Setelah brankas siap, program kembali melakukan `fork()`. Child process kedua menjalankan perintah `execlp("cp", "cp", "buku_hutang.csv", "brankas_kedai/", NULL);` untuk menyalin data target ke dalam folder brankas.

```c
...snip...
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
...snip...
```

## Poin 4: Ekstraksi Data "Belum Lunas"

Ini adalah tahap yang membutuhkan trik khusus karena larangan menggunakan `system()`. Untuk melakukan *redirection output* (`>`) ke file `daftar_penunggak.txt`, kita tidak bisa memanggil `grep` secara langsung. Oleh karena itu, *child process* ketiga menggunakan *shell execution*:
```c
...snip...
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
...snip...
```
Perintah `bash -c` memungkinkan *shell* untuk membaca seluruh *string* tersebut sebagai satu perintah utuh, sehingga karakter `>` dapat berfungsi sebagaimana mestinya di *terminal*. Berikut adalah output dari `daftar_penunggak.txt`

```
Mail,Ayam Goreng 2 Paha,10.50,Belum Lunas
Fizi,Es ABC Spesial,3.50,Belum Lunas
Ahtong,Kopi O Tarik,2.00,Belum Lunas
Upin,Nasi Lemak Telur Mata,4.00,Belum Lunas
Ipin,Ayam Goreng 1 Dada,5.00,Belum Lunas
Mei Mei,Susu Kedelai,2.50,Belum Lunas
Susanti,Mie Goreng Mamak,5.50,Belum Lunas
Ijat,Es Limau,1.50,Belum Lunas
Ehsan,Nasi Lemak Ayam Goreng,9.50,Belum Lunas
```

## Poin 5: Mengompres Folder menjadi rahasia_muthu.zip

*Child process* keempat bertugas memaketkan folder `brankas_kedai` menjadi arsip `.zip`. Perintah yang dieksekusi adalah `execlp("zip", "zip", "-r", "rahasia_muthu.zip", "brankas_kedai", NULL);`. Flag `-r` (rekursif) digunakan agar seluruh isi folder ikut terkompres.

```c
...snip...
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
...snip...
```

## Poin 7: Pesan Keberhasilan

Jika seluruh langkah dari 1 hingga 4 berhasil melewati fungsi `check_status(status)`, maka baris terakhir pada fungsi `main()` akan dieksekusi untuk mencetak:
`[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.`

```c
...snip...
    printf("[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.\n");
...snip...
```

## Output Terminal

Kita bisa meng-*compile* kodenya menggunakan GCC dengan command `gcc kasir_muthu.c -o kasir_muthu`. Lalu, saat dijalankan, outputnya adalah
```
┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_1]
└─$ ./kasir_muthu
  adding: brankas_kedai/ (stored 0%)
  adding: brankas_kedai/buku_hutang.csv (deflated 48%)
  adding: brankas_kedai/daftar_penunggak.txt (deflated 49%)
[INFO] Fuhh, selamat! Buku hutang dan daftar penagihan berhasil diamankan.
```

# Soal 2 - The world never stops, even when you feel tired

Tugas ini mensimulasikan sebuah proses di balik layar (*daemon*) yang menggambarkan konsep "keberlanjutan". *Daemon* ini bertugas mencatat log aktivitas secara berkala, membuat file kontrak (*agreement*), memonitor integritas file kontrak tersebut dari modifikasi atau penghapusan, dan memberikan pesan khusus ketika proses *daemon* dihentikan secara paksa. Semua operasi harus dilakukan murni menggunakan bahasa pemrograman C tanpa intervensi file manual.

## Poin 1 & 2: Daemon Process dan Log Rutin
- Pembuatan Daemon: Program diubah menjadi *background process* menggunakan fungsi `daemonize()`. Fungsi ini melakukan *double fork* untuk melepaskan proses dari terminal yang mengontrolnya (controlling terminal) (alasan lebih lengkapnya bisa di baca [disini](https://stackoverflow.com/questions/881388/what-is-the-reason-for-performing-a-double-fork-when-creating-a-daemon)), memanggil `setsid()` untuk membuat *session* baru, dan menutup *standard I/O* (`stdin`, `stdout`, `stderr`) agar program berjalan murni di balik layar secara "sunyi".

```c
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
```

- Log `work.log`: Di dalam fungsi `main()`, terdapat sebuah *infinite loop* (`while(1)`) utama yang diatur menggunakan `sleep(5)` agar mengeksekusi perintah setiap 5 detik. Terdapat array string `statuses` yang berisi tiga status. Fungsi `rand() % 3` digunakan untuk memilih indeks secara acak, kemudian program mencetak pesan `"still working... [status]"` menggunakan mode *append* ("a") ke file `work.log`.

```c
...snip...
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
...snip...
```

## Poin 3: Inisialisasi File Kontrak

- Saat program baru berjalan (sebelum masuk ke loop utama), fungsi `write_contract(0)` dipanggil. Parameter 0 menandakan bahwa ini adalah pembuatan awal.

```c
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

```

- Fungsi ini memanggil `get_timestamp()` yang memanfaatkan `localtime()` dan `strftime()` untuk mendapatkan format tanggal dan waktu saat ini (contoh: 2026-04-04 12:13:55). Teks sesuai format soal beserta timestamp tersebut ditulis ke dalam `contract.txt`.

```c
void get_timestamp(char *buffer, size_t size) {
    time_t rawtime;
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", info);
}
```

- Teks yang ditulis ini juga disimpan ke variabel global `expected_content` sebagai acuan validasi nanti.

## Poin 4 & 5: Monitor Penghapusan File (Restore) dan Perubahan Isi Kontrak (Violation)

- Program menggunakan thread terpisah melalui *POSIX Threads* (`pthread_create`). Fungsi `monitor_thread` berjalan paralel dengan loop utama.

```c
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
```

```c
...snip...
    // 4. Jalankan thread monitor contract.txt
    pthread_t tid;
    pthread_create(&tid, NULL, monitor_thread, NULL);
...snip...
```

- Thread ini melakukan pengecekan setiap 1 detik (`sleep(1)`). Program mencoba membuka contract.txt dalam mode read ("r"). Jika *return value* pointer `FILE` adalah `NULL`, artinya file tersebut tidak ada atau terhapus. Maka, program akan memanggil `write_contract(1)`. Parameter 1 (`is_restored`) akan mengubah baris kedua kontrak menjadi format `"restored at: <timestamp>"`.

- Jika file `contract.txt` ditemukan, `monitor_thread` akan membaca keseluruhan isinya menggunakan `fread()` dan menyimpannya ke variabel sementara `current_content`. Fungsi `strcmp()` digunakan untuk membandingkan isi file saat ini dengan variabel `expected_content` (isi asli yang seharusnya). Jika hasilnya tidak sama (`!= 0`), maka file telah dimodifikasi. Program akan membuka `work.log` untuk mencetak peringatan `"contract violated."` menggunakan fungsi `write_violation_log`, lalu segera memanggil `write_contract(1)` untuk melakukan pemulihan (*restore*) file `contract.txt` ke isi yang sah beserta *timestamp* terbaru.

## Poin 6: Penanganan Terminasi Program

- Daemon didesain agar tidak berhenti dengan sendirinya. Untuk menghentikannya, sistem perlu mengirim sinyal terminasi (misalnya perintah `kill <PID>`). Untuk memenuhi poin 6, di dalam fungsi `main()`, digunakan fungsi `signal(SIGTERM, handle_signal)` dan `signal(SIGINT, handle_signal)`.

```c
...snip...
    // 2. Tangkap sinyal terminasi (untuk poin ke-6)
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
...snip...
```

- Ketika sinyal *kill* diterima, alih-alih langsung mati secara diam-diam, alur program dialihkan ke fungsi `handle_signal()`. Fungsi ini akan menuliskan pesan `"We really weren't meant to be together"` ke dalam file `work.log` sebelum akhirnya mengeksekusi `exit(0)` untuk benar-benar menghentikan proses.

```c
void handle_signal(int sig) {
    FILE *fp = fopen(log_filename, "a");
    if (fp) {
        fprintf(fp, "We really weren't meant to be together\n");
        fclose(fp);
    }
    exit(0);
}
```

## Output Terminal
```
┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ ./contract_daemon

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ ls
contract_daemon  contract_daemon.c  contract.txt  work.log

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ cat contract.txt
"A promise to keep going, even when unseen."
created at: 2026-04-06 09:41:49

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ cat work.log
still working... [awake]
still working... [numbness]
still working... [awake]

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ rm contract.txt

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ cat contract.txt
"A promise to keep going, even when unseen."
restored at: 2026-04-06 09:42:08

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ echo "VIOLATED" >> contract.txt

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ cat work.log
still working... [awake]
still working... [numbness]
still working... [awake]
still working... [awake]
still working... [awake]
still working... [numbness]
still working... [numbness]
still working... [numbness]
contract violated.

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ kill $(pgrep contract_daemon)

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_2]
└─$ cat work.log
still working... [awake]
still working... [numbness]
still working... [awake]
still working... [awake]
still working... [awake]
still working... [numbness]
still working... [numbness]
still working... [numbness]
contract violated.
still working... [awake]
still working... [drifting]
still working... [drifting]
We really weren't meant to be together
```

# Soal 3 - One letter for destiny

Program `angel.c` adalah sebuah program C yang memiliki fungsi utama sebagai *daemon* yang berjalan di latar belakang (*background process*). Program ini bertugas untuk menulis kalimat acak ke dalam file `LoveLetter.txt` secara berkala (setiap 10 detik) dan langsung mengenkripsinya menggunakan *Base64* agar tidak dapat dibaca orang lain. Selain itu, program dilengkapi dengan argumen *command-line* untuk mendekripsi file, menghentikan *daemon*, dan mencatat seluruh aktivitas sistem ke dalam file log `ethereal.log`.

## Poin 1 & 7: Pembuatan Daemon dan Manipulasi Nama Proses

Untuk menjadikan program berjalan sebagai *daemon* (dengan argumen `-daemon`), digunakan teknik standar double `fork()`.
- `fork()` pertama dipanggil untuk membuat *child process*, kemudian *parent process* dihentikan (exit).
- `setsid()` dipanggil untuk melepaskan proses dari terminal.
- `fork()` kedua memastikan proses tidak akan pernah mengambil alih terminal kembali.
- Agar file yang dibuat (`LoveLetter.txt` dan `ethereal.log`) tersimpan di direktori saat perintah dieksekusi, fungsi `chdir(cwd)` digunakan untuk menahan proses di *Current Working Directory*, alih-alih berpindah ke *root* direktori `/`.
- Sesuai permintaan soal untuk mengubah nama proses di `ps aux` menjadi `"maya"`, digunakan fungsi `prctl(PR_SET_NAME, "maya", 0, 0, 0)`, argumen `argv[0]` ditimpa dengan string `"maya"`, dan argumen lainnya dibersihkan.

```c
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
...snip...
```

## Poin 2: Fitur `secret` (Penulisan Kalimat Acak)

Fitur ini berjalan di dalam *infinite loop* `while(1)` dari proses *daemon*. Program mendeklarasikan *array* berisi empat kalimat hardcode. Dengan menggunakan `srand(time(NULL))` sebagai *seed* dan `rand() % 4`, program memilih satu kalimat secara acak, membuka file `LoveLetter.txt` dengan mode "w" (*write/overwrite*), dan menulis kalimat tersebut.

```c
...snip...
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
...snip...
```

## Poin 3 & 4: Fitur `surprise` (Enkripsi Otomatis Berkala)

Segera setelah fitur `secret` menulis pesan asli ke dalam file, fitur `surprise` mengambil alih. Program membaca ulang isi file tersebut ke dalam memori sementara, memanggil fungsi pembantu `b64_encode()` untuk mengubah *plain-text* menjadi format *Base64*, dan menimpa kembali file `LoveLetter.txt` dengan pesan yang telah dienkripsi. Seluruh proses secret dan surprise ini akan tertidur sementara dengan fungsi `sleep(10)` dan berulang terus menerus (Poin 4).

```c
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
```

## Poin 5: Fitur `decrypt` (Pengembalian Pesan Asli)

Fitur ini dipanggil terpisah melalui terminal dengan perintah `./angel -decrypt`. Program tidak berjalan sebagai *daemon*, melainkan proses biasa. Program membaca isi `LoveLetter.txt`, memasukkannya ke fungsi `b64_decode()` untuk meng-"dekripsi" string *Base64* nya, dan menulis kembali hasilnya ke file yang sama. Error handling ditambahkan dengan mengecek eksistensi *pointer* `FILE *f` (jika file tidak ada) dan mengecek validitas output dari *decoder*.

```c
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
```

## Poin 6: Fitur kill (Mematikan Daemon)

Program memanfaatkan direktori *native* sistem Linux (`/proc`) untuk melacak *daemon* yang sedang berjalan.
Ketika fitur `./angel -kill` dipanggil:
- Program membuka direktori `/proc` menggunakan `opendir()`.
- Ia mengiterasi seluruh folder di dalamnya dengan `readdir()`.
- Memfilter folder yang namanya berupa susunan angka penuh (menandakan bahwa folder tersebut adalah representasi *Process ID*).
- Masuk ke dalam setiap folder PID dan membaca file `comm` (contoh: `/proc/1234/comm`) yang menyimpan nama proses.
- Jika string yang terbaca identik dengan `"maya"`, PID tersebut disimpan, dan iterasi dihentikan.
- Perintah `kill(target_pid, SIGTERM)` akan dieksekusi ke PID tersebut, yang mana akan mematikan *daemon* secara instan.

```c
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
```

## Poin 8: Pencatatan Log Aktivitas

Untuk merekam riwayat semua aksi (baik oleh proses *daemon* yang berjalan sendiri, maupun aksi CLI manual oleh user), disediakan fungsi `write_log()`. Setiap kali ada proses seperti `secret`, `surprise`, `decrypt`, maupun `kill` dieksekusi, statusnya dicatat: saat memulai (`RUNNING`), saat selesai tanpa error (`SUCCESS`), maupun saat menemukan kesalahan (`ERROR`). Format timestamp didapatkan dengan mengekstrak data dari `time(NULL)` dan diformat sedemikian rupa menjadi `[dd:mm:yyyy]-[hh:mm:ss]_nama-proses_STATUS` untuk kemudian di append ke file `ethereal.log`.

```c
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
```

## Output Terminal

```
┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ./angel
Penggunaan:
  ./angel -daemon  : jalankan sebagai daemon (nama proses: maya)
  ./angel -decrypt : decrypt LoveLetter.txt
  ./angel -kill    : kill proses

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ./angel -daemon

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ps aux
USER         PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
root           1  0.0  0.1  24404 14332 ?        Ss   07:38   0:00 /sbin/init
root           2  0.0  0.0   3060  1920 ?        Sl   07:38   0:00 /init
root           7  0.0  0.0   3060  1792 ?        Sl   07:38   0:00 plan9 --control-socket 7 --log-level 4 --server-fd 8 --pipe-fd 10 --log-truncate
root          54  0.0  0.1  50076 15104 ?        Ss   07:38   0:00 /usr/lib/systemd/systemd-journald
root          68  0.1  0.0  35008 12032 ?        Ss   07:38   0:02 /usr/lib/systemd/systemd-udevd
root         154  0.0  0.0   4280  2176 ?        Ss   07:39   0:00 /usr/sbin/cron -f
message+     155  0.0  0.0   8188  3840 ?        Ss   07:39   0:00 /usr/bin/dbus-daemon --system --address=systemd: --nofork --nopidfile --systemd-activation --syslog-only
polkitd      191  0.0  0.0 306508  7168 ?        Ssl  07:39   0:00 /usr/lib/polkit-1/polkitd --no-debug --log-level=notice
root         206  0.0  0.0  17796  8320 ?        Ss   07:39   0:00 /usr/lib/systemd/systemd-logind
root         397  0.0  0.0   5168  2432 hvc0     Ss+  07:39   0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d --keep-b
root         398  0.0  0.0   5124  2432 tty1     Ss+  07:39   0:00 /usr/sbin/agetty --noreset --noclear --issue-file=/etc/issue:/etc/issue.d:/run/issue.d:/usr/lib/issue.d - linux
root         599  0.0  0.0   3064   896 ?        Ss   07:39   0:00 /init
root         600  0.0  0.0   3080  1024 ?        S    07:39   0:00 /init
hannn        601  0.0  0.0   8648  5248 pts/0    Ss   07:39   0:00 -bash
root         602  0.0  0.0   7900  4224 ?        Ss   07:39   0:00 login -- hannn
hannn        625  0.0  0.0  22556 12672 ?        Ss   07:39   0:00 /usr/lib/systemd/systemd --user
hannn        627  0.0  0.0  22548  3720 ?        S    07:39   0:00 (sd-pam)
hannn        656  0.0  0.0   7184  3328 ?        Ss   07:39   0:00 /usr/bin/mpris-proxy
hannn        657  0.0  0.0   5448  4736 pts/1    Ss+  07:39   0:00 -bash
hannn        659  0.0  0.0   8004  4224 ?        Ss   07:39   0:00 /usr/bin/dbus-daemon --session --address=systemd: --nofork --nopidfile --systemd-activation --syslog-only
hannn       1542  0.0  0.0   2564  1408 ?        S    08:12   0:00 maya
hannn       1552  0.0  0.0   9336  4096 pts/0    R+   08:12   0:00 ps aux

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ls
angel  angel.c  ethereal.log  LoveLetter.txt

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ./angel -decrypt
Isi file LoveLetter.txt berhasil dikembalikan (decrypt).

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ cat LoveLetter.txt
aku akan menjauh darimu, hingga takdir mempertemukan kita di versi kita yang terbaik.
┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ ./angel -kill
Proses daemon 'maya' (PID: 1542) berhasil dihentikan.

┌──(hannn㉿Hannn-Legion)-[/mnt/d/Desktop/SISOP-2-2026-IT-052/soal_3]
└─$ cat ethereal.log
[07:04:2026]-[08:12:43]_secret_RUNNING
[07:04:2026]-[08:12:43]_secret_SUCCESS
[07:04:2026]-[08:12:43]_surprise_RUNNING
[07:04:2026]-[08:12:43]_surprise_SUCCESS
[07:04:2026]-[08:12:53]_secret_RUNNING
[07:04:2026]-[08:12:53]_secret_SUCCESS
[07:04:2026]-[08:12:53]_surprise_RUNNING
[07:04:2026]-[08:12:53]_surprise_SUCCESS
[07:04:2026]-[08:12:55]_decrypt_RUNNING
[07:04:2026]-[08:12:55]_decrypt_SUCCESS
[07:04:2026]-[08:13:02]_kill_RUNNING
[07:04:2026]-[08:13:02]_kill_SUCCESS
```