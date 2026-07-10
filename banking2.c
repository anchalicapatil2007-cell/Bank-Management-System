/*
 * ============================================================
 *         ADVANCED BANK MANAGEMENT SYSTEM - C PROJECT
 *              First Year C Programming Project
 *       ** WITH TELEGRAM NOTIFICATIONS **
 * ============================================================
 *
 *  Features:
 *   [1]  Create Account         (name/phone validation, min deposit)
 *   [2]  Account Login          (Deposit / Withdraw / Statement)
 *   [3]  Fund Transfer          (Account to Account)
 *   [4]  Admin View             (All Accounts + Locked Status)
 *   [5]  Export to CSV          (Excel-Readable)
 *   [6]  Activity Log           (View Recent Activity)
 *   [7]  Unlock Account         (Admin - unblock after lockout)
 *   [0]  Exit
 *
 *  Security Features Added:
 *   - PIN authentication with 4-digit enforcement
 *   - 3 failed login attempt detection (persistent across sessions)
 *   - Automatic account lock after 3 consecutive wrong PINs
 *   - Admin unlock flow with Telegram notification
 *   - Telegram alerts: lock, unlock, login, deposit, withdraw, transfer
 *   - Input validation: name (letters), phone (10 digits), amounts
 *   - Insufficient balance protection + Savings minimum Rs.500
 *
 * ============================================================
 *  HOW TO SET UP YOUR FREE TELEGRAM BOT:
 * ============================================================
 *   Step 1: Open Telegram, search for @BotFather
 *   Step 2: Send /newbot -- follow the steps, get a BOT TOKEN
 *           (looks like: 123456789:ABCDefgh...)
 *   Step 3: Search for @userinfobot in Telegram
 *           Send it any message -- it replies with your CHAT ID
 *           (looks like: 987654321)
 *   Step 4: Open your new bot and press START (or send /start)
 *   Step 5: Paste your BOT_TOKEN and CHAT_ID below
 *
 *  REQUIREMENTS:
 *   - Internet connection
 *   - Telegram app installed on your phone
 *   - Bot started once with /start
 *   - libcurl installed (see compile instructions below)
 *
 * ============================================================
 *  Compile (Windows MinGW):
 *
 *    gcc banking1.c -I"C:\C_project\curl-8.20.0_2-win64-mingw\include" ^
 *        -L"C:\C_project\curl-8.20.0_2-win64-mingw\lib" ^
 *        -o banking1.exe -lcurl
 *
 *  Run:
 *    .\banking1.exe
 *
 *  IMPORTANT: Keep this DLL in C:\C_project\ before running:
 *    libcurl-x64.dll
 *    (copy from curl-8.20.0_2-win64-mingw\bin\)
 *
 *  Files generated:
 *   bank_accounts.dat  -- binary account data
 *   bank_log.txt       -- activity log
 *   bank_export.csv    -- CSV export
 *   bank_export.md     -- Markdown table
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>

/* ============================================================
   TELEGRAM CONFIGURATION  <-- FILL THESE IN
   ============================================================
   Replace YOUR_BOT_TOKEN with your actual bot token from BotFather
   Replace YOUR_CHAT_ID  with your actual chat ID from @userinfobot
   ============================================================ */
#define TELEGRAM_BOT_TOKEN  "8691927942:AAHMGoymWTwTpTh51crWPXIZ-gFzPHNFsos"
#define TELEGRAM_CHAT_ID    "8610531573"

/* ============================================================
   CONSTANTS
   ============================================================ */
#define MAX_TX        10
#define MAX_ACCOUNTS  500
#define DATA_FILE     "bank_accounts.dat"
#define LOG_FILE      "bank_log.txt"
#define CSV_FILE      "bank_export.csv"
#define MD_FILE       "bank_export.md"
#define PIN_ATTEMPTS  3

/* ============================================================
   STRUCTS
   ============================================================ */
typedef struct {
    char  type[30];
    float amount;
    float balAfter;
    char  date[25];
} Transaction;

typedef struct {
    int         accNo;
    char        name[50];
    char        phone[15];
    char        accType[15];
    float       balance;
    int         pin;
    int         active;
    Transaction history[MAX_TX];
    int         txCount;
    int         totalTx;
    char        createdAt[25];
    int         failedAttempts;   /* Count of consecutive wrong PINs  */
    int         isLocked;         /* 1 = account locked after 3 fails */
} Account;

/* ============================================================
   FORWARD DECLARATIONS
   ============================================================ */
void clearBuffer(void);
void pause(void);
void getCurrentDate(char *buf);
void clearScreen(void);
void addTransaction(Account *a, const char *type, float amount, float bal);
void logActivity(int accNo, const char *action);
int  readAll(Account *list);
void writeAll(Account *list, int count);
int  findAcc(Account *list, int count, int accNo);
int  accExists(Account *list, int count, int accNo);
void header(void);
void sectionTitle(const char *title);
void printLine(char c, int len);
void printReceipt(const char *title, int accNo, const char *name,
                  const char *txType, float amount, float balAfter);
int  verifyPIN(Account *a);
void createAccount(void);
void accountLogin(void);
void accountDashboard(Account *a, Account *list, int count);
void depositMoney(Account *a, Account *list, int count);
void withdrawMoney(Account *a, Account *list, int count);
void miniStatement(Account *a);
void changePIN(Account *a, Account *list, int count);
void deleteAccountFromDash(Account *a, Account *list, int count);
void transferFunds(void);
void adminView(void);
void unlockAccount(void);
void exportCSV(void);
void viewActivityLog(void);

/* Telegram Function */
void sendTelegram(const char *message);

/* ============================================================
   TELEGRAM: SEND MESSAGE
   Sends a text message to your Telegram bot chat.
   Uses HTTP GET via libcurl (simple and reliable).
   ============================================================ */
void sendTelegram(const char *message) {

    /* Skip if not configured */
    if (strcmp(TELEGRAM_BOT_TOKEN, "YOUR_BOT_TOKEN") == 0 ||
        strcmp(TELEGRAM_CHAT_ID,   "YOUR_CHAT_ID")   == 0) {
        printf("  [Telegram] Token/Chat ID not set. Skipping alert.\n");
        return;
    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (!curl) {
        printf("  [Telegram] CURL init failed.\n");
        return;
    }

    /* URL-encode the message text so spaces and special chars work */
    char *encodedMsg = curl_easy_escape(curl, message, 0);
    if (!encodedMsg) {
        printf("  [Telegram] Message encoding failed.\n");
        curl_easy_cleanup(curl);
        return;
    }

    /* Build the Telegram Bot API URL */
    char url[2048];
    snprintf(url, sizeof(url),
        "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s",
        TELEGRAM_BOT_TOKEN,
        TELEGRAM_CHAT_ID,
        encodedMsg);

    curl_easy_setopt(curl, CURLOPT_URL,           url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT,        10L);

    /* Suppress response output to terminal */
    FILE *devnull = fopen(
#ifdef _WIN32
        "NUL",
#else
        "/dev/null",
#endif
        "wb");
    if (devnull) {
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);
    }

    res = curl_easy_perform(curl);

    if (devnull) fclose(devnull);

    if (res == CURLE_OK) {
        printf("  [Telegram] Alert sent successfully!\n");
    } else {
        printf("  [Telegram] Failed: %s\n", curl_easy_strerror(res));
    }

    curl_free(encodedMsg);
    curl_easy_cleanup(curl);
}

/* ============================================================
   UTILITY FUNCTIONS
   ============================================================ */
void clearBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause(void) {
    printf("\n  Press Enter to continue...");
    clearBuffer();
}

void getCurrentDate(char *buf) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, 25, "%d/%m/%Y %H:%M:%S", tm_info);
}

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void printLine(char c, int len) {
    int i;
    for (i = 0; i < len; i++) putchar(c);
    putchar('\n');
}

void addTransaction(Account *a, const char *type, float amount, float bal) {
    int idx = a->txCount % MAX_TX;
    strncpy(a->history[idx].type, type, sizeof(a->history[idx].type) - 1);
    a->history[idx].type[sizeof(a->history[idx].type) - 1] = '\0';
    a->history[idx].amount   = amount;
    a->history[idx].balAfter = bal;
    getCurrentDate(a->history[idx].date);
    a->txCount++;
    a->totalTx++;
}

void logActivity(int accNo, const char *action) {
    FILE *fp = fopen(LOG_FILE, "a");
    if (!fp) return;
    char date[25];
    getCurrentDate(date);
    fprintf(fp, "[%s]  Acc#%-6d  -->  %s\n", date, accNo, action);
    fclose(fp);
}

/* ============================================================
   FILE HELPERS
   ============================================================ */
int readAll(Account *list) {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) return 0;
    int n = 0;
    while (n < MAX_ACCOUNTS && fread(&list[n], sizeof(Account), 1, fp) == 1)
        n++;
    fclose(fp);
    return n;
}

void writeAll(Account *list, int count) {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) { printf("  [ERROR] Cannot write data file!\n"); return; }
    fwrite(list, sizeof(Account), count, fp);
    fclose(fp);
}

int findAcc(Account *list, int count, int accNo) {
    int i;
    for (i = 0; i < count; i++)
        if (list[i].accNo == accNo && list[i].active)
            return i;
    return -1;
}

int accExists(Account *list, int count, int accNo) {
    int i;
    for (i = 0; i < count; i++)
        if (list[i].accNo == accNo)
            return 1;
    return 0;
}

/* ============================================================
   UI HELPERS
   ============================================================ */
void header(void) {
    clearScreen();
    printLine('=', 65);
    printf("  ____    _    _   _ _  __  ______   ____  \n");
    printf(" | __ )  / \\  | \\ | | |/ / / ___\\ \\ / / _| \n");
    printf(" |  _ \\ / _ \\ |  \\| | ' /  \\___ \\ V /\\_ \\ \n");
    printf(" | |_) / ___ \\| |\\  | . \\   ___) || |  __) \n");
    printf(" |____/_/   \\_\\_| \\_|_|\\_\\ |____/ |_| |___| \n");
    printf("\n");
    printf("       ADVANCED BANK MANAGEMENT SYSTEM  v3.0\n");
    printf("         with Telegram Alerts (FREE)\n");
    printLine('=', 65);
}

void sectionTitle(const char *title) {
    printf("\n");
    printLine('-', 65);
    printf("  >>> %s\n", title);
    printLine('-', 65);
}

/* ============================================================
   RECEIPT PRINTER
   ============================================================ */
void printReceipt(const char *title, int accNo, const char *name,
                  const char *txType, float amount, float balAfter) {
    char date[25];
    getCurrentDate(date);
    printf("\n");
    printLine('=', 55);
    printf("  %*s%s\n", (int)(28 - (int)strlen(title) / 2), "", title);
    printLine('-', 55);
    printf("  %-20s : %d\n",        "Account No",     accNo);
    printf("  %-20s : %s\n",        "Account Holder",  name);
    printf("  %-20s : %s\n",        "Transaction",     txType);
    printf("  %-20s : Rs. %9.2f\n", "Amount",          amount);
    printf("  %-20s : Rs. %9.2f\n", "Balance After",   balAfter);
    printf("  %-20s : %s\n",        "Date & Time",     date);
    printLine('-', 55);
    printf("        ** Transaction Successful **\n");
    printf("      Thank you for banking with BankSys!\n");
    printLine('=', 55);
}

/* ============================================================
   PIN VERIFICATION  (with account locking + persistent counter)
   ============================================================ */
int verifyPIN(Account *a) {
    /* ---- FEATURE: Check if account is already locked ---- */
    if (a->isLocked) {
        printf("\n");
        printf("  +----------------------------------------------------+\n");
        printf("  |  [LOCKED] Account #%d is LOCKED!                   |\n", a->accNo);
        printf("  |  Too many wrong PIN attempts were made.            |\n");
        printf("  |  Please contact bank administration to unlock.     |\n");
        printf("  +----------------------------------------------------+\n");
        logActivity(a->accNo, "LOGIN BLOCKED - Account Locked");
        return 0;
    }

    int pin;
    int remainingAttempts = PIN_ATTEMPTS - a->failedAttempts;

    while (remainingAttempts > 0) {
        printf("  Enter PIN (%d attempt(s) left) : ", remainingAttempts);
        if (scanf("%d", &pin) != 1) { clearBuffer(); continue; }
        clearBuffer();

        if (pin == a->pin) {
            /* ---- FEATURE: Reset fail counter on success ---- */
            a->failedAttempts = 0;
            return 1;
        }

        a->failedAttempts++;
        remainingAttempts--;

        if (remainingAttempts > 0) {
            printf("  [!] Wrong PIN! %d attempt(s) remaining.\n", remainingAttempts);
        }
    }

    /* ---- FEATURE: Lock account after 3 cumulative failures ---- */
    a->isLocked = 1;
    printf("\n");
    printf("  +----------------------------------------------------+\n");
    printf("  |  [X] ACCOUNT LOCKED!                               |\n");
    printf("  |  3 consecutive wrong PIN attempts detected.        |\n");
    printf("  |  Account #%d has been locked for security.         |\n", a->accNo);
    printf("  |  Contact bank administration to unlock.            |\n");
    printf("  +----------------------------------------------------+\n");
    logActivity(a->accNo, "ACCOUNT LOCKED - 3 Failed PIN Attempts");

    /* TELEGRAM ALERT: Account locked after 3 wrong PINs */
    char date[25];
    getCurrentDate(date);
    char msg[400];
    snprintf(msg, sizeof(msg),
        "🔒 BankSys SECURITY ALERT!\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "ACCOUNT LOCKED!\n"
        "Acc#%d (%s)\n"
        "3 consecutive wrong PIN attempts!\n"
        "Locked at: %s\n"
        "━━━━━━━━━━━━━━━━━━━━━━━━\n"
        "If this was NOT you, your account is at risk!\n"
        "Contact bank IMMEDIATELY!",
        a->accNo, a->name, date);
    sendTelegram(msg);

    return 0;
}

/* ============================================================
   1. CREATE ACCOUNT
   ============================================================ */
void createAccount(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);
    Account a;
    memset(&a, 0, sizeof(a));

    header();
    sectionTitle("CREATE NEW ACCOUNT");

    printf("  Account Number    : ");
    while (scanf("%d", &a.accNo) != 1 || a.accNo <= 0) {
        printf("  [!] Enter a positive number: ");
        clearBuffer();
    }
    clearBuffer();

    if (accExists(list, count, a.accNo)) {
        printf("\n  [X] Account #%d already exists!\n", a.accNo);
        pause(); return;
    }

    printf("  Full Name         : ");
    fgets(a.name, sizeof(a.name), stdin);
    a.name[strcspn(a.name, "\n")] = '\0';
    if (strlen(a.name) == 0) {
        printf("  [X] Name cannot be empty!\n");
        pause(); return;
    }
    /* ---- FEATURE: Validate name (letters and spaces only) ---- */
    {
        int valid = 1, k;
        for (k = 0; a.name[k]; k++) {
            char ch = a.name[k];
            if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') ||
                   ch == ' ' || ch == '.' || ch == '-')) {
                valid = 0; break;
            }
        }
        if (!valid) {
            printf("  [X] Name must contain letters only (A-Z, spaces, '.', '-').\n");
            pause(); return;
        }
    }

    /* ---- FEATURE: Validate phone number (10 digits) ---- */
    printf("  Phone Number (10 digits) : ");
    fgets(a.phone, sizeof(a.phone), stdin);
    a.phone[strcspn(a.phone, "\n")] = '\0';
    {
        int valid = 1, k;
        int len = (int)strlen(a.phone);
        if (len != 10) valid = 0;
        for (k = 0; k < len && valid; k++)
            if (a.phone[k] < '0' || a.phone[k] > '9') valid = 0;
        if (!valid) {
            printf("  [X] Phone must be exactly 10 digits (e.g. 9876543210).\n");
            pause(); return;
        }
    }

    printf("  Account Type:\n");
    printf("   [1] Savings\n");
    printf("   [2] Current\n");
    printf("  Choice            : ");
    int type = 1;
    scanf("%d", &type);
    clearBuffer();
    strcpy(a.accType, (type == 2) ? "Current" : "Savings");

    printf("  Initial Deposit   : Rs. ");
    while (scanf("%f", &a.balance) != 1 || a.balance < 0) {
        printf("  [!] Invalid amount: Rs. ");
        clearBuffer();
    }
    clearBuffer();

    /* ---- FEATURE: Minimum initial deposit for Savings ---- */
    if (strcmp(a.accType, "Savings") == 0 && a.balance < 500.0f) {
        printf("  [X] Savings accounts require a minimum initial deposit of Rs. 500.\n");
        pause(); return;
    }

    printf("  Set 4-digit PIN   : ");
    while (scanf("%d", &a.pin) != 1 || a.pin < 1000 || a.pin > 9999) {
        printf("  [!] PIN must be exactly 4 digits: ");
        clearBuffer();
    }
    clearBuffer();

    a.active  = 1;
    a.txCount = 0;
    a.totalTx = 0;
    getCurrentDate(a.createdAt);
    addTransaction(&a, "Initial Deposit", a.balance, a.balance);

    list[count++] = a;
    writeAll(list, count);
    logActivity(a.accNo, "ACCOUNT CREATED");

    /* TELEGRAM ALERT: New account created */
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: New Account Created!\n"
        "Acc#%d | %s\n"
        "Type: %s | Balance: Rs.%.2f\n"
        "Welcome to BankSys!",
        a.accNo, a.name, a.accType, a.balance);
    sendTelegram(msg);

    printf("\n");
    printLine('=', 65);
    printf("  [OK] Account Created Successfully!\n");
    printLine('-', 65);
    printf("  Account No  : %-10d  Name    : %s\n",       a.accNo,   a.name);
    printf("  Type        : %-10s  Balance : Rs. %.2f\n",  a.accType, a.balance);
    printf("  Phone       : %-10s  PIN Set : ****\n",      a.phone);
    printf("  Created At  : %s\n",                         a.createdAt);
    printLine('=', 65);
    pause();
}

/* ============================================================
   2. ACCOUNT LOGIN
   ============================================================ */
void accountLogin(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);

    header();
    sectionTitle("ACCOUNT LOGIN");

    printf("  Enter Account Number : ");
    int accNo;
    if (scanf("%d", &accNo) != 1) {
        clearBuffer();
        printf("  [X] Invalid input!\n");
        pause(); return;
    }
    clearBuffer();

    int idx = findAcc(list, count, accNo);
    if (idx == -1) {
        printf("\n  [X] Account not found or inactive!\n");
        pause(); return;
    }

    int pinOk = verifyPIN(&list[idx]);
    /* ---- FEATURE: Persist fail count / lock state to disk ---- */
    writeAll(list, count);
    if (!pinOk) { pause(); return; }

    logActivity(accNo, "LOGIN");

    /* TELEGRAM ALERT: Successful login */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: Login Alert!\n"
        "Acc#%d (%s) logged in at %s.\n"
        "Balance: Rs.%.2f\n"
        "Not you? Call bank now!",
        accNo, list[idx].name, date, list[idx].balance);
    sendTelegram(msg);

    accountDashboard(&list[idx], list, count);
}

/* ============================================================
   ACCOUNT DASHBOARD
   ============================================================ */
void accountDashboard(Account *a, Account *list, int count) {
    int choice;
    while (1) {
        header();
        printf("\n  Welcome, %s!\n", a->name);
        printLine('-', 65);
        printf("  Account No  : %d\n",       a->accNo);
        printf("  Type        : %s\n",        a->accType);
        printf("  Balance     : Rs. %.2f\n",  a->balance);
        printf("  Phone       : %s\n",        a->phone);
        printLine('=', 65);
        printf("  [1] Deposit Money\n");
        printf("  [2] Withdraw Money\n");
        printf("  [3] Mini Statement\n");
        printf("  [4] Change PIN\n");
        printf("  [5] Delete This Account\n");
        printf("  [0] Logout\n");
        printLine('=', 65);
        printf("  Choice : ");

        if (scanf("%d", &choice) != 1) { clearBuffer(); continue; }
        clearBuffer();

        switch (choice) {
            case 1: depositMoney(a, list, count);          break;
            case 2: withdrawMoney(a, list, count);         break;
            case 3: miniStatement(a);                      break;
            case 4: changePIN(a, list, count);             break;
            case 5: deleteAccountFromDash(a, list, count); return;
            case 0: logActivity(a->accNo, "LOGOUT");       return;
            default:
                printf("  [!] Invalid choice!\n");
                pause();
        }
    }
}

/* ============================================================
   3. DEPOSIT
   ============================================================ */
void depositMoney(Account *a, Account *list, int count) {
    int i;
    header();
    sectionTitle("DEPOSIT MONEY");

    printf("  Account     : #%d -- %s\n",   a->accNo, a->name);
    printf("  Balance     : Rs. %.2f\n\n",  a->balance);
    printf("  Deposit Amt : Rs. ");

    float amt;
    if (scanf("%f", &amt) != 1 || amt <= 0) {
        clearBuffer();
        printf("\n  [X] Invalid amount!\n");
        pause(); return;
    }
    clearBuffer();

    a->balance += amt;
    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i] = *a; break; }
    addTransaction(a, "Deposit", amt, a->balance);
    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i] = *a; break; }

    writeAll(list, count);
    logActivity(a->accNo, "DEPOSIT");

    /* TELEGRAM ALERT: Deposit */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: Amount Credited!\n"
        "Rs.%.2f deposited to Acc#%d (%s)\n"
        "Date: %s\n"
        "Balance: Rs.%.2f",
        amt, a->accNo, a->name, date, a->balance);
    sendTelegram(msg);

    printReceipt("*** DEPOSIT RECEIPT ***",
                 a->accNo, a->name, "Credit / Deposit", amt, a->balance);
    pause();
}

/* ============================================================
   4. WITHDRAW
   ============================================================ */
void withdrawMoney(Account *a, Account *list, int count) {
    int i;
    header();
    sectionTitle("WITHDRAW MONEY");

    printf("  Account     : #%d -- %s\n",   a->accNo, a->name);
    printf("  Balance     : Rs. %.2f\n\n",  a->balance);
    printf("  Withdraw    : Rs. ");

    float amt;
    if (scanf("%f", &amt) != 1 || amt <= 0) {
        clearBuffer();
        printf("\n  [X] Invalid amount!\n");
        pause(); return;
    }
    clearBuffer();

    if (amt > a->balance) {
        printf("\n  [X] Insufficient balance! Available: Rs. %.2f\n", a->balance);
        pause(); return;
    }

    /* ---- FEATURE: Minimum balance protection for Savings accounts ---- */
    #define MIN_SAVINGS_BALANCE 500.0f
    if (strcmp(a->accType, "Savings") == 0 &&
        (a->balance - amt) < MIN_SAVINGS_BALANCE) {
        printf("\n  [X] Withdrawal denied!\n");
        printf("  Savings accounts must maintain a minimum balance of Rs. %.2f.\n",
               MIN_SAVINGS_BALANCE);
        printf("  Your balance    : Rs. %.2f\n", a->balance);
        printf("  Requested amount: Rs. %.2f\n", amt);
        printf("  Max withdrawable: Rs. %.2f\n",
               a->balance - MIN_SAVINGS_BALANCE > 0
               ? a->balance - MIN_SAVINGS_BALANCE : 0.0f);
        pause(); return;
    }
    #undef MIN_SAVINGS_BALANCE

    a->balance -= amt;
    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i] = *a; break; }
    addTransaction(a, "Withdrawal", amt, a->balance);
    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i] = *a; break; }

    writeAll(list, count);
    logActivity(a->accNo, "WITHDRAWAL");

    /* TELEGRAM ALERT: Withdrawal */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: Amount Debited!\n"
        "Rs.%.2f withdrawn from Acc#%d (%s)\n"
        "Date: %s\n"
        "Balance: Rs.%.2f",
        amt, a->accNo, a->name, date, a->balance);
    sendTelegram(msg);

    printReceipt("*** WITHDRAWAL RECEIPT ***",
                 a->accNo, a->name, "Debit / Withdrawal", amt, a->balance);
    pause();
}

/* ============================================================
   5. FUND TRANSFER
   ============================================================ */
void transferFunds(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);

    header();
    sectionTitle("FUND TRANSFER");

    printf("  Your Account Number   : ");
    int from; scanf("%d", &from); clearBuffer();

    int fromIdx = findAcc(list, count, from);
    if (fromIdx == -1) {
        printf("\n  [X] Source account not found!\n");
        pause(); return;
    }
    int pinOk = verifyPIN(&list[fromIdx]);
    /* Persist fail count / lock state */
    writeAll(list, count);
    if (!pinOk) { pause(); return; }

    printf("  Target Account Number : ");
    int to; scanf("%d", &to); clearBuffer();

    int toIdx = findAcc(list, count, to);
    if (toIdx == -1) {
        printf("\n  [X] Target account not found!\n");
        pause(); return;
    }
    if (from == to) {
        printf("\n  [X] Cannot transfer to the same account!\n");
        pause(); return;
    }

    printf("  Transfer Amount : Rs. ");
    float amt; scanf("%f", &amt); clearBuffer();

    if (amt <= 0 || amt > list[fromIdx].balance) {
        printf("\n  [X] Invalid or insufficient amount! Available: Rs. %.2f\n",
               list[fromIdx].balance);
        pause(); return;
    }

    char desc[35];
    list[fromIdx].balance -= amt;
    snprintf(desc, sizeof(desc), "Transfer -> Acc#%d", to);
    addTransaction(&list[fromIdx], desc, amt, list[fromIdx].balance);

    list[toIdx].balance += amt;
    snprintf(desc, sizeof(desc), "Transfer <- Acc#%d", from);
    addTransaction(&list[toIdx], desc, amt, list[toIdx].balance);

    writeAll(list, count);
    logActivity(from, "TRANSFER OUT");
    logActivity(to,   "TRANSFER IN");

    /* TELEGRAM ALERT: Fund Transfer */
    char date[25];
    getCurrentDate(date);
    char msg[400];
    snprintf(msg, sizeof(msg),
        "BankSys: Money Transfer Successful!\n"
        "Rs.%.2f transferred\n"
        "From: Acc#%d (%s) --> Balance: Rs.%.2f\n"
        "To:   Acc#%d (%s) --> Balance: Rs.%.2f\n"
        "Date: %s",
        amt,
        from, list[fromIdx].name, list[fromIdx].balance,
        to,   list[toIdx].name,   list[toIdx].balance,
        date);
    sendTelegram(msg);

    char date2[25]; getCurrentDate(date2);
    printf("\n");
    printLine('=', 55);
    printf("         *** FUND TRANSFER RECEIPT ***\n");
    printLine('-', 55);
    printf("  %-20s : %s\n",        "Date & Time",    date2);
    printf("  %-20s : Rs. %9.2f\n", "Amount",          amt);
    printLine('-', 55);
    printf("  SENDER\n");
    printf("  %-20s : #%d -- %s\n", "Account", from,  list[fromIdx].name);
    printf("  %-20s : Rs. %9.2f\n", "Balance Now",     list[fromIdx].balance);
    printLine('-', 55);
    printf("  RECEIVER\n");
    printf("  %-20s : #%d -- %s\n", "Account", to,    list[toIdx].name);
    printf("  %-20s : Rs. %9.2f\n", "Balance Now",     list[toIdx].balance);
    printLine('=', 55);
    printf("         ** Transfer Successful **\n");
    printLine('=', 55);
    pause();
}

/* ============================================================
   6. MINI STATEMENT
   ============================================================ */
void miniStatement(Account *a) {
    header();
    sectionTitle("MINI STATEMENT  (Last 5 Transactions)");

    printf("  Account : %s (#%d)   |   Balance : Rs. %.2f\n",
           a->name, a->accNo, a->balance);
    printLine('-', 65);
    printf("  %-24s  %-20s  %9s  %10s\n",
           "Date & Time", "Type", "Amount", "Balance");
    printLine('-', 65);

    int show = (a->totalTx < MAX_TX) ? a->totalTx : MAX_TX;
    if (show == 0) {
        printf("  No transactions recorded yet.\n");
    } else {
        int shown = (show < 5) ? show : 5;
        int i;
        for (i = 0; i < shown; i++) {
            int j = ((a->txCount - 1 - i) % MAX_TX + MAX_TX) % MAX_TX;
            Transaction *t = &a->history[j];
            printf("  %-24s  %-20s  %9.2f  %10.2f\n",
                   t->date, t->type, t->amount, t->balAfter);
        }
    }
    printLine('-', 65);
    pause();
}

/* ============================================================
   7. ADMIN VIEW
   ============================================================ */
void adminView(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);
    int i;

    header();
    sectionTitle("ADMIN VIEW -- ALL ACCOUNTS");

    int   active     = 0;
    float totalFunds = 0;
    int   savings    = 0, current = 0;

    printLine('-', 75);
    printf("  %-6s  %-22s  %-12s  %-10s  %12s\n",
           "AccNo", "Name", "Phone", "Type", "Balance");
    printLine('-', 75);

    for (i = 0; i < count; i++) {
        if (!list[i].active) continue;
        printf("  %-6d  %-22s  %-12s  %-10s  Rs.%9.2f  %s\n",
               list[i].accNo, list[i].name, list[i].phone,
               list[i].accType, list[i].balance,
               list[i].isLocked ? "[LOCKED]" : "");
        active++;
        totalFunds += list[i].balance;
        if (strcmp(list[i].accType, "Savings") == 0) savings++;
        else current++;
    }

    if (active == 0) {
        printf("  No active accounts found.\n");
    } else {
        printLine('=', 75);
        printf("  Total Active Accounts : %d\n",       active);
        printf("  Savings Accounts      : %d\n",       savings);
        printf("  Current Accounts      : %d\n",       current);
        printf("  Total Bank Funds      : Rs. %.2f\n", totalFunds);
    }
    printLine('-', 75);
    pause();
}

/* ============================================================
   8. CHANGE PIN
   ============================================================ */
void changePIN(Account *a, Account *list, int count) {
    int i;
    header();
    sectionTitle("CHANGE PIN");

    int newPin;
    printf("  Enter new 4-digit PIN : ");
    while (scanf("%d", &newPin) != 1 || newPin < 1000 || newPin > 9999) {
        printf("  [!] PIN must be exactly 4 digits: ");
        clearBuffer();
    }
    clearBuffer();

    int confirm;
    printf("  Confirm new PIN       : ");
    scanf("%d", &confirm); clearBuffer();

    if (newPin != confirm) {
        printf("\n  [X] PINs do not match! PIN not changed.\n");
        pause(); return;
    }

    a->pin = newPin;
    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i] = *a; break; }

    writeAll(list, count);
    logActivity(a->accNo, "PIN CHANGED");

    /* TELEGRAM ALERT: PIN changed */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: PIN Changed!\n"
        "Acc#%d (%s) PIN was changed on %s.\n"
        "Not you? Contact bank immediately!",
        a->accNo, a->name, date);
    sendTelegram(msg);

    printf("\n  [OK] PIN changed successfully!\n");
    pause();
}

/* ============================================================
   9. DELETE ACCOUNT
   ============================================================ */
void deleteAccountFromDash(Account *a, Account *list, int count) {
    int i;
    header();
    sectionTitle("DELETE ACCOUNT");

    printf("\n  [WARNING] This will permanently delete:\n");
    printf("  Account #%d -- %s\n",    a->accNo, a->name);
    printf("  Balance : Rs. %.2f\n\n", a->balance);
    printf("  Type  DELETE  to confirm : ");

    char confirm[20];
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strcmp(confirm, "DELETE") != 0) {
        printf("\n  Deletion cancelled.\n");
        pause(); return;
    }

    for (i = 0; i < count; i++)
        if (list[i].accNo == a->accNo) { list[i].active = 0; break; }

    writeAll(list, count);
    logActivity(a->accNo, "ACCOUNT DELETED");

    /* TELEGRAM ALERT: Account deleted */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "BankSys: Account Deleted!\n"
        "Acc#%d (%s) was deleted on %s.\n"
        "If not you, contact bank now!",
        a->accNo, a->name, date);
    sendTelegram(msg);

    printf("\n  [OK] Account #%d (%s) deleted successfully.\n",
           a->accNo, a->name);
    pause();
}

/* ============================================================
   UNLOCK ACCOUNT  (Admin function)
   ============================================================ */
void unlockAccount(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);
    int i;

    header();
    sectionTitle("UNLOCK ACCOUNT  (Admin)");

    /* Show all locked accounts */
    int found = 0;
    printf("\n  Locked Accounts:\n");
    printLine('-', 45);
    for (i = 0; i < count; i++) {
        if (list[i].active && list[i].isLocked) {
            printf("  Acc#%-6d  %s\n", list[i].accNo, list[i].name);
            found++;
        }
    }
    if (!found) {
        printf("  No locked accounts found.\n");
        printLine('-', 45);
        pause(); return;
    }
    printLine('-', 45);

    printf("\n  Enter Account Number to Unlock : ");
    int accNo;
    if (scanf("%d", &accNo) != 1) { clearBuffer(); pause(); return; }
    clearBuffer();

    int idx = -1;
    for (i = 0; i < count; i++)
        if (list[i].accNo == accNo && list[i].active) { idx = i; break; }

    if (idx == -1) {
        printf("\n  [X] Account not found!\n");
        pause(); return;
    }
    if (!list[idx].isLocked) {
        printf("\n  [!] Account #%d is not locked.\n", accNo);
        pause(); return;
    }

    list[idx].isLocked       = 0;
    list[idx].failedAttempts = 0;
    writeAll(list, count);
    logActivity(accNo, "ACCOUNT UNLOCKED by Admin");

    /* TELEGRAM ALERT: Account unlocked */
    char date[25];
    getCurrentDate(date);
    char msg[300];
    snprintf(msg, sizeof(msg),
        "✅ BankSys: Account Unlocked!\n"
        "Acc#%d (%s) was unlocked by admin on %s.\n"
        "You may now log in normally.",
        list[idx].accNo, list[idx].name, date);
    sendTelegram(msg);

    printf("\n  [OK] Account #%d (%s) unlocked successfully!\n",
           list[idx].accNo, list[idx].name);
    pause();
}

/* ============================================================
   10. EXPORT TO CSV + MARKDOWN TABLE
   ============================================================ */
void exportCSV(void) {
    Account list[MAX_ACCOUNTS];
    int count = readAll(list);
    int i;

    header();
    sectionTitle("EXPORT  (CSV + Markdown Table for VS Code)");

    int   exported   = 0;
    float grandTotal = 0;
    for (i = 0; i < count; i++) {
        if (!list[i].active) continue;
        exported++;
        grandTotal += list[i].balance;
    }

    FILE *fc = fopen(CSV_FILE, "w");
    if (!fc) {
        printf("\n  [X] Cannot create CSV file!\n");
        pause(); return;
    }
    fprintf(fc, "Account No,Name,Phone,Account Type,Balance,Status,Created At,Total Transactions\n");
    for (i = 0; i < count; i++) {
        if (!list[i].active) continue;
        fprintf(fc, "%d,\"%s\",%s,%s,%.2f,Active,\"%s\",%d\n",
                list[i].accNo, list[i].name, list[i].phone,
                list[i].accType, list[i].balance,
                list[i].createdAt, list[i].totalTx);
    }
    fclose(fc);

    char exportDate[25];
    getCurrentDate(exportDate);

    FILE *fm = fopen(MD_FILE, "w");
    if (!fm) {
        printf("\n  [X] Cannot create Markdown file!\n");
        pause(); return;
    }
    fprintf(fm, "# BankSys - Account Export\n\n");
    fprintf(fm, "> **Exported on:** %s  \n", exportDate);
    fprintf(fm, "> **Total active accounts:** %d  \n", exported);
    fprintf(fm, "> **Total funds:** Rs. %.2f\n\n", grandTotal);
    fprintf(fm, "---\n\n");
    fprintf(fm, "| Account No | Name | Phone | Account Type | Balance (Rs.) | Status | Created At | Total Tx |\n");
    fprintf(fm, "|:----------:|:-----|:-----:|:------------:|-------------:|:------:|:----------:|:--------:|\n");
    for (i = 0; i < count; i++) {
        if (!list[i].active) continue;
        fprintf(fm, "| %d | %s | %s | %s | %.2f | Active | %s | %d |\n",
                list[i].accNo, list[i].name, list[i].phone,
                list[i].accType, list[i].balance,
                list[i].createdAt, list[i].totalTx);
    }
    fprintf(fm, "\n---\n\n");
    fprintf(fm, "| | **TOTAL** | | | **%.2f** | | | **%d** |\n", grandTotal, exported);
    fprintf(fm, "\n---\n\n");
    fprintf(fm, "_Generated by Advanced Bank Management System v3.0 - C Language Project_\n");
    fclose(fm);

    logActivity(0, "CSV + MARKDOWN EXPORTED");

    #define HLINE() \
        printf("  +--------+------------------------+---------------+-----------+----------------+---------+---------------------+------+\n")

    printf("\n");
    HLINE();
    printf("  | %-6s | %-22s | %-13s | %-9s | %-14s | %-7s | %-19s | %-4s |\n",
           "AccNo", "Name", "Phone", "Type", "Balance (Rs.)", "Status", "Created At", "Txns");
    HLINE();

    if (exported == 0) {
        printf("  |  No active accounts found.                                                                                        |\n");
    } else {
        for (i = 0; i < count; i++) {
            if (!list[i].active) continue;
            char balStr[16];
            snprintf(balStr, sizeof(balStr), "%.2f", list[i].balance);
            printf("  | %-6d | %-22s | %-13s | %-9s | %14s | %-7s | %-19s | %4d |\n",
                   list[i].accNo,   list[i].name,    list[i].phone,
                   list[i].accType, balStr,           "Active",
                   list[i].createdAt, list[i].totalTx);
        }
        HLINE();
        char totalStr[20];
        snprintf(totalStr, sizeof(totalStr), "%.2f", grandTotal);
        printf("  | %-6s | %-22s | %-13s | %-9s | %14s | %-7s | %-19s | %4d |\n",
               "TOTAL", "", "", "", totalStr, "", "", exported);
    }
    HLINE();
    #undef HLINE

    printf("\n");
    printLine('=', 65);
    printf("  [OK] %d account(s) exported successfully!\n\n", exported);
    printf("  %-30s %s\n", "CSV  (Excel / Sheets)  :", CSV_FILE);
    printf("  %-30s %s\n", "Markdown (VS Code)     :", MD_FILE);
    printLine('=', 65);
    pause();
}

/* ============================================================
   11. ACTIVITY LOG VIEWER
   ============================================================ */
void viewActivityLog(void) {
    header();
    sectionTitle("ACTIVITY LOG");

    FILE *fp = fopen(LOG_FILE, "r");
    if (!fp) {
        printf("\n  No activity log found.\n");
        printf("  (Perform some operations first.)\n");
        pause(); return;
    }

    char line[120];
    int  lines = 0;
    char buf[30][120];

    while (fgets(line, sizeof(line), fp)) {
        strncpy(buf[lines % 30], line, 119);
        buf[lines % 30][119] = '\0';
        lines++;
    }
    fclose(fp);

    int limit = (lines < 30) ? lines : 30;
    printf("  Showing last %d log entries (Total: %d)\n\n", limit, lines);
    printLine('-', 65);

    int start = (lines > 30) ? lines % 30 : 0;
    int i;
    for (i = 0; i < limit; i++)
        printf("  %s", buf[(start + i) % 30]);

    printLine('-', 65);
    pause();
}

/* ============================================================
   MAIN MENU
   ============================================================ */
int main(void) {

    /* Startup diagnostic */
    printf("  [STARTUP] Starting Bank Management System...\n");
    fflush(stdout);

#ifdef _WIN32
    printf("  [STARTUP] Windows detected.\n");
    printf("  [STARTUP] Ensure libcurl-x64.dll is in C:\\C_project\\\n");
    fflush(stdout);
#endif

    curl_global_init(CURL_GLOBAL_DEFAULT);
    printf("  [STARTUP] Curl initialized successfully.\n");
    printf("  [STARTUP] Loading menu...\n\n");
    fflush(stdout);

    int choice, i;

    while (1) {
        header();

        Account list[MAX_ACCOUNTS];
        int   count  = readAll(list);
        int   active = 0;
        float total  = 0;
        for (i = 0; i < count; i++)
            if (list[i].active) { active++; total += list[i].balance; }

        printf("\n  Active Accounts : %-4d   Total Funds : Rs. %.2f\n",
               active, total);
        printLine('=', 65);
        printf("                   MAIN  MENU\n");
        printLine('=', 65);
        printf("   [1]  Create New Account\n");
        printf("   [2]  Account Login  (Deposit / Withdraw / Statement)\n");
        printf("   [3]  Fund Transfer  (Account to Account)\n");
        printf("   [4]  Admin View     (All Accounts Summary)\n");
        printf("   [5]  Export to CSV  (Excel-Readable)\n");
        printf("   [6]  Activity Log   (View Recent Activity)\n");
        printf("   [7]  Unlock Account (Admin - After Lockout)\n");
        printf("   [0]  Exit\n");
        printLine('=', 65);
        printf("  Enter your choice : ");

        if (scanf("%d", &choice) != 1) {
            clearBuffer();
            printf("  [!] Invalid input! Enter a number 0-7.\n");
            pause();
            continue;
        }
        clearBuffer();

        switch (choice) {
            case 1: createAccount();   break;
            case 2: accountLogin();    break;
            case 3: transferFunds();   break;
            case 4: adminView();       break;
            case 5: exportCSV();       break;
            case 6: viewActivityLog(); break;
            case 7: unlockAccount();   break;
            case 0:
                clearScreen();
                printLine('=', 65);
                printf("  Thank you for using Advanced Bank System!\n");
                printf("  Data saved to : %s\n", DATA_FILE);
                printf("  Goodbye!\n");
                printLine('=', 65);
                printf("\n");
                curl_global_cleanup();
#ifdef _WIN32
                system("pause");
#endif
                exit(0);
            default:
                printf("\n  [!] Invalid choice! Enter 0 to 7.\n");
                pause();
        }
    }

    curl_global_cleanup();
    return 0;
}