# Edge AI Smart Scale (Cân Thông Minh Nhận Diện Trái Cây & Tính Tiền Tự Động)

> **Dự án Bài Tập Lớn Vi Điều Khiển & Hệ Thống Nhúng (MCU & TinyML Embedded System)**  
> Tác giả: **Trần Bảo Khánh** ([@Khanhbaoaiss](https://github.com/Khanhbaoaiss))

---

## 📌 Giới Thiệu Dự Án

**Edge AI Smart Scale** là hệ thống cân thông minh thương mại bán lẻ tích hợp Trí tuệ Nhân tạo tại biên (**Edge AI / TinyML**). Hệ thống kết hợp sức mạnh xử lý thời gian thực của vi điều khiển **STM32F103C8T6** và năng lực thị giác máy tính của vi điều khiển **ESP32-CAM**:

1. **Đo lường khối lượng chính xác:** Sử dụng cảm biến lực **Loadcell** kết hợp module ADC 24-bit chuyên dụng **HX711**, được vi điều khiển STM32 đọc và lọc nhiễu, tự động cân bằng (Tare) về 0.
2. **Nhận diện loại quả bằng Edge AI:** Camera **OV2640** trên **ESP32-CAM** thu nhận hình ảnh và chạy trực tiếp mô hình mạng nơ-ron **FOMO (Faster Objects, More Objects)** đã được lượng tử hóa INT8 (Edge Impulse / TensorFlow Lite Micro). Nhận diện tức thời các loại trái cây (Táo, Cam, Lê,...) mà không cần kết nối Internet.
3. **Hiển thị tại chỗ & Tự động tính tiền:** STM32 giao tiếp UART hai chiều với ESP32-CAM, nhận diện tên quả và đơn giá, tính tổng tiền và hiển thị chi tiết lên màn hình **LCD 1602 (I2C)**.
4. **Hệ thống Web Dashboard & Desktop POS:** ESP32-CAM phát WiFi SoftAP phục vụ Web Dashboard điều khiển thời gian thực; đi kèm ứng dụng **Desktop POS** chuyên nghiệp (giao diện Catppuccin / Hyprland Rice phong cách hiện đại) giúp thanh toán, in hóa đơn và tự động lưu nhật ký giao dịch ra file CSV.

---

## 🏛️ Kiến Trúc Hệ Thống

```
                     ┌───────────────────────────┐
                     │   CẢM BIẾN LOADCELL 5KG   │
                     └─────────────┬─────────────┘
                                   │ (Tín hiệu vi sai)
                     ┌─────────────▼─────────────┐
                     │     MODULE ADC 24-BIT     │
                     │          HX711            │
                     └─────────────┬─────────────┘
                                   │ DT (PB0), SCK (PB1)
                     ┌─────────────▼─────────────┐
                     │      STM32F103C8T6        │◄─── I2C (PB6/PB7) ───► Màn hình LCD 1602
                     │      (Master Metrology)   │                        (Hiển thị Cân & Giá)
                     └─────────────▲─────────────┘
                                   │
                 UART (115200bps)  │ PA9 (TX)  ──► GPIO 14 (RX)
                 Bản tin: W:<g>    │ PA10 (RX) ◄── GPIO 15 (TX)
                                   │
                     ┌─────────────▼─────────────┐
                     │         ESP32-CAM         │◄─── Camera OV2640
                     │   (Edge AI Coprocessor)   │     (TinyML FOMO Model)
                     └──────┬─────────────┬──────┘
                            │             │
                WiFi SoftAP │             │ WebSockets / HTTP REST
          "ESP32-CAM-SCALE" │             │
                            ▼             ▼
               ┌────────────────┐   ┌───────────────────────────┐
               │ Web Dashboard  │   │     Desktop POS App       │
               │ (Trực tiếp     │   │ (Quản lý đơn hàng, in bill│
               │  trên trình    │   │  lưu lịch sử ra file CSV) │
               │  duyệt web)    │   └───────────────────────────┘
               └────────────────┘
```

---

## 📁 Cấu Trúc Thư Mục & Chức Năng Từng Folder

Dưới đây là sơ đồ tổ chức mã nguồn và mô tả chi tiết chức năng của từng thư mục trong dự án:

```
AI Scale/
├── data/                             # Dữ liệu giao dịch và cấu hình POS cấp hệ thống
│   ├── pos_orders_history.csv        # Nhật ký đơn hàng đã thanh toán
│   └── pos_state.json                # Lưu trạng thái, bảng giá, cấu hình POS
├── FruitTinyML/                      # Dự án firmware ESP32-CAM (Edge AI & Web Dashboard)
│   ├── dataset/                      # Tập dữ liệu ảnh mẫu các loại hoa quả (train/test)
│   │   ├── apple/                    # Ảnh mẫu Quả Táo
│   │   ├── orange/                   # Ảnh mẫu Quả Cam
│   │   └── pear/                     # Ảnh mẫu Quả Lê
│   ├── include/                      # Các header nhúng
│   │   └── dashboard_html_gz.h       # Giao diện Web Dashboard nén Gzip lưu trong Flash
│   ├── lib/                          # Thư viện TinyML
│   │   └── a3FruitVision_V1_inferencing/ # Thư viện Edge Impulse C++ Deployment (INT8)
│   ├── scripts/                      # Công cụ Python hỗ trợ ESP32
│   │   ├── collect_dataset_gui.py    # GUI chụp & gắn nhãn dataset từ camera ESP32
│   │   ├── collect_dataset_cli.py    # CLI chụp dataset dòng lệnh
│   │   ├── compress_dashboard.py     # Script nén HTML thành header C
│   │   ├── pre_build_compress.py     # Hook tự động nén HTML trước khi build PlatformIO
│   │   └── pos_desktop_app.py        # Mã nguồn ứng dụng POS quản lý bán hàng
│   ├── src/                          # Mã nguồn firmware C++ của ESP32-CAM
│   │   ├── main.cpp                  # Firmware chính: AI FOMO + WebServer + UART STM32
│   │   └── gather.cpp                # Firmware phụ: Thu thập ảnh stream từ camera
│   ├── platformio.ini                # Cấu hình biên dịch PlatformIO cho ESP32-CAM
│   ├── Rice_Dashboard.html           # Giao diện Dashboard giao diện hiện đại (Rice Edition)
│   ├── smart_scale_dashboard.html    # Giao diện Dashboard bản tiêu chuẩn
│   ├── Run_POS_App.bat               # File chạy nhanh ứng dụng POS
│   └── run_collector.bat             # File chạy nhanh công cụ chụp dataset
├── scripts/                          # Thư mục scripts ở cấp thư mục gốc
│   └── pos_desktop_app.py            # Bản khởi chạy POS Desktop tại thư mục gốc
├── STM32/                            # Dự án vi điều khiển STM32F103 (Keil C MDK-ARM)
│   ├── delay.c / delay.h             # Module tạo trễ chính xác bằng Timer phần cứng TIM2
│   ├── hx711.c / hx711.h             # Driver đọc ADC 24-bit từ cảm biến Loadcell HX711
│   ├── i2c_soft.c / i2c_soft.h       # Thư viện mô phỏng I2C phần mềm (Software I2C)
│   ├── lcd1602.c / lcd1602.h         # Driver điều khiển màn hình LCD 1602 qua chip PCF8574
│   ├── uart_esp32.c / uart_esp32.h   # Driver xử lý giao tiếp UART USART1 với ESP32-CAM
│   ├── main.c / main.h               # Luồng xử lý chính: đọc cân, tare, hiển thị, tính tiền
│   ├── LCD_Loadcell.uvprojx          # File dự án Keil uVision MDK-ARM v5
│   ├── RTE/                          # CMSIS Device Drivers & Startup files chuẩn của ARM
│   └── Objects/                      # Chứa file nhị phân biên dịch
│       └── LCD_Loadcell.hex          # Firmware HEX đã biên dịch sẵn (nạp được ngay)
├── .gitignore                        # Cấu hình bỏ qua các file rác biên dịch (.pio, *.o, ...)
├── README.md                         # Tài liệu hướng dẫn chi tiết dự án
└── Run_POS_App.bat                   # Phím tắt click đúp chạy ứng dụng POS từ thư mục gốc
```

---

### 1. Thư mục `STM32/` (Vi Điều Khiển Đo Lường & Điều Khiển Ngoại Vi)

Thư mục này chứa mã nguồn C thuần túy lập trình cho vi điều khiển **STM32F103C8T6** (dòng ARM Cortex-M3 72MHz), phát triển trên môi trường **Keil C uVision 5**:

* **`main.c`, `main.h`**: 
  * Cấu hình xung nhịp hệ thống lên tối đa 72MHz sử dụng thạch anh ngoài HSE 8MHz qua nhân tần PLL.
  * Khởi tạo ngoại vi (TIM2, I2C, USART1, HX711, LCD1602).
  * Quy trình khởi động: Hiển thị màn hình chào `"SMART SCALE"`, tự động lấy 10 mẫu để trừ bì điểm 0 (**Tare**) cho cảm biến cân.
  * Vòng lặp chính: Liên tục đọc giá trị cân, xử lý làm mịn dữ liệu, phân tích bản tin từ ESP32-CAM và cập nhật màn hình hiển thị.
* **`hx711.c`, `hx711.h`**:
  * Giao tiếp 2 dây (Data: PB0, Clock: PB1) với IC ADC 24-bit HX711.
  * Đọc giá trị raw bù 2 (2's complement), trừ offset điểm không và chia hệ số tỉ lệ (`SCALE_FACTOR = 420.0f`) để quy đổi sang gram.
* **`lcd1602.c`, `lcd1602.h` & `i2c_soft.c`, `i2c_soft.h`**:
  * Điều khiển màn hình LCD1602 qua module chuyển đổi I2C (PCF8574, địa chỉ mặc định `0x4E` / `0x27`).
  * Sử dụng giải pháp Software I2C trên 2 chân GPIO (SCL: PB6, SDA: PB7) để tương thích tối đa và chống treo bus cứng.
  * Hiển thị:
    * **Dòng 1:** Tên hoa quả và khối lượng thời gian thực (ví dụ: `APPLE: 320g`).
    * **Dòng 2:** Đơn giá và thành tiền theo đơn vị VNĐ (ví dụ: `T: 19,200 VND`).
* **`uart_esp32.c`, `uart_esp32.h`**:
  * Quản lý bộ phát hiện ngắt nhận `USART1_IRQHandler` (PA9 TX, PA10 RX) tốc độ 115200bps.
  * Tự động gửi trọng lượng sang ESP32 định dạng `W:<weight>\r\n`.
  * Bộ giải mã nhận chuỗi `RES:<fruit_name>,<confidence>,<price>\r\n` từ ESP32, tách thông tin tên quả và đơn giá để tính tiền.
* **`Objects/LCD_Loadcell.hex`**:
  * File firmware nhị phân nạp trực tiếp qua mạch nạp ST-Link hoặc phần mềm STM32CubeProgrammer / STM32 ST-LINK Utility mà không cần cài Keil C.

---

### 2. Thư mục `FruitTinyML/` (ESP32-CAM Edge AI & Web Server)

Thư mục này cấu trúc theo chuẩn **PlatformIO**, đảm nhiệm toàn bộ phần xử lý thông minh và kết nối mạng:

* **`src/main.cpp`**:
  * Khởi tạo phần cứng camera **OV2640** với cấu hình tối ưu bộ nhớ đệm DMA và PSRAM ngoài (4MB).
  * Khởi tạo mô hình Edge Impulse FOMO (`run_classifier`). Cắt khung hình, tiền xử lý và chạy suy luận mạng nơ-ron nhận dạng hoa quả trực tiếp trên chip với thời gian đáp ứng cực nhanh (~100-150ms).
  * Khởi tạo mạng WiFi SoftAP với SSID: `ESP32-CAM-SCALE` (IP: `192.168.4.1`).
  * Cung cấp HTTP Web Server: truyền luồng camera MJPEG (`/stream`), REST API lấy trạng thái cân & nhận diện (`/status`), cập nhật bảng giá (`/set_price`).
  * Giao tiếp UART với STM32 (Serial2 trên GPIO 14/15) để đồng bộ hóa khối lượng và kết quả AI.
* **`src/gather.cpp`**:
  * Firmware chuyên dụng dùng khi cần lấy thêm dữ liệu ảnh: cung cấp luồng video độ phân giải cao phục vụ công cụ chụp dataset.
* **`lib/a3FruitVision_V1_inferencing/`**:
  * Thư viện nhúng C++ chứa toàn bộ kiến trúc mô hình học sâu, trọng số (weights) INT8 và thư viện tăng tốc **ESP-NN** của Espressif.
* **`dataset/`**:
  * Chứa tập dữ liệu ảnh mẫu thực tế chụp từ camera OV2640 phân loại theo từng thư mục nhãn:
    * `apple/`: Các góc chụp quả táo trong các điều kiện ánh sáng khác nhau.
    * `orange/`: Ảnh mẫu quả cam.
    * `pear/`: Ảnh mẫu quả lê.
* **`scripts/`**:
  * `collect_dataset_gui.py`: Ứng dụng giao diện đồ họa Python (Tkinter) kết nối đến luồng stream ESP32-CAM, cho phép xem hình trực tiếp, bấm phím tắt chụp ảnh, tự động gán nhãn và lưu vào thư mục dataset tương ứng.
  * `compress_dashboard.py` / `pre_build_compress.py`: Tự động nén mã nguồn HTML/CSS/JS của dashboard thành định dạng Gzip nhị phân để nhúng trực tiếp vào Flash của ESP32 (`dashboard_html_gz.h`), giúp tải trang tức thì và tiết kiệm 80% bộ nhớ.
  * `pos_desktop_app.py`: Ứng dụng Desktop POS quản lý bán hàng.
* **`Rice_Dashboard.html` & `smart_scale_dashboard.html`**:
  * Giao diện Web Dashboard hiển thị camera stream, bounding box nhận diện, đồng hồ cân nặng kỹ thuật số, đơn giá, tổng tiền và giỏ hàng.
* **`platformio.ini`**:
  * Cấu hình PlatformIO: Chip `esp32cam`, Framework `arduino`, xung nhịp CPU `240MHz`, Flash `80MHz QIO`, cờ tối ưu hóa bộ nhớ đệm PSRAM `-DBOARD_HAS_PSRAM` và `-DEI_CLASSIFIER_TFLITE_ENABLE_ESP_NN=1`.

---

### 3. Thư mục `scripts/` (Bộ Công Cụ Tiện Ích Cấp Dự Án)

* **`scripts/pos_desktop_app.py`**:
  * Ứng dụng máy tính POS (Point of Sale) chuyên nghiệp viết bằng Python.
  * Chạy server nền cục bộ đa luồng (`ThreadingHTTPServer` trên cổng `8765`), tự động mở cửa sổ giao diện ứng dụng native không viền (dựa trên WebView / Edge App mode).
  * Tự động kết nối đến ESP32-CAM qua mạng WiFi (`192.168.4.1`) để nhận dữ liệu cân và kết quả AI thời gian thực.
  * Cho phép chỉnh sửa bảng giá từng loại hoa quả, bấm in hóa đơn, trừ kho, và tự động đồng bộ hóa lịch sử đơn hàng vào file CSV trên máy tính.

---

### 4. Thư mục `data/` (Cơ Sở Dữ Liệu Bán Hàng)

* **`pos_orders_history.csv`**:
  * File nhật ký giao dịch lưu chuẩn mã hóa UTF-8 với BOM (`utf-8-sig`) để mở mượt mà trên Microsoft Excel mà không bị lỗi font tiếng Việt.
  * Các trường dữ liệu gồm: `Mã đơn`, `Thời gian giao dịch`, `Tên loại quả`, `Khối lượng (gram)`, `Đơn giá (VNĐ/kg)`, `Thành tiền (VNĐ)`.
* **`pos_state.json`**:
  * File cấu hình JSON lưu trạng thái hoạt động: số thứ tự đơn hàng tiếp theo (`next_order_no`), bảng giá hiện tại (`prices`), địa chỉ IP của cân (`esp_ip`), chế độ tự động thêm đơn (`auto_add`), giao diện theme ban đêm (`night`).

---

## 🔌 Sơ Đồ Đấu Nối Phần Cứng (Pinout Wiring)

### 1. Kết Nối Giữa STM32F103C8T6 và Cảm Biến / Màn Hình

| Thiết Bị | Chân Thiết Bị | Chân STM32 | Chú Thích |
| :--- | :--- | :--- | :--- |
| **Module HX711** | VCC | 5V / 3.3V | Nguồn cấp cho IC ADC |
| | GND | GND | Nối mass chung |
| | DT (Data) | **PB0** | Tín hiệu dữ liệu cân |
| | SCK (Clock) | **PB1** | Xung nhịp điều khiển đọc |
| **Màn Hình LCD 1602 (I2C)** | VCC | 5V | Nguồn cấp màn hình |
| | GND | GND | Nối mass chung |
| | SDA | **PB7** | I2C Data (Software I2C) |
| | SCL | **PB6** | I2C Clock (Software I2C) |

---

### 2. Giao Tiếp UART Giữa STM32F103C8T6 và ESP32-CAM

> [!IMPORTANT]
> Bắt buộc phải **nối chung Mass (GND)** giữa nguồn cấp của STM32 và nguồn cấp của ESP32-CAM để mức điện áp logic được tham chiếu chính xác.

| STM32F103C8T6 | Hướng Tín Hiệu | ESP32-CAM (AI-Thinker) | Chức Năng |
| :--- | :---: | :--- | :--- |
| **PA9 (USART1_TX)** | ───► | **GPIO 14 (Serial2_RX)** | STM32 truyền giá trị cân (`W:<g>\r\n`) |
| **PA10 (USART1_RX)**| ◄─── | **GPIO 15 (Serial2_TX)** | ESP32 gửi kết quả AI & giá tiền (`RES:...`) |
| **GND** | ◄───► | **GND** | Nối chung mass hệ thống |

---

## 📡 Giao Thức Truyền Nhận UART (Communication Protocol)

Hệ thống sử dụng giao thức truyền thông dạng chuỗi ký tự ASCII kết thúc bằng ký tự xuống dòng `\r\n`, tốc độ truyền **115200 bps, 8-N-1**:

1. **Từ STM32 gửi sang ESP32-CAM:**
   * Cú pháp: `W:<trọng_lượng_gram>\r\n`
   * Ví dụ: `W:285\r\n` (thông báo khối lượng vật phẩm đặt trên cân là 285g).
2. **Từ ESP32-CAM gửi sang STM32:**
   * Cú pháp phản hồi AI: `RES:<tên_quả>,<độ_tin_cậy>,<đơn_giá>\r\n`
   * Ví dụ: `RES:apple,96,60000\r\n` (Nhận diện quả táo với độ tin cậy 96%, đơn giá 60.000 VNĐ/kg).
   * Cú pháp yêu cầu cân: `REQ\r\n` (Yêu cầu STM32 phản hồi ngay khối lượng tức thời).
3. **Thuật toán tính tiền trên STM32:**
   $$\text{Thành tiền (VNĐ)} = \frac{\text{Khối lượng (g)} \times \text{Đơn giá (VNĐ/kg)}}{1000}$$

---

## 🚀 Hướng Dẫn Biên Dịch & Nạp Code

### 1. Nạp Firmware Cho STM32F103C8T6
* **Cách 1 (Nhanh nhất - Không cần cài Keil C):**
  1. Kết nối mạch nạp ST-Link V2 với STM32 (SWCLK, SWDIO, 3.3V, GND).
  2. Mở phần mềm **STM32CubeProgrammer** hoặc **STM32 ST-LINK Utility**.
  3. Chọn file `STM32/Objects/LCD_Loadcell.hex` và nhấn **Program & Verify**.
* **Cách 2 (Mở bằng Keil C):**
  1. Mở file `STM32/LCD_Loadcell.uvprojx` bằng Keil uVision MDK-ARM v5.
  2. Nhấn `F7` để Build project.
  3. Nhấn `F8` (Download) để nạp vào vi điều khiển.

---

### 2. Nạp Firmware Cho ESP32-CAM
1. Cài đặt **Visual Studio Code** và extension **PlatformIO IDE**.
2. Mở thư mục `FruitTinyML/` trong VS Code.
3. Nối ESP32-CAM với máy tính thông qua mạch nạp USB-to-UART (CP2102/FT232RL):
   * ESP32 U0T -> Adapter RXD
   * ESP32 U0R -> Adapter TXD
   * Nối chân **GPIO 0 với GND** khi bấm nút Reset để đưa ESP32 vào chế độ nạp (Flash mode).
4. Nhấn nút **Upload** trên thanh công cụ PlatformIO để nạp môi trường mặc định `env:esp32cam`.
5. Sau khi nạp xong, rút chân GPIO 0 ra khỏi GND và nhấn Reset để chip bắt đầu hoạt động.

---

### 3. Khởi Chạy Ứng Dụng Bán Hàng (POS Desktop App)
1. Kết nối máy tính vào mạng WiFi do cân phát ra: `ESP32-CAM-SCALE` (mạng mở, không có mật khẩu).
2. Click đúp vào file `Run_POS_App.bat` ở thư mục gốc của dự án.
3. Ứng dụng POS sẽ tự động khởi tạo server, mở giao diện điều khiển hiện đại, tự động hiển thị camera stream, nhận diện trái cây, cân nặng và cho phép bấm thanh toán đơn hàng.

---

## 👨‍💻 Tác Giả & Đóng Góp

* **Trần Bảo Khánh** - *Edge AI & Embedded Firmware Developer* - [GitHub Profile](https://github.com/Khanhbaoaiss)
* Kho lưu trữ dự án: [Edge-AI-Smart-Scale-BTL-MCU](https://github.com/Khanhbaoaiss/Edge-AI-Smart-Scale-BTL-MCU.git)
