#include "errno_except.hpp"

void throw_errno(int e, const std::string &description) {
    switch(e) {
        case EPERM: throw errno_eperm();
        case ENOENT: throw errno_enoent();
        case ESRCH: throw errno_esrch();
        case EINTR: throw errno_eintr();
        case EIO: throw errno_eio();
        case ENXIO: throw errno_enxio();
        case E2BIG: throw errno_e2big();
        case ENOEXEC: throw errno_enoexec();
        case EBADF: throw errno_ebadf();
        case ECHILD: throw errno_echild();
        case EAGAIN: throw errno_eagain();
        case ENOMEM: throw errno_enomem();
        case EACCES: throw errno_eacces();
        case EFAULT: throw errno_efault();
#ifndef _WIN32
        case ENOTBLK: throw errno_enotblk();
#endif
        case EBUSY: throw errno_ebusy();
        case EEXIST: throw errno_eexist();
        case EXDEV: throw errno_exdev();
        case ENODEV: throw errno_enodev();
        case ENOTDIR: throw errno_enotdir();
        case EISDIR: throw errno_eisdir();
        case EINVAL: throw errno_einval();
        case ENFILE: throw errno_enfile();
        case EMFILE: throw errno_emfile();
        case ENOTTY: throw errno_enotty();
        case ETXTBSY: throw errno_etxtbsy();
        case EFBIG: throw errno_efbig();
        case ENOSPC: throw errno_enospc();
        case ESPIPE: throw errno_espipe();
        case EROFS: throw errno_erofs();
        case EMLINK: throw errno_emlink();
        case EPIPE: throw errno_epipe();
        case EDOM: throw errno_edom();
        case ERANGE: throw errno_erange();
        default: throw errno_error(description);
    }
}
