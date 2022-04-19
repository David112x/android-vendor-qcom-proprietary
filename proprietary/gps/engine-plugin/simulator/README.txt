
Download tool chain from
https://releases.linaro.org/components/toolchain/binaries/6.4-2017.08/arm-linux-gnueabihf/gcc-linaro-6.4.1-2017.08-x86_64_arm-linux-gnueabihf.tar.xz

-   Unzip folder on to linux machine
    tar xf gcc-linaro-6.4.1-2017.08-x86_64_arm-linux-gnueabihf.tar.xz

Compile reference module with Makefile

-   Add tool chain Path
    export PATH=<Too chainpath>/gcc-linaro-6.4.1-2017.08-x86_64/gcc-linaro-6.4.1-2017.08-x86_64_arm-linux-gnueabihf/bin/:$PATH

-   Copy "EnginePluginApi.h" file to this folder

-   Generate "libep_ref_simulator.so" by running "make all"
    make all
-   Reference Makefile is present in this folder

Test Behavior:
Reference EnginePluginAPISimulator will log engine message to diag logs (0x1C54).
It will log the name of the callback received from the EnginePlugin.

Please set the following configurations in /etc/gps.conf to see the logs:
EP_DIAG_MSG_LOG_LEVEL_MASK = 1
EP_DIAG_MSG_LOG_TYPE_MASK = 8
