#ifndef _GUESTSYNC_CTRL_H_
#define _GUESTSYNC_CTRL_H_

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#endif /* CONFIG_NATIVE_WINDOWS */


#define GUESTSYNC "guest_syncd"

/**
 * struct guestsync_ctrl - Internal structure for control interface library
 *
 * This structure is used by the guestsyncd control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */
typedef struct _guestsync_ctrl {
    int s;
#ifdef CONFIG_CTRL_IFACE_UDP
    struct sockaddr_in local;
    struct sockaddr_in dest;
#else /* CONFIG_CTRL_IFACE_UDP */
    struct sockaddr_un local;
    struct sockaddr_un dest;
#endif /* CONFIG_CTRL_IFACE_UDP */
} guestsync_ctrl;


/* guestsyncd control interface access */

/**
 * guestsync_ctrl_open - Open a control interface to guestsyncd
 * @ctrl_path: Path for UNIX domain sockets; ignored if UDP sockets are used.
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to guestsyncd.
 * ctrl_path is usually /var/run/guestsyncd. This path
 * is configured in guestsyncd and other programs using the control
 * interface need to use matching path configuration.
 */
guestsync_ctrl * guestsync_ctrl_open(const char *ctrl_path);


/**
 * guestsync_ctrl_close - Close a control interface to guestsyncd
 * @ctrl: Control interface data from guestsync_ctrl_open()
 *
 * This function is used to close a control interface.
 */
void guestsync_ctrl_close(guestsync_ctrl *ctrl);


/**
 * guestsync_ctrl_request - Send a command to guestsyncd
 * @ctrl: Control interface data from guestsync_ctrl_open()
 * @cmd: Command; usually, ASCII text, e.g., "PING"
 * @cmd_len: Length of the cmd in bytes
 * @reply: Buffer for the response
 * @reply_len: Reply buffer length
 * @msg_cb: Callback function for unsolicited messages or %NULL
 * if not used Returns: 0 on success, -1 on error (send or
 * receive failed), -2 on timeout
 *
 * This function is used to send commands to guestsyncd. Received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply. This function will block for up to two seconds while waiting
 * for the reply. If unsolicited messages are received, the blocking time may
 * be longer.
 *
 * msg_cb can be used to register a callback function that will be called for
 * unsolicited messages received while waiting for the command response. These
 * messages may be received if guestsync_ctrl_request() is called at the same time as
 * guestsyncd is sending such a message. This can happen only if
 * the program has used guestsync_ctrl_attach() to register itself as a monitor for
 * event messages. Alternatively to msg_cb, programs can register two control
 * interface connections and use one of them for commands and the other one for
 * receiving event messages, in other words, call guestsync_ctrl_attach() only for
 * the control interface connection that will be used for event messages.
 */
int guestsync_ctrl_request(guestsync_ctrl *ctrl, const char *cmd, size_t cmd_len,
        char *reply, size_t *reply_len,
        void (*msg_cb)(char *msg, size_t len));


/**
 * guestsync_ctrl_detach - Unregister event monitor from the control interface
 * @ctrl: Control interface data from guestsync_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function unregisters the control interface connection as a monitor for
 * guestsyncd events, i.e., cancels the registration done with
 * guestsync_ctrl_attach().
 */
int guestsync_ctrl_detach(guestsync_ctrl *ctrl);


/**
 * guestsync_ctrl_recv - Receive a pending control interface message
 * @ctrl: Control interface data from guestsync_ctrl_open()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. This
 * function will block if no messages are available. The received response will
 * be written to reply and reply_len is set to the actual length of the reply.
 * guestsync_ctrl_recv() is only used for event messages, i.e., guestsync_ctrl_attach()
 * must have been used to register the control interface as an event monitor.
 */
int guestsync_ctrl_recv(guestsync_ctrl *ctrl, char *reply, size_t *reply_len);


/**
 * guestsync_ctrl_pending - Check whether there are pending event messages
 * @ctrl: Control interface data from guestsync_ctrl_open()
 * Returns: Non-zero if there are pending messages
 *
 * This function will check whether there are any pending control interface
 * message available to be received with guestsync_ctrl_recv(). guestsync_ctrl_pending() is
 * only used for event messages, i.e., guestsync_ctrl_attach() must have been used to
 * register the control interface as an event monitor.
 */
int guestsync_ctrl_pending(guestsync_ctrl *ctrl);


/**
 * guestsync_ctrl_get_fd - Get file descriptor used by the control interface
 * @ctrl: Control interface data from guestsync_ctrl_open()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets; instead, the library functions guestsync_ctrl_request() and
 * guestsync_ctrl_recv() must be used for this.
 */
int guestsync_ctrl_get_fd(guestsync_ctrl *ctrl);

#ifdef CONFIG_CTRL_IFACE_UDP
#define guestsync_CTRL_IFACE_PORT 9877
#define WPA_GLOBAL_CTRL_IFACE_PORT 9878
#endif /* CONFIG_CTRL_IFACE_UDP */


#ifdef  __cplusplus
}
#endif

#endif /* _GUESTSYNC_CTRL_H_ */
