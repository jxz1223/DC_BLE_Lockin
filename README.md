# DC_BLE_Lockin
关于直流传感器的功能代码开发

## 开发板 LED 蓝牙状态提示

P-NUCLEO-WB55 板载绿色 LED 现在专用于显示传感器侧蓝牙链路状态：

- 上电并进入等待连接/广播状态：绿色 LED 每 250 ms 翻转一次。
- HCI 蓝牙连接建立：停止闪烁，绿色 LED 常亮。
- 蓝牙连接断开：绿色 LED 立即熄灭并重新开始闪烁。

闪烁由 RTC Hardware Timer Server 的重复定时器驱动，不使用 `HAL_Delay`，不会阻塞 BLE、ADC、DAC 扫描或数据队列。原来在 ADC 回调、DAC 命令和通知开关中操作绿色 LED 的代码已移除，避免覆盖连接状态。蓝色 LED 继续用于命令/扫描活动，红色 LED 继续用于错误反馈。

调试构建产物：

- `build/Debug/BLE_Custom.elf`
- `build/Debug/BLE_Custom.hex`
- `build/Debug/BLE_Custom.bin`
