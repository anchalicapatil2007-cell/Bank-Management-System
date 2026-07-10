<h1 align="center">🏦 Advanced Bank Management System</h1>

<p align="center">
A feature-rich <b>console-based Bank Management System</b> developed in <b>C</b> using <b>File Handling</b>, <b>libcurl</b>, and the <b>Telegram Bot API</b>. The system securely manages banking operations while sending real-time Telegram notifications after every important transaction.
</p>

<p align="center">
<img src="https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white">
<img src="https://img.shields.io/badge/Compiler-GCC-red?style=for-the-badge&logo=gnu">
<img src="https://img.shields.io/badge/Platform-Windows-lightgrey?style=for-the-badge">
<img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge">
</p>

---

# 📖 Overview

The **Advanced Bank Management System** is a console-based application developed in **C** to simulate real-world banking operations. It demonstrates practical implementation of **file handling**, **structured programming**, **authentication**, **transaction management**, **CSV export**, and **Telegram Bot API integration** using **libcurl**.

The project securely stores customer records using files and automatically sends Telegram notifications whenever banking transactions are performed.

---

# ✨ Features

- 👤 Create New Account
- ✅ Duplicate Account Validation
- 🔐 Secure PIN Authentication
- 🔑 Change PIN
- 💰 Deposit Money
- 💸 Withdraw Money
- 🔄 Fund Transfer
- 💳 Balance Inquiry
- 📄 Account Statement
- 👨‍💼 Admin Dashboard
- 🔓 Unlock Locked Accounts
- 🗑 Delete Account
- 📊 Export Customer Data to CSV
- 📝 Activity Logging
- 📲 Real-Time Telegram Notifications

---

# 🛠 Technologies Used

| Technology | Purpose |
|------------|---------|
| C Programming | Core Application |
| GCC (MinGW-w64) | Compilation |
| File Handling | Data Storage |
| libcurl | HTTP Requests |
| Telegram Bot API | Real-Time Notifications |
| CSV | Data Export |

---

# 🏗 System Workflow

```text
                   User
                     │
                     ▼
               Main Menu
                     │
      ┌──────────────┼──────────────┐
      ▼              ▼              ▼
Create Account   Transactions     Admin
                     │
                     ▼
          Update Customer Records
                     │
                     ▼
            Generate Activity Log
                     │
                     ▼
           Export CSV (Optional)
                     │
                     ▼
      Send Telegram Notification
```

---

# 📂 Project Structure

```text
Bank-Management-System/
│
├── banking1.c
├── README.md
├── LICENSE
├── .gitignore
│
├── screenshots/
│   ├── main_menu.png
│   ├── create_account.png
│   ├── account_created.png
│   ├── deposit_money.png
│   ├── withdraw_money.png
│   ├── change_pin.png
│   ├── delete_account.png
│   ├── unlock_account.png
│   ├── admin_dashboard.png
│   ├── activity.png
│   ├── export_csv.png
│   └── telegram_notification.png
│
├── bank_accounts.dat
├── bank_export.csv
├── bank_export.md
├── bank_log.txt
└── libcurl-x64.dll
```

---

# ⚙ Installation

## Clone Repository

```bash
git clone https://github.com/anchalicapatil2007-cell/Bank-Management-System.git
```

---

# 🔨 Compilation

```bash
gcc banking1.c -I"C:\curl-8.21.0_2-win64-mingw\include" -L"C:\curl-8.21.0_2-win64-mingw\lib" -o banking.exe -lcurl
```

---

# ▶ Run

```bash
banking.exe
```

---

# 📲 Telegram Configuration

Create your Telegram Bot using **@BotFather**.

Replace the placeholders inside your source code:

```c
#define TELEGRAM_BOT_TOKEN "YOUR_BOT_TOKEN"
#define TELEGRAM_CHAT_ID "YOUR_CHAT_ID"
```

> **Important:** Never upload your real Bot Token or Chat ID to GitHub. Always keep them private.

---

# 📸 Project Screenshots

## 🏠 Main Menu

<p align="center">
<img src="screenshots/main_menu.png" width="900">
</p>

The main interface providing access to all banking operations.

---

## ➕ Create Account

<p align="center">
<img src="screenshots/create_account.png" width="900">
</p>

Register a new customer account with the required banking details.

---

## ✅ Account Created Successfully

<p align="center">
<img src="screenshots/account_created.png" width="900">
</p>

Displays successful account creation after validating customer information.

---

## 💰 Deposit Money

<p align="center">
<img src="screenshots/deposit_money.png" width="900">
</p>

Deposit money into an account while automatically updating the balance.

---

## 💸 Withdraw Money

<p align="center">
<img src="screenshots/withdraw_money.png" width="900">
</p>

Withdraw funds securely after successful authentication.

---

## 🔑 Change PIN

<p align="center">
<img src="screenshots/change_pin.png" width="900">
</p>

Allows users to update their account PIN securely.

---

## 🗑 Delete Account

<p align="center">
<img src="screenshots/delete_account.png" width="900">
</p>

Administrator can permanently remove customer accounts.

---

## 🔓 Unlock Account

<p align="center">
<img src="screenshots/unlock_account.png" width="900">
</p>

Unlock customer accounts after multiple incorrect login attempts.

---

## 👨‍💼 Admin Dashboard

<p align="center">
<img src="screenshots/admin_dashboard.png" width="900">
</p>

Administrative panel for managing customer accounts and system operations.

---

## 📋 Activity Log

<p align="center">
<img src="screenshots/activity.png" width="900">
</p>

Displays the transaction history and system activity logs.

---

## 📊 Export CSV

<p align="center">
<img src="screenshots/export_csv.png" width="900">
</p>

Export customer records into CSV format for reporting and backup.

---

## 📲 Telegram Notification

<p align="center">
<img src="screenshots/telegram_notification.png" width="450">
</p>

Real-time Telegram notifications are automatically sent after successful banking transactions using the Telegram Bot API integrated through **libcurl**.

---

# 🎯 Learning Outcomes

This project strengthened practical knowledge of:

- C Programming
- Structured Programming
- File Handling
- Data Validation
- Authentication
- REST API Integration
- libcurl
- Git & GitHub
- Console Application Development
- Software Documentation

---

# 🚀 Future Enhancements

- Password Encryption
- SQLite Database Integration
- Graphical User Interface (GUI)
- Email Notifications
- QR Code Payments
- Interest Calculation
- Cloud Database Support
- Multi-user Authentication

---

# 👩‍💻 Author

**Anchalica Shital Patil**

B.Tech – Artificial Intelligence & Data Science

Vishwakarma Institute of Technology, Pune

**GitHub:**  
https://github.com/anchalicapatil2007-cell

---

# 📄 License

This project is licensed under the **MIT License**.

---

# ⭐ Support

If you found this project useful, consider giving it a ⭐ on GitHub.

Your support motivates further development and improvements.
