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

...

**Solusi**

...

> Pembuatan logger

**Teori**

...

**Solusi**

...

> Implementasi membaca _attribute_ file/directory - `getattr`

**Teori**

Fungsi `getattr` pada FUSE digunakan untuk memberikan informasi file atau direktori seperti izin akses, ukuran file, waktu modifikasi, dan kepemilikan. Pada FUSE, `getattr` akan diteruskan dari kernel ke daemon pengguna, lalu mengambil metadata dari file backend dan mengembalikkannya ke kernel. Menurut Vangoor (2017), fungsi `getattr` masuk ke kategori operasi metadata dan dapat menambah latensi karena harus menunggu respons dari daemon user-space.

**Solusi**

Pada file `fuselogger.c`, fungsi `getattr` diimplementasikan sebagai berikut:
```
static int xmp_getattr(const char *path, struct stat *stbuf) {
    int res;
    char fpath[PATH_MAX];
    fullpath(fpath, path);
    res = lstat(fpath, stbuf);
    if (res == -1)
        return -errno;
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
```
static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi,
                       enum fuse_readdir_flags flags) {
    DIR *dp;
    struct dirent *de;
    char fpath[PATH_MAX];
    fullpath(fpath, path);

    dp = opendir(fpath);
    if (dp == NULL)
        return -errno;

    while ((de = readdir(dp)) != NULL) {
        struct stat st = {0};
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buf, de->d_name, &st, 0, 0))
            break;
    }

    closedir(dp);
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

...

**Solusi**

...

> Implementasi membuat file - `create`, `write`

**Teori**

...

**Solusi**

...

> Implementasi mengubah file - `truncate`

**Teori**

...

**Solusi**

...

> Implementasi menghapus file - `unlink`

**Teori**

Fungsi `unlink` bertugas untuk menghapus file. Dalam sistem file berbasis FUSE, operasi ini penting karena bisa melibatkan propagasi ke storage backend. Dalam jurnal FusionFS (Zhang, 2022), operasi penghapusan sederhana ini bisa dioptimalkan jika digabung dalam CISC operation, tapi dalam konteks FUSE biasa, overhead tetap ada.

**Solusi**

Pada file `fuselogger.c`, fungsi `unlink` diimplementasikan sebagai berikut:
```
static int xmp_unlink(const char *path) {
    char fpath[PATH_MAX];
    fullpath(fpath, path);
    int res = unlink(fpath);
    if (res == -1)
        return -errno;
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

Vangoor, B. K. R., Tarasov, V., & Zadok, E. (2017). To FUSE or Not to FUSE: Performance of User-Space File Systems. Proceedings of the 15th USENIX Conference on File and Storage Technologies (FAST '17).
Cho, K.-J., Choi, J., Kwon, H., & Kim, J.-S. (2024). RFUSE: Modernizing Userspace Filesystem Framework through Scalable Kernel-Userspace Communication. Proceedings of the 22nd USENIX Conference on File and Storage Technologies (FAST '24).
Zhang, J., Ren, Y., & Kannan, S. (2022). FusionFS: Fusing I/O Operations using CISCOps in Firmware File Systems. Proceedings of the 20th USENIX Conference on File and Storage Technologies (FAST '22).
