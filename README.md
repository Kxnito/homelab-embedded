# Homelab & Embedded Systems

Distributed embedded telemetry pipeline and homelab infrastructure

---

## Architecture

STM32 Nucleo-F446RE (bare metal C, MPU-6050 over I2C)
    ↓ UART
Raspberry Pi 5 (Python edge node, rolling average filter)
    ↓ HTTP / Prometheus metrics
Dell OptiPlex 3070 (Proxmox → Ubuntu Server VM → Docker)
    ↓ Prometheus scrape
Grafana dashboard → grafana.kenny-lab.com (Cloudflare Tunnel)

---

## Embedded Firmware (STM32 Nucleo-F446RE)

ARM Cortex-M4, 180MHz, bare metal C — no HAL library

| Driver | Description |
|--------|-------------|
| LED blink | Bare metal GPIO — confirms toolchain end to end |
| UART driver | Bare metal USART2 at 115200 baud, direct register access |
| I2C + MPU-6050 | Bare metal I2C1, reads accelerometer X/Y/Z over I2C |

All drivers written from scratch using memory-mapped register access.

---

## Infrastructure (Dell OptiPlex 3070)

| Component | Details |
|-----------|---------|
| Hypervisor | Proxmox VE 9.2 on bare metal |
| VM | Ubuntu Server 26.04 LTS |
| Containers | Docker — Prometheus, Grafana, Cloudflare Tunnel |
| Monitoring | Node Exporter → Prometheus → Grafana |
| Public URL | grafana.kenny-lab.com via Cloudflare Tunnel |

---

## Edge Node (Raspberry Pi 5)

- Receives raw UART sensor data from STM32 over USB serial
- Applies rolling average filter to reduce sensor noise
- Exposes filtered metrics via Prometheus HTTP endpoint
- Runs as a systemd service — auto-starts on boot

---

## Hardware

| Device | Specs |
|--------|-------|
| Dell OptiPlex 3070 SFF | i5-9500, 16GB RAM, 256GB NVMe |
| STM32 Nucleo-F446RE | ARM Cortex-M4, 180MHz, 512KB Flash |
| Raspberry Pi 5 | Edge processing node |
| MPU-6050 | 6-axis IMU (accelerometer + gyroscope) |
| Cisco SG300-10 | Managed switch (Phase 2) |
