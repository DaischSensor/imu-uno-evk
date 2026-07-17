<div align="right">

[English](../en/Data_Passthrough_Configuration.md) | **简体中文**

</div>

# 数据透传配置

## 为什么需要配置数据透传？

上位机通过 Type-C 接口连接开发套件，但 STM32U585 芯片本身无法直接通过 Type-C 输出数据。数据必须经由 QRB2210 芯片中转，因此需要运行转发脚本来实现这一功能。

## 如何启动数据透传？

### 1. 安装 Python 串口库
```bash
sudo apt update
sudo apt install python3-serial
```

### 2. 停用相关串口的系统服务

```bash
# 停止并禁用 arduino-router 主服务（占用 ttyHS1）
sudo systemctl stop arduino-router.service
sudo systemctl disable arduino-router.service
sudo systemctl mask arduino-router.service
# 停止并禁用串口代理服务（占用 ttyGS0）
sudo systemctl stop arduino-router-serial.service
sudo systemctl disable arduino-router-serial.service
sudo systemctl mask arduino-router-serial.service
# 清理其他相关服务
sudo systemctl stop arduino-app-cli.service arduino-router.service arduino-router-serial.service
sudo systemctl disable arduino-app-cli.service arduino-router.service arduino-router-serial.service
sudo systemctl mask arduino-router.service arduino-router-serial.service
```

### 3. 确认端口已被释放
```bash
sudo lsof /dev/ttyHS1      # 查看哪些进程还在占用 HS1
sudo lsof /dev/ttyGS0      # 查看哪些进程还在占用 GS0
```
> 若无输出结果，说明端口已完全释放，可正常使用。

### 4.启动脚本
将 [stm32_to_usb_gadget_bridge.py](../../host_tools/stm32_to_usb_gadget_bridge.py) 推送至 Debian 系统后，在该Python文件路径下执行以下命令即可启动：

```bash
python3 stm32_to_usb_gadget_bridge.py
```

## （可选）设置开机自启

若希望设备上电后自动运行转发脚本，可通过 systemd 服务实现。

### 步骤一：创建服务文件

```bash
sudo nano /etc/systemd/system/usb-bridge.service
```

### 步骤二：写入服务配置

```
[Unit]
Description=USB Gadget Bridge
After=network.target

[Service]
ExecStart=/usr/bin/python3 /path/to/stm32_to_usb_gadget_bridge.py
WorkingDirectory=/path/to/
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

> ⚠️ 请将 `/path/to/` 替换为脚本所在的实际路径。

### 步骤三：启用并启动服务

```bash
sudo systemctl enable usb-bridge.service
sudo systemctl start usb-bridge.service
```

> 💡 注意：由于数据转发依赖于 Debian 系统的完整启动，设置为开机自启后，上电需等待 Debian 启动完成（约 25 s），方可正常输出数据。