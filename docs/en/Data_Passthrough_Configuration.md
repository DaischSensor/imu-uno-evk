<div align="right">

**English** | [简体中文](../zh-CN/Data_Passthrough_Configuration.zh-CN.md)

</div>

# Data Transparent Transmission Configuration

## Why Do You Need to Configure Data Transparent Transmission?

The host PC connects to the development kit via the Type‑C interface, but the STM32U585 chip itself cannot directly output data through Type‑C. The data must be relayed via the QRB2210 chip, so a forwarding script is required to enable this functionality.

## How to Start Data Transparent Transmission?

### 1. Install the Python serial library
```bash
sudo apt update
sudo apt install python3-serial
```

### 2. Disable system services that occupy the relevant serial ports

```bash
# Stop and disable the main arduino-router service (occupies ttyHS1)
sudo systemctl stop arduino-router.service
sudo systemctl disable arduino-router.service
sudo systemctl mask arduino-router.service
# Stop and disable the serial proxy service (occupies ttyGS0)
sudo systemctl stop arduino-router-serial.path arduino-router-serial.service
sudo systemctl disable arduino-router-serial.path arduino-router-serial.service
sudo systemctl mask arduino-router-serial.path arduino-router-serial.service
# Clean up other related services
sudo systemctl stop arduino-app-cli.service arduino-router.service arduino-router-serial.service
sudo systemctl disable arduino-app-cli.service arduino-router.service arduino-router-serial.service
sudo systemctl mask arduino-router.service arduino-router-serial.service
```

### 3. Confirm that the ports have been released
```bash
sudo lsof /dev/ttyHS1      # Check which processes are still occupying HS1
sudo lsof /dev/ttyGS0      # Check which processes are still occupying GS0
```
> If there is no output, the ports are completely free and ready for use.

### 4. Start the script
Push [stm32_to_usb_gadget_bridge.py](../../host_tools/stm32_to_usb_gadget_bridge.py) to the Debian system, then navigate to the directory containing the Python file and execute:

```bash
python3 stm32_to_usb_gadget_bridge.py
```

## (Optional) Set Up Auto‑Start on Boot

If you want the forwarding script to run automatically when the device is powered on, you can set it up as a systemd service.

### Step 1: Create the Service File

```bash
sudo nano /etc/systemd/system/usb-bridge.service
```

### Step 2: Write the Service Configuration

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

> ⚠️ Replace `/path/to/` with the actual path where the script is located.

### Step 3: Enable and Start the Service

```bash
sudo systemctl enable usb-bridge.service
sudo systemctl start usb-bridge.service
```

> 💡 Note: Since data forwarding relies on the full startup of the Debian system, after enabling auto‑start, you need to wait for Debian to finish booting (about 25 seconds) before data output is available.