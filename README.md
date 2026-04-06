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