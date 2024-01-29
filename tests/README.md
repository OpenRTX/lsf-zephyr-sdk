# 测试代码说明

## 测试代码分类
tests/audio/track

播音测试

tests/audio/recorder

录音录音测试

tests/thinker/wasm

推理测试

tests/littlefs

文件系统测试

### 初始化工程
```
git clone https://cloud.listenai.com/listenai/lsf-zephyr-sdk.git
lisa zep init-app
```

### 更新依赖
```
lisa zep update
```

### 播音测试

#### 烧录 DSP 固件
```
# 注意：COMx 指的是你的串口地址，在 Linux 是 /dev/ttyUSBx
lisa zep exec cskburn -s COMx -C 6 -b 748800 0x100000 resources/dsp_firmware.bin
```

#### 烧录PCM音频文件
```
lisa zep exec cskburn -s COM12 -C 6 -b 748800 0x200000 resources/test.pcm
```

#### 烧录AAC音频文件
```
lisa zep exec cskburn -s COM12 -C 6 -b 748800 0x300000 resources/shihao16k.aac
```

#### 编译工程并烧录
```
lisa zep build -p -b csk6012_nano -s tests/audio/track
lisa zep flash -r csk --port COMx
```

### 录音测试

#### 编译工程并烧录
```
lisa zep build -p -b csk6012_nano -s tests/audio/recorder
lisa zep flash -r csk --port COMx
```

### 推理测试

#### 烧录WASM工程
[resnet_sample](https://cloud.listenai.com/listenai/wasm/sample_resnet18)
建议在linux环境下编译此工程

resnet_sample的两个依赖

[下载wasi-sdk](https://github.com/WebAssembly/wasi-sdk/releases/tag/wasi-sdk-17)

[wasm-sdk](https://cloud.listenai.com/listenai/wasm/wasm-sdk)

```
lisa zep exec cskburn -s COM12 -C 6 -b 748800 0x200000 ..\sample_resnet18\build\thinker_resnet18.aot
```

#### 烧录模型
```
lisa zep exec cskburn -s COM12 -C 6 -b 748800 0x300000 .\resources\renet18-12-regular.bin
```

#### 编译工程并烧录
```
lisa zep build -p -b csk6012_nano -s tests/thinker/wasm
lisa zep flash -r csk --port COMx
```

### 文件系统测试

#### 烧录文件系统
```
lisa zep exec cskburn -s COM12 -C 6 -b 748800 0x700000 .\resources\littlefs_image.bin
```

#### 编译工程并烧录
```
lisa zep build -p -b csk6012_nano -s tests/littlefs
lisa zep flash -r csk --port COMx
```

### VAD测试

#### 编译工程并烧录
```
lisa zep build -p -b csk6012_nano -s samples\voice_wakeup
lisa zep flash -r csk --port COMx
```