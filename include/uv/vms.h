/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef UV_VMS_H
#define UV_VMS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include <time.h>
#define _XOPEN_SOURCE_EXTENDED /* for new 64-bit iovec */

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>  /* MAXHOSTNAMELEN on Solaris */

#undef _XOPEN_SOURCE_EXTENDED /* to get struct winsize */

#include <termios.h>
#include <pwd.h>

#include <semaphore.h>
#include <sys/param.h> /* MAXHOSTNAMELEN on Linux and the BSDs */
#include <pthread.h>
#include <signal.h>

#include "uv/threadpool.h"

#ifndef NI_MAXHOST
# define NI_MAXHOST 1025
#endif

#ifndef NI_MAXSERV
# define NI_MAXSERV 32
#endif

#ifndef UV_IO_PRIVATE_PLATFORM_FIELDS
# define UV_IO_PRIVATE_PLATFORM_FIELDS /* empty */
#endif

struct uv__io_s;
struct uv_loop_s;

typedef void (*uv__io_cb)(struct uv_loop_s* loop,
                          struct uv__io_s* w,
                          unsigned int events);
typedef struct uv__io_s uv__io_t;

struct uv__io_s {
  uv__io_cb cb;
  struct uv__queue pending_queue;
  struct uv__queue watcher_queue;
  unsigned int pevents; /* Pending event mask i.e. mask at next tick. */
  unsigned int events;  /* Current event mask. */
  int fd;
  UV_IO_PRIVATE_PLATFORM_FIELDS
};

#ifndef UV_PLATFORM_SEM_T
# define UV_PLATFORM_SEM_T sem_t
#endif

#ifndef UV_PLATFORM_FS_EVENT_FIELDS
# define UV_PLATFORM_FS_EVENT_FIELDS /* empty */
#endif

#ifndef UV_STREAM_PRIVATE_PLATFORM_FIELDS
# define UV_STREAM_PRIVATE_PLATFORM_FIELDS /* empty */
#endif

/* Note: May be cast to struct iovec. See writev(2). */
typedef struct uv_buf_t {
  char* base;
  size_t len;
} uv_buf_t;

typedef int uv_file;
typedef int uv_os_sock_t;
typedef int uv_os_fd_t;
typedef pid_t uv_pid_t;

#define UV_ONCE_INIT PTHREAD_ONCE_INIT

typedef pthread_once_t uv_once_t;
typedef pthread_t uv_thread_t;
typedef pthread_mutex_t uv_mutex_t;
typedef pthread_rwlock_t uv_rwlock_t;
typedef UV_PLATFORM_SEM_T uv_sem_t;
typedef pthread_cond_t uv_cond_t;
typedef pthread_key_t uv_key_t;

/* Note: guard clauses should match uv_barrier_init's in src/unix/thread.c. */
/* TODO(bnoordhuis) Merge into uv_barrier_t in v2. */
struct _uv_barrier {
  uv_mutex_t mutex;
  uv_cond_t cond;
  unsigned threshold;
  unsigned in;
  unsigned out;
};

typedef struct {
  struct _uv_barrier* b;
} uv_barrier_t;

/* Platform-specific definitions for uv_spawn support. */
typedef gid_t uv_gid_t;
typedef uid_t uv_uid_t;

typedef struct dirent uv__dirent_t;

#define UV_DIR_PRIVATE_FIELDS \
  DIR* dir;

#define UV__DT_FILE -1
#define UV__DT_DIR -2
#define UV__DT_LINK -3
#define UV__DT_FIFO -4
#define UV__DT_SOCKET -5
#define UV__DT_CHAR -6
#define UV__DT_BLOCK -7

/* Platform-specific definitions for uv_dlopen support. */
#define UV_DYNAMIC /* empty */

typedef struct {
  void* handle;
  char* errmsg;
} uv_lib_t;

#define UV_LOOP_PRIVATE_FIELDS                                                \
  unsigned long flags;                                                        \
  int backend_fd;                                                             \
  struct uv__queue pending_queue;                                             \
  struct uv__queue watcher_queue;                                             \
  uv__io_t** watchers;                                                        \
  unsigned int nwatchers;                                                     \
  unsigned int nfds;                                                          \
  struct uv__queue wq;                                                        \
  uv_mutex_t wq_mutex;                                                        \
  uv_async_t wq_async;                                                        \
  uv_rwlock_t cloexec_lock;                                                   \
  uv_handle_t* closing_handles;                                               \
  struct uv__queue process_handles;                                           \
  struct uv__queue prepare_handles;                                           \
  struct uv__queue check_handles;                                             \
  struct uv__queue idle_handles;                                              \
  struct uv__queue async_handles;                                             \
  struct {                                                                    \
    void* min;                                                                \
    unsigned int nelts;                                                       \
  } timer_heap;                                                               \
  uint64_t timer_counter;                                                     \
  uint64_t time;                                                              \
  uv__io_t signal_io_watcher;                                                 \
  uv_signal_t child_watcher;                                                  \
  uv_thread_t poll_thread;                                                    \
  int poll_fds_iterating;                                                     \
  int poll_thread_paused;                                                     \
  struct pollfd* poll_fds;                                                    \
  size_t poll_fds_used;                                                       \
  size_t poll_fds_size;                                                       \
  unsigned int async_ef;                                                      \
  unsigned int timer_ef;                                                      \
  int signal_pipefd[2];                                                       \
  int poll_thread_sockfd[2];                                                  \
  uv_async_t* ast_async_tail;                                                 \
  int async_pending_flags;                                                    \

/* Async pending type flags */
#define UV__LOOP_PENDING_AST            1
#define UV__LOOP_PENDING_ASYNC          2
#define UV__LOOP_PENDING_POLL           4

#define UV_REQ_TYPE_PRIVATE /* empty */

#define UV_REQ_PRIVATE_FIELDS  /* empty */

#define UV_PRIVATE_REQ_TYPES /* empty */

#define UV_WRITE_PRIVATE_FIELDS                                               \
  struct uv__queue queue;                                                     \
  unsigned int write_index;                                                   \
  uv_buf_t* bufs;                                                             \
  unsigned int nbufs;                                                         \
  int error;                                                                  \
  uv_buf_t bufsml[4];                                                         \

#define UV_CONNECT_PRIVATE_FIELDS                                             \
  struct uv__queue queue;                                                     \

#define UV_SHUTDOWN_PRIVATE_FIELDS /* empty */

#define UV_UDP_SEND_PRIVATE_FIELDS                                            \
  struct uv__queue queue;                                                     \
  struct sockaddr_storage addr;                                               \
  unsigned int nbufs;                                                         \
  uv_buf_t* bufs;                                                             \
  ssize_t status;                                                             \
  uv_udp_send_cb send_cb;                                                     \
  uv_buf_t bufsml[4];                                                         \

#define UV_HANDLE_PRIVATE_FIELDS                                              \
  uv_handle_t* next_closing;                                                  \
  unsigned int flags;                                                         \

#define UV_STREAM_PRIVATE_FIELDS                                              \
  uv_connect_t *connect_req;                                                  \
  uv_shutdown_t *shutdown_req;                                                \
  uv__io_t io_watcher;                                                        \
  struct uv__queue write_queue;                                               \
  struct uv__queue write_completed_queue;                                     \
  uv_connection_cb connection_cb;                                             \
  int delayed_error;                                                          \
  int accepted_fd;                                                            \
  void* queued_fds;                                                           \
  UV_STREAM_PRIVATE_PLATFORM_FIELDS                                           \

#define UV_TCP_PRIVATE_FIELDS /* empty */

#define UV_UDP_PRIVATE_FIELDS                                                 \
  uv_alloc_cb alloc_cb;                                                       \
  uv_udp_recv_cb recv_cb;                                                     \
  uv__io_t io_watcher;                                                        \
  struct uv__queue write_queue;                                               \
  struct uv__queue write_completed_queue;                                     \

#define UV_PIPE_PRIVATE_FIELDS                                                \
  const char* pipe_fname; /* NULL or strdup'ed */

#define UV_POLL_PRIVATE_FIELDS                                                \
  uv__io_t io_watcher;

#define UV_PREPARE_PRIVATE_FIELDS                                             \
  uv_prepare_cb prepare_cb;                                                   \
  struct uv__queue queue;                                                     \

#define UV_CHECK_PRIVATE_FIELDS                                               \
  uv_check_cb check_cb;                                                       \
  struct uv__queue queue;                                                     \

#define UV_IDLE_PRIVATE_FIELDS                                                \
  uv_idle_cb idle_cb;                                                         \
  struct uv__queue queue;                                                     \

#define UV_ASYNC_PRIVATE_FIELDS                                               \
  uv_async_cb async_cb;                                                       \
  struct uv__queue queue;                                                     \
  int pending;                                                                \

#define UV_TIMER_PRIVATE_FIELDS                                               \
  uv_timer_cb timer_cb;                                                       \
  void* heap_node[3];                                                         \
  uint64_t timeout;                                                           \
  uint64_t repeat;                                                            \
  uint64_t start_id;

#define UV_GETADDRINFO_PRIVATE_FIELDS                                         \
  struct uv__work work_req;                                                   \
  uv_getaddrinfo_cb cb;                                                       \
  struct addrinfo* hints;                                                     \
  char* hostname;                                                             \
  char* service;                                                              \
  struct addrinfo* addrinfo;                                                  \
  int retcode;

#define UV_GETNAMEINFO_PRIVATE_FIELDS                                         \
  struct uv__work work_req;                                                   \
  uv_getnameinfo_cb getnameinfo_cb;                                           \
  struct sockaddr_storage storage;                                            \
  int flags;                                                                  \
  char host[NI_MAXHOST];                                                      \
  char service[NI_MAXSERV];                                                   \
  int retcode;

#define UV_PROCESS_PRIVATE_FIELDS                                             \
  struct uv__queue queue;                                                     \
  int status;                                                                 \

#define UV_FS_PRIVATE_FIELDS                                                  \
  const char *new_path;                                                       \
  uv_file file;                                                               \
  int flags;                                                                  \
  mode_t mode;                                                                \
  unsigned int nbufs;                                                         \
  uv_buf_t* bufs;                                                             \
  off_t off;                                                                  \
  uv_uid_t uid;                                                               \
  uv_gid_t gid;                                                               \
  double atime;                                                               \
  double mtime;                                                               \
  struct uv__work work_req;                                                   \
  uv_buf_t bufsml[4];                                                         \

#define UV_WORK_PRIVATE_FIELDS                                                \
  struct uv__work work_req;

#define UV_TTY_PRIVATE_FIELDS                                                 \
  struct termios orig_termios;                                                \
  int mode;

#define UV_SIGNAL_PRIVATE_FIELDS                                              \
  /* RB_ENTRY(uv_signal_s) tree_entry; */                                     \
  struct {                                                                    \
    struct uv_signal_s* rbe_left;                                             \
    struct uv_signal_s* rbe_right;                                            \
    struct uv_signal_s* rbe_parent;                                           \
    int rbe_color;                                                            \
  } tree_entry;                                                               \
  /* Use two counters here so we don have to fiddle with atomics. */          \
  unsigned int caught_signals;                                                \
  unsigned int dispatched_signals;

#define UV_FS_EVENT_PRIVATE_FIELDS                                            \
  uv_fs_event_cb cb;                                                          \
  UV_PLATFORM_FS_EVENT_FIELDS                                                 \

/* fs open() flags supported on this platform: */
#if defined(O_APPEND)
# define UV_FS_O_APPEND       O_APPEND
#else
# define UV_FS_O_APPEND       0
#endif
#if defined(O_CREAT)
# define UV_FS_O_CREAT        O_CREAT
#else
# define UV_FS_O_CREAT        0
#endif

#if defined(O_DIRECT)
# define UV_FS_O_DIRECT       O_DIRECT
#else
# define UV_FS_O_DIRECT       0
#endif

#if defined(O_DIRECTORY)
# define UV_FS_O_DIRECTORY    O_DIRECTORY
#else
# define UV_FS_O_DIRECTORY    0
#endif
#if defined(O_DSYNC)
# define UV_FS_O_DSYNC        O_DSYNC
#else
# define UV_FS_O_DSYNC        0
#endif
#if defined(O_EXCL)
# define UV_FS_O_EXCL         O_EXCL
#else
# define UV_FS_O_EXCL         0
#endif
#if defined(O_EXLOCK)
# define UV_FS_O_EXLOCK       O_EXLOCK
#else
# define UV_FS_O_EXLOCK       0
#endif
#if defined(O_NOATIME)
# define UV_FS_O_NOATIME      O_NOATIME
#else
# define UV_FS_O_NOATIME      0
#endif
#if defined(O_NOCTTY)
# define UV_FS_O_NOCTTY       O_NOCTTY
#else
# define UV_FS_O_NOCTTY       0
#endif
#if defined(O_NOFOLLOW)
# define UV_FS_O_NOFOLLOW     O_NOFOLLOW
#else
# define UV_FS_O_NOFOLLOW     0
#endif
#if defined(O_NONBLOCK)
# define UV_FS_O_NONBLOCK     O_NONBLOCK
#else
# define UV_FS_O_NONBLOCK     0
#endif
#if defined(O_RDONLY)
# define UV_FS_O_RDONLY       O_RDONLY
#else
# define UV_FS_O_RDONLY       0
#endif
#if defined(O_RDWR)
# define UV_FS_O_RDWR         O_RDWR
#else
# define UV_FS_O_RDWR         0
#endif
#if defined(O_SYMLINK)
# define UV_FS_O_SYMLINK      O_SYMLINK
#else
# define UV_FS_O_SYMLINK      0
#endif
#if defined(O_SYNC)
# define UV_FS_O_SYNC         O_SYNC
#else
# define UV_FS_O_SYNC         0
#endif
#if defined(O_TRUNC)
# define UV_FS_O_TRUNC        O_TRUNC
#else
# define UV_FS_O_TRUNC        0
#endif
#if defined(O_WRONLY)
# define UV_FS_O_WRONLY       O_WRONLY
#else
# define UV_FS_O_WRONLY       0
#endif

/* fs open() flags supported on other platforms: */
#define UV_FS_O_FILEMAP       0
#define UV_FS_O_RANDOM        0
#define UV_FS_O_SHORT_LIVED   0
#define UV_FS_O_SEQUENTIAL    0
#define UV_FS_O_TEMPORARY     0

#endif /* UV_VMS_H */
