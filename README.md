# Final Project Sistem Operasi IT

## Kelompok C07

| Nama                | NRP        |
| ------------------- | ---------- |
| Ardhi Putra Pradana | 5027241022 |
| Salsa Bil Ulla      | 5027241052 |
| Muhammad Rafi` Adly | 5027241082 |

## Deskripsi Soal

**FUSE** - Logging system after open, write, and delete a file.

Buatlah sebuah program FUSE yang dapat mount sebuah directory. Saat sebuah file text di directory tersebut dibuka, diubah isinya, maupun dihapus filenya, maka setiap aksi tersebut akan dicatat di dalam sebuah log file bernama history.log. Pastikan juga terdapat tanggal dan waktu untuk tiap log.

### Catatan

Struktur folder:

```
.
..
├── .gitignore
├── Makefile
├── README.md
├── assets
├── dist (fuse mountpoint)
├── fuselogger.c
└── source (fuse target)
└── history.log
```

## Pengerjaan

> Inisialisasi

**Teori**

Dalam inisialasi ini adalah mempersiapkan hal - hal yang dibutuhkan untuk menjalankan dan juga membuat fuse. Untuk mempermudah dalam menjalankan fuse maka dibuat `Makefile` untuk meng-automasi hal - hal yang diperlukan dalam menjalankan fuse.

Lalu untuk membuat kode program fuse sendiri disini menggunakan diperlukan `libfuse` yang nanti akan digunakan sebagai library dari kode `c`.

**Solusi**

1. Makefile

```make
MAIN_FILE = fuselogger.c
MAIN_OUTPUT = fuselogger

FUSE_DIR = dist

build:
	@gcc -Wall $(MAIN_FILE) $(shell pkg-config fuse --cflags --libs) -o $(MAIN_OUTPUT)

unmount:
	@if mountpoint -q $(FUSE_DIR) 2>/dev/null; then \
		fusermount -u $(FUSE_DIR); \
	fi

run: build unmount
	@if [ ! -d "$(FUSE_DIR)" ]; then mkdir $(FUSE_DIR); fi
	@./$(MAIN_OUTPUT) $(FUSE_DIR)
```

Penjelasan:

1. `MAIN_FILE` dan `MAIN_OUTPUT` adalah inisialisasi variable sebagai input dan output executable fuse nya nanti.
2. sub command `build` digunakan untuk mendefinisikan perintah - perintah untuk meng-compile kode `c` yang dibuat. yaitu dengan menjalankan perintah `@gcc -Wall $(MAIN_FILE) $(shell pkg-config fuse --cflags --libs) -o $(MAIN_OUTPUT)`
3. sub command `unmount` digunakan untuk mendefinisikan perintah yang nantinya akan melakukan unmounting dari sebuah directory `fuse` jika directory tersebut memang sedang dalam keadaan mounting, untuk mengeceknya menggunakan if clause dan menggunakan perintah `mountpoint -q $(FUSE_DIR) 2>/dev/null` dan untuk melakukan unmounting menggunakan perintah `fusermount -u $(FUSE_DIR);`
4. Terakhir sub command `run` memiliki dependencies command ke `build` dan `unmount` dimana artinya sebelum menjalankan semua hal yang ada di command `run` akan menjalankan semua hal yang menjadi dependencies command tersebut.

5. Implementasi kode `c`

```c
#define _DEFAULT_SOURCE
#define FUSE_USE_VERSION 28

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

char *fullpath(const char *path, const char *base);
void logger(const char *action, const char *path, const char *details);

static int xmp_getattr(const char *path, struct stat *stbuf);
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
static int xmp_open(const char *path, struct fuse_file_info *fi);
static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi);
static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int xmp_truncate(const char *path, off_t size);
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
static int xmp_release(const char *path, struct fuse_file_info *fi);
static int xmp_unlink(const char *path);

static struct fuse_operations xmp_oper = {
    .getattr = xmp_getattr,
    .readdir = xmp_readdir,
    .open = xmp_open,
    .read = xmp_read,
    .create = xmp_create,
    .write = xmp_write,
    .truncate = xmp_truncate,
    .release = xmp_release,
    .unlink = xmp_unlink,
};

#define SOURCE_DIR "source"

static char cwd[PATH_MAX];

int main(int argc, char *argv[]) {
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    char *fpath = fullpath("/", SOURCE_DIR);
    if (fpath == NULL) {
        perror("fullpath");
        return EXIT_FAILURE;
    }

    if (access(fpath, F_OK) == -1) {
        if (mkdir(fpath, 0755) == -1) {
            perror("mkdir");
            free(fpath);
            return EXIT_FAILURE;
        }
    }
    free(fpath);

    umask(0);
    return fuse_main(argc, argv, &xmp_oper, NULL);
}
```

Penjelasan:

1. Pada bagian `define` paling atas adalah untuk mendefinisikan versi `fuse` yang digunakan dan melakukan defining flag terhadap `_DEFAULT_SOURCE`
2. Lalu bagian selanjutnya adalah melakukan `include` terhadap required library yang digunakan untuk menjalankan semua hal yang ada pada kode tersebut termasuk lib `fuse` nya.
3. Selanjutnya adalah membuat placeholder function yang nantinya akan didefinisikan dan diimplementasikan nantinya, mulai dari placeholder utility function seperti `fullpath` dan `logger`, serta placeholder function untuk setiap `fuse` action yang ada, dan kemudian membuat `static struct fuse_operations` untuk melakukan setting terhadap action - action fuse yang ada.
4. Melakukan `define` `SOURCE_DIR` yang digunakan untuk target relative path dari folder yang akan dimounting menjadi `fuse`.
5. `static char cwd[PATH_MAX];` definisi variabel ini akan menyimpan current working directory dimana letak binary atau programnya dijalankan.
6. Selanjutnya adalah function `main` sebagai gerbang untuk semua program yang dijalankan.
7. Bagian kode yang ada didalem function `main` dibawah ini digunakan untuk melakukan global variabel `cwd` dan mengecek apakah folder target ada atau tidak.

```c
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd");
        return EXIT_FAILURE;
    }

    char *fpath = fullpath("/", SOURCE_DIR);
    if (fpath == NULL) {
        perror("fullpath");
        return EXIT_FAILURE;
    }

    if (access(fpath, F_OK) == -1) {
        if (mkdir(fpath, 0755) == -1) {
            perror("mkdir");
            free(fpath);
            return EXIT_FAILURE;
        }
    }
    free(fpath);
```

8. `umask(0);` digunakan untuk file creation masking untuk melakukan set permission running process terhadap owning programnya.
9. `return fuse_main(argc, argv, &xmp_oper, NULL);` Terakhir ini digunakan untuk mulai menjalankan `fuse` functionnya.

> Pembuatan logger

**Teori**

Teknik logging adalah teknik untuk mencoba melakukan output dari sebuah program kedalam sebuah `stdout`, logging biasanya digunakan untuk debugging atau bahkan untuk memberikan informasi tambahan atau mencatat sebuah informasi. Dalam program kali ini sesuai dengan deskripsi soal maka `stdout` yang digunakan adalah `file pointer` atau outputnya akan dimasukkan ke dalam sebuah file.

**Solusi**

```c
void logger(const char *action, const char *path, const char *details) {
    char *fpath = fullpath("history.log", "");
    FILE *fp = fopen(fpath, "a");
    if (fp == NULL) return;

    char time_str[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    if (details && strlen(details) > 0) {
        fprintf(fp, "[%s] %s: %s - %s\n", time_str, action, path, details);
    } else {
        fprintf(fp, "[%s] %s: %s\n", time_str, action, path);
    }

    fclose(fp);
}
```

Penjelasan:

1. Function `logger` akan menerima 3 argument yaitu `action` untuk mendifinisikan action apa yang dilakukan (cont: EDIT, OPEN, dll), selanjutnya `path` digunakan untuk mencatat `path` apa yang sedang diakses karena dalam kasus ini logging yang dilakukan untuk melakukan tracking fuse operationnya, terakhir adalah `details` untuk memberikan informasi tambahan pada log tersebut.
2. Bagian dibawah ini digunakan untuk membuka sebuah file atau membuat file pointer ke file `history.log`

```c
char *fpath = fullpath("history.log", "");
FILE *fp = fopen(fpath, "a");
if (fp == NULL) return;
```

3. Bagian dibawah ini digunakan untuk membuat timestamp format string.

```c
char time_str[32];
time_t now = time(NULL);
struct tm *tm_info = localtime(&now);
strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
```

4. Selanjutnya bagian dibawah ini untuk mulai melakukan writing konten ke file yang dituju

```c
if (details && strlen(details) > 0) {
    fprintf(fp, "[%s] %s: %s - %s\n", time_str, action, path, details);
} else {
    fprintf(fp, "[%s] %s: %s\n", time_str, action, path);
}
```

5. `fclose(fp);` close file pointer.

> Implementasi membaca _attribute_ file/directory - `getattr`

**Teori**

Fungsi `getattr` pada FUSE digunakan untuk memberikan informasi file atau direktori seperti izin akses, ukuran file, waktu modifikasi, dan kepemilikan. Pada FUSE, `getattr` akan diteruskan dari kernel ke daemon pengguna, lalu mengambil metadata dari file backend dan mengembalikkannya ke kernel. Menurut Vangoor (2017), fungsi `getattr` masuk ke kategori operasi metadata dan dapat menambah latensi karena harus menunggu respons dari daemon user-space.

**Solusi**

Pada file `fuselogger.c`, fungsi `getattr` diimplementasikan sebagai berikut:

```c
static int xmp_getattr(const char *path, struct stat *stbuf) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;
    int res = lstat(fpath, stbuf);
    free(fpath);
    if (res == -1) return -errno;
    return 0;
}
```

Penjelasan:

- `fullpath(fpath, path)` digunakan untuk menyusun path absolut dari file.
- `lstat()` mengambil metadata file atau direktori.
- Jika gagal, kode akan mengembalikan `-errno` sebagai tanda error.
  Pendekatan ini efisien karena menggunakan sistem call standar `lstat()` langsung ke file sistem backend tanpa perubahan atau manipulasi metadata.

> Implementasi membaca _directory_ - `readdir`

**Teori**

`readdir` adalah fungsi yang membaca entri-entri dalam sebuah direktori. Dalam FUSE, proses ini diawali dengan kernel mengirim request ke daemon user-space, dan respons dikirim melalui buffer. Menurut Cho (2024), ini termasuk kategori operasi metadata dan dapat mengalami bottleneck karena overhead komunikasi user-kernel.

**Solusi**

Pada file `fuselogger.c`, fungsi `readdir` diimplementasikan sebagai berikut:

```c
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    DIR *dp = opendir(fpath);
    if (dp == NULL) {
        free(fpath);
        return -errno;
    }

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
        if (filler(buf, de->d_name, NULL, 0) != 0) {
            closedir(dp);
            free(fpath);
            return -ENOMEM;
        }
    }

    closedir(dp);
    free(fpath);

    logger("LIST", path, "mengakses directory");
    return 0;
}
```

Penjelasan:

- Membuka direktori menggunakan `opendir`.
- Membaca semua entri dengan `readdir`.
- Mengisi buffer kernel menggunakan `filler()`.
  Metode ini mengikuti alur tipikal FUSE dan tidak menyimpan cache atau modifikasi tambahan, juga mengimplementasikan loop `readdir` secara efisien dan aman.

> Implementasi membuka dan membaca file - `open`, `read`, `release`

**Teori**

Pada filesystem berbasis FUSE, konsep membuka file menggunakan `open()` system call dengan meneruskan permintaan ke file nyata. Setiap file yang sedang dibuka direpresentasikan oleh sebuah entri dalam tabel file terbuka pada sistem secara keseluruhan, yang menyimpan posisi saat ini dalam file serta cara akses yang digunakan (Sylberschatz, et al., 2011). Dalam FUSE, fungsi `open()` tidak membuka file seperti biasa, tapi hanya untuk menyiapkan descriptor dan validasi akses.

Setelah file berhasil dibuka, untuk membaca sebuah file dimulai dari offset tertentu tanpa mengubah posisi baca internal file yang kemudian memperbarui read pointer (Sylberschatz, et al., 2011). Terakhir, file ditutup dengan fungsi `close()`

**Solusi**

Untuk membuka file, pada `fuselogger.c`, fungsi `open` diimplementasikan sebagai berikut:

```c
static int xmp_open(const char *path, struct fuse_file_info *fi) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int fd = open(fpath, fi->flags);
    free(fpath);

    if (fd == -1) return -errno;
    fi->fh = fd;

    logger("OPEN", path, "membuka file");
    return 0;
}
```

Penjelasan:

- Membentuk path absolut dengan `fullpath()`
- Membuka file dengan sistem call `open()`
- Jika gagal membuka file, mengembalikan nilai ke `-errno`
- Mempersiapkan descriptor `fi->fh = fd` untuk kemudian dipanggil di fungsi `write()` atau `read()`
- Mencatat log aksi dengan `OPEN` menggunakan fungsi `logger()`

Untuk membaca file, diimplementasikan dengan fungsi `read` sebagai berikut:

```c
static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res = pread(fd, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}
```

Penjelasan:

- Membaca file menggunakan fungsi `pread()`
- Jika gagal, mengembalikan nilai ke `-errno`

Kemudian file ditutup dengan fungsi `release` yang diimplementasikan sebagai berikut:

```c
static int xmp_release(const char *path, struct fuse_file_info *fi) {
    int fd = fi->fh;
    if (fd > 0) close(fd);
    return 0;
}
```

Penjelasan:

> Implementasi membuat file - `create`, `write`

**Teori**

...

**Solusi**

```c
static int xmp_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int fd = creat(fpath, mode);
    free(fpath);

    if (fd == -1) return -errno;
    fi->fh = fd;

    logger("CREATE", path, "membuat file baru");
    return 0;
}
```

Penjelasan:

```c
static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
    int fd = fi->fh;
    int res = pwrite(fd, buf, size, offset);
    if (res == -1) return -errno;
    return res;
}
```

Penjelasan:

> Implementasi mengubah file - `truncate`

**Teori**

Fungsi `truncate` digunakan untuk mengubah ukuran file. Jika ukuran diperkecil, data setelah batas baru akan dihapus; jika diperbesar, sistem file akan menambahkan byte kosong. Dalam sistem file berbasis FUSE, `truncate` termasuk dalam operasi metadata yang memerlukan sistem call dari kernel ke user space.

Menurut jurnal FusionFS (Zhang, 2022), `truncate` adalah kandidat bagus untuk digabung dalam CISCOps (compound operations) untuk mengurangi overhead sistem call, karena ia mengubah struktur file secara langsung. Namun, dalam sistem FUSE konvensional, truncate tetap menimbulkan overhead karena membutuhkan beberapa lapis komunikasi user-kernel.

Jurnal Vangoor (2017) dan ACM Transactions on Storage (2019) juga mencatat bahwa `truncate` bisa menyebabkan latensi tambahan terutama saat sistem file perlu melakukan sinkronisasi metadata dan invalidasi cache.

**Solusi**

Pada file `fuselogger.c`, fungsi `truncate` diimplementasikan sebagai berikut:

```c
static int xmp_truncate(const char *path, off_t size) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int res = truncate(fpath, size);
    free(fpath);

    if (res == -1) return -errno;

    logger("EDIT", path, "mengubah file");

    return 0;
}
```

Penjelasan:

- Membentuk path absolut dengan `fullpath()`.
- Menjalankan `truncate()` langsung ke sistem file backend.
- Jika gagal, error dikembalikan dengan `-errno`.
- Melakukan pencatatan log aksi sebagai "EDIT" menggunakan fungsi `logger()`.
  Implementasi ini sederhana dan langsung, sesuai dengan pendekatan minimalis FUSE. Fungsi `truncate` bekerja efektif tanpa perlu membuka file descriptor atau caching khusus, meskipun overhead tetap ada dalam arsitektur FUSE tradisional.

> Implementasi menghapus file - `unlink`

**Teori**

Fungsi `unlink` bertugas untuk menghapus file. Dalam sistem file berbasis FUSE, operasi ini penting karena bisa melibatkan propagasi ke storage backend. Dalam jurnal FusionFS (Zhang, 2022), operasi penghapusan sederhana ini bisa dioptimalkan jika digabung dalam CISC operation, tapi dalam konteks FUSE biasa, overhead tetap ada.

**Solusi**

Pada file `fuselogger.c`, fungsi `unlink` diimplementasikan sebagai berikut:

```c
static int xmp_unlink(const char *path) {
    char *fpath = fullpath(path, SOURCE_DIR);
    if (fpath == NULL) return -ENOMEM;

    int res = unlink(fpath);
    free(fpath);
    if (res == -1) return -errno;

    logger("DELETE", path, "menghapus file");
    return 0;
}
```

Penjelasan:

- Membentuk path absolut.
- Menjalankan `unlink()` langsung ke sistem file backend.
- Jika gagal, error dikembalikan.
  Ini implementasi paling sederhana dan langsung, serta sudah cukup efisien karena tidak memerlukan pemrosesan tambahan.

## Video Menjalankan Program

[Akses video](./assets/demo.mp4)

https://github.com/user-attachments/assets/ee0e1e3f-8728-4cb7-81e2-b7122c9827d9

## Daftar Pustaka

Cho, K.-J., Choi, J., Kwon, H., & Kim, J.-S. (2024). RFUSE: Modernizing Userspace Filesystem Framework through Scalable Kernel-Userspace Communication. Proceedings of the 22nd USENIX Conference on File and Storage Technologies (FAST '24).

Silberschatz, A., Galvin, P. B., & Gagne, G. (2011). Operating system concepts essentials (8th ed.). John Wiley & Sons, Inc.

Vangoor, B. K. R., Agarwal, P., Mathew, M., Ramachandran, A., Sivaraman, S., Tarasov, V., & Zadok, E. (2019). Performance and Resource Utilization of FUSE User-Space File Systems. ACM Transactions on Storage (TOS), 15(2), Article 15.

Vangoor, B. K. R., Tarasov, V., & Zadok, E. (2017). To FUSE or Not to FUSE: Performance of User-Space File Systems. Proceedings of the 15th USENIX Conference on File and Storage Technologies (FAST '17).

Zhang, J., Ren, Y., & Kannan, S. (2022). FusionFS: Fusing I/O Operations using CISCOps in Firmware File Systems. Proceedings of the 20th USENIX Conference on File and Storage Technologies (FAST '22).
