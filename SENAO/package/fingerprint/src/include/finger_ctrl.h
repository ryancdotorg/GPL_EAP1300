#ifndef _FINGER_CTRL_H_
#define _FINGER_CTRL_H_

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef CONFIG_NATIVE_WINDOWS
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#endif /* CONFIG_NATIVE_WINDOWS */


#define FINGERSYNC "finger_syncd"

/**
 * struct fingersync_ctrl - Internal structure for control interface library
 *
 * This structure is used by the fingersyncd control interface
 * library to store internal data. Programs using the library should not touch
 * this data directly. They can only use the pointer to the data structure as
 * an identifier for the control interface connection and use this as one of
 * the arguments for most of the control interface library functions.
 */
typedef struct _fingersync_ctrl {
    int s;
#ifdef CONFIG_CTRL_IFACE_UDP
    struct sockaddr_in local;
    struct sockaddr_in dest;
#else /* CONFIG_CTRL_IFACE_UDP */
    struct sockaddr_un local;
    struct sockaddr_un dest;
#endif /* CONFIG_CTRL_IFACE_UDP */
} fingersync_ctrl;


/* fingersyncd control interface access */

/**
 * fingersync_ctrl_open - Open a control interface to fingersyncd
 * @ctrl_path: Path for UNIX domain sockets; ignored if UDP sockets are used.
 * Returns: Pointer to abstract control interface data or %NULL on failure
 *
 * This function is used to open a control interface to fingersyncd.
 * ctrl_path is usually /var/run/fingersyncd. This path
 * is configured in fingersyncd and other programs using the control
 * interface need to use matching path configuration.
 */
fingersync_ctrl * fingersync_ctrl_open(const char *ctrl_path);


/**
 * fingersync_ctrl_close - Close a control interface to fingersyncd
 * @ctrl: Control interface data from fingersync_ctrl_open()
 *
 * This function is used to close a control interface.
 */
void fingersync_ctrl_close(fingersync_ctrl *ctrl);


/**
 * fingersync_ctrl_request - Send a command to fingersyncd
 * @ctrl: Control interface data from fingersync_ctrl_open()
 * @cmd: Command; usually, ASCII text, e.g., "PING"
 * @cmd_len: Length of the cmd in bytes
 * @reply: Buffer for the response
 * @reply_len: Reply buffer length
 * @msg_cb: Callback function for unsolicited messages or %NULL
 * if not used Returns: 0 on success, -1 on error (send or
 * receive failed), -2 on timeout
 *
 * This function is used to send commands to fingersyncd. Received
 * response will be written to reply and reply_len is set to the actual length
 * of the reply. This function will block for up to two seconds while waiting
 * for the reply. If unsolicited messages are received, the blocking time may
 * be longer.
 *
 * msg_cb can be used to register a callback function that will be called for
 * unsolicited messages received while waiting for the command response. These
 * messages may be received if fingersync_ctrl_request() is called at the same time as
 * fingersyncd is sending such a message. This can happen only if
 * the program has used fingersync_ctrl_attach() to register itself as a monitor for
 * event messages. Alternatively to msg_cb, programs can register two control
 * interface connections and use one of them for commands and the other one for
 * receiving event messages, in other words, call fingersync_ctrl_attach() only for
 * the control interface connection that will be used for event messages.
 */
int fingersync_ctrl_request(fingersync_ctrl *ctrl, const char *cmd, size_t cmd_len,
        char *reply, size_t *reply_len,
        void (*msg_cb)(char *msg, size_t len));


/**
 * fingersync_ctrl_detach - Unregister event monitor from the control interface
 * @ctrl: Control interface data from fingersync_ctrl_open()
 * Returns: 0 on success, -1 on failure, -2 on timeout
 *
 * This function unregisters the control interface connection as a monitor for
 * fingersyncd events, i.e., cancels the registration done with
 * fingersync_ctrl_attach().
 */
int fingersync_ctrl_detach(fingersync_ctrl *ctrl);


/**
 * fingersync_ctrl_recv - Receive a pending control interface message
 * @ctrl: Control interface data from fingersync_ctrl_open()
 * @reply: Buffer for the message data
 * @reply_len: Length of the reply buffer
 * Returns: 0 on success, -1 on failure
 *
 * This function will receive a pending control interface message. This
 * function will block if no messages are available. The received response will
 * be written to reply and reply_len is set to the actual length of the reply.
 * fingersync_ctrl_recv() is only used for event messages, i.e., fingersync_ctrl_attach()
 * must have been used to register the control interface as an event monitor.
 */
int fingersync_ctrl_recv(fingersync_ctrl *ctrl, char *reply, size_t *reply_len);


/**
 * fingersync_ctrl_pending - Check whether there are pending event messages
 * @ctrl: Control interface data from fingersync_ctrl_open()
 * Returns: Non-zero if there are pending messages
 *
 * This function will check whether there are any pending control interface
 * message available to be received with fingersync_ctrl_recv(). fingersync_ctrl_pending() is
 * only used for event messages, i.e., fingersync_ctrl_attach() must have been used to
 * register the control interface as an event monitor.
 */
int fingersync_ctrl_pending(fingersync_ctrl *ctrl);


/**
 * fingersync_ctrl_get_fd - Get file descriptor used by the control interface
 * @ctrl: Control interface data from fingersync_ctrl_open()
 * Returns: File descriptor used for the connection
 *
 * This function can be used to get the file descriptor that is used for the
 * control interface connection. The returned value can be used, e.g., with
 * select() while waiting for multiple events.
 *
 * The returned file descriptor must not be used directly for sending or
 * receiving packets; instead, the library functions fingersync_ctrl_request() and
 * fingersync_ctrl_recv() must be used for this.
 */
int fingersync_ctrl_get_fd(fingersync_ctrl *ctrl);

#ifdef CONFIG_CTRL_IFACE_UDP
#define fingersync_CTRL_IFACE_PORT 9877
#define WPA_GLOBAL_CTRL_IFACE_PORT 9878
#endif /* CONFIG_CTRL_IFACE_UDP */


#ifdef  __cplusplus
}
#endif

#endif /* _FINGER_CTRL_H_ */
