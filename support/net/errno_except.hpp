#pragma once

#include "../commondefs.hpp"
#include <errno.h>

class errno_error: public std::runtime_error {
public:
    errno_error(const std::string &descr = "socket error"): runtime_error(descr) {}
};

RT_ERROR_CLASS_MSG(errno_eperm, "EPERM");
RT_ERROR_CLASS_MSG(errno_enoent, "ENOENT");
RT_ERROR_CLASS_MSG(errno_esrch, "ESRCH");
RT_ERROR_CLASS_MSG(errno_enxio, "ENXIO");
RT_ERROR_CLASS_MSG(errno_e2big, "E2BIG");
RT_ERROR_CLASS_MSG(errno_enoexec, "ENOEXEC");
RT_ERROR_CLASS_MSG(errno_echild, "ECHILD");
RT_ERROR_CLASS_MSG(errno_enotblk, "ENOTBLK");
RT_ERROR_CLASS_MSG(errno_ebusy, "EBUSY");
RT_ERROR_CLASS_MSG(errno_eexist, "EEXIST");
RT_ERROR_CLASS_MSG(errno_exdev, "EXDEV");
RT_ERROR_CLASS_MSG(errno_enodev, "ENODEV");
RT_ERROR_CLASS_MSG(errno_enotdir, "ENOTDIR");
RT_ERROR_CLASS_MSG(errno_eisdir, "EISDIR");
RT_ERROR_CLASS_MSG(errno_enfile, "ENFILE");
RT_ERROR_CLASS_MSG(errno_emfile, "EMFILE");
RT_ERROR_CLASS_MSG(errno_enotty, "ENOTTY");
RT_ERROR_CLASS_MSG(errno_etxtbsy, "ETXTBSY");
RT_ERROR_CLASS_MSG(errno_efbig, "EFBIG");
RT_ERROR_CLASS_MSG(errno_enospc, "ENOSPC");
RT_ERROR_CLASS_MSG(errno_espipe, "ESPIPE");
RT_ERROR_CLASS_MSG(errno_erofs, "EROFS");
RT_ERROR_CLASS_MSG(errno_emlink, "EMLINK");
RT_ERROR_CLASS_MSG(errno_edom, "EDOM");
RT_ERROR_CLASS_MSG(errno_erange, "ERANGE");
RT_ERROR_CLASS_MSG(errno_eagain, "EAGAIN");
RT_ERROR_CLASS_MSG(errno_eacces, "EACCES");
RT_ERROR_CLASS_MSG(errno_ebadf, "EBADF");
RT_ERROR_CLASS_MSG(errno_econnreset, "ECONNRESET");
RT_ERROR_CLASS_MSG(errno_eintr, "EINTR");
RT_ERROR_CLASS_MSG(errno_edestaddrreq, "EDESTADDRREQ");
RT_ERROR_CLASS_MSG(errno_efault, "EFAULT");
RT_ERROR_CLASS_MSG(errno_einval, "EINVAL");
RT_ERROR_CLASS_MSG(errno_eisconn, "EISCONN");
RT_ERROR_CLASS_MSG(errno_emsgsize, "EMSGSIZE");
RT_ERROR_CLASS_MSG(errno_enobufs, "ENOBUFS");
RT_ERROR_CLASS_MSG(errno_enomem, "ENOMEM");
RT_ERROR_CLASS_MSG(errno_enotconn, "ENOTCONN");
RT_ERROR_CLASS_MSG(errno_enotsock, "ENOTSOCK");
RT_ERROR_CLASS_MSG(errno_enotsupp, "ENOTSUPP");
RT_ERROR_CLASS_MSG(errno_epipe, "EPIPE");
RT_ERROR_CLASS_MSG(errno_eio, "EIO");
RT_ERROR_CLASS_MSG(errno_ewouldblock, "EWOULDBLOCK");

void throw_errno(int e, const std::string &description = std::string());
