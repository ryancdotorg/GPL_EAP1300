### Package Version (Mandatory)

(opkg list pkg-name)

### Which Model/version/Platform/Codebase encountered

- Modle: (e.g. ECW120)_

- Version: (cat /etc/version |head -1)

- Kernel: (uanem -a)

### Summary

(Summarize the bug encountered concisely)

### Steps to reproduce

(How one can reproduce the issue - this is very important)

### What is the current *bug* behavior?

(What actually happens)

### What is the expected *correct* behavior?

(What you should see instead)

### Collection package configs

- Package Config:
```
cat /etc/config/syskey
```

- Other Config:
```
cat /proc/sys/net/bridge/bridge-nf-call-iptables
cat /proc/sys/net/bridge/bridge-nf-call-ip6tables
ebtables -L
```

### Collection logs and/or screenshots

(Paste any relevant logs - please use code blocks (```) to format console output,
logs, and code as it's tough to read otherwise.)


/label ~Bug

