LSF Zephyr SDK
==============
## 快速上手

#### 1. 初始化工程

```sh
git clone https://cloud.listenai.com/listenai/lsf-zephyr-sdk.git
lisa zep init-app
```

#### 2. 烧录 DSP 固件

```sh
# 注意：COMx 指的是你的串口地址，在 Linux 是 /dev/ttyUSBx
lisa zep exec cskburn -s COMx -C 6 -b 748800 0x100000 resources/dsp_firmware.bin
```

#### 3. 编译示例工程并烧录

[`samples`](samples) 目录包含了一些单点功能的参考示例，以 [`samples/audio_recorder`](samples/audio_recorder) 为例：

```sh
# 注意：COMx 指的是你的串口地址，在 Linux 是 /dev/ttyUSBx
lisa zep build -p -b csk6012_nano -s samples/audio_recorder
lisa zep flash -r csk --port COMx
```
