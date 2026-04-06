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