#ifndef _PHYLINK_H_
#define _PHYLINK_H_

#if 1//def DEBUG
#define DPRINTF(x, ...) do { \
	printf("[phylink] %s:%d " x, __func__, __LINE__, ##__VA_ARGS__ ); \
	} while(0)
#define dbg_printk(x, ...) do { \
	printk(x, ##__VA_ARGS__); \
	} while(0)
#else
#define DPRINTF(x, ...) do { \
	} while(0)
#define dbg_printk(x...) do { \
	} while(0)
#endif

#endif
