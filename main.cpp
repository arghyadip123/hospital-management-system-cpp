#include "appointment.h"
#include "billing.h"
#include "doctor.h"
#include "patient.h"
#include "storage_utils.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <queue>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <windows.h>
#endif

using namespace std;

namespace Console {

enum class Color {
    Default = 7,
    Heading = 11,
    Success = 10,
    Warning = 14,
    Error = 12,
    Accent = 13
};

void setColor(Color color) {
#ifdef _WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
#else
    switch (color) {
    case Color::Heading:
        cout << "\033[36m";
        break;
    case Color::Success:
        cout << "\033[32m";
        break;
    case Color::Warning:
        cout << "\033[33m";
        break;
    case Color::Error:
        cout << "\033[31m";
        break;
    case Color::Accent:
        cout << "\033[35m";
        break;
    default:
        cout << "\033[0m";
        break;
    }
#endif
}

void resetColor() {
    setColor(Color::Default);
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void line(int width = 92, char fill = '=') {
    cout << string(width, fill) << '\n';
}

void centered(const string &text, int width = 92) {
    const int padding = max(0, (width - static_cast<int>(text.length())) / 2);
    cout << string(padding, ' ') << text << '\n';
}

void header(const string &title) {
    setColor(Color::Heading);
    line();
    centered(title);
    line();
    resetColor();
}

void hospitalHeader() {
    setColor(Color::Heading);

    cout << "==================================================\n";
    cout << "              CITY CARE HOSPITAL\n";
    cout << "             Hospital Management System\n";
    cout << "==================================================\n";

    resetColor();
}

void success(const string &message) {
    setColor(Color::Success);
    cout << "Success: " << message << '\n';
    resetColor();
}

void warning(const string &message) {
    setColor(Color::Warning);
    cout << "Warning: " << message << '\n';
    resetColor();
}

void error(const string &message) {
    setColor(Color::Error);
    cout << "Error: " << message << '\n';
    resetColor();
}

void info(const string &message) {
    setColor(Color::Accent);
    cout << message << '\n';
    resetColor();
}

void pause() {
    cout << "\nPress Enter to continue...";
    string ignored;
    getline(cin, ignored);
}

} // namespace Console

namespace Input {

string readLine(const string &prompt) {
    string value;
    cout << prompt;
    getline(cin, value);
    return value;
}

string readPassword(const string &prompt) {
    cout << prompt;

#ifdef _WIN32
    string password;
    while (true) {
        const int ch = _getch();

        if (ch == '\r' || ch == '\n') {
            cout << '\n';
            break;
        }

        if (ch == '\b') {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
            }
            continue;
        }

        if (ch == 0 || ch == 224) {
            _getch();
            continue;
        }

        if (isprint(static_cast<unsigned char>(ch))) {
            password += static_cast<char>(ch);
            cout << '*';
        }
    }

    return password;
#else
    return readLine("");
#endif
}

string readRequiredLine(const string &prompt) {
    while (true) {
        string value = Storage::trim(readLine(prompt));
        if (!value.empty()) {
            return value;
        }
        Console::error("This field cannot be empty.");
    }
}

string readOptionalLine(const string &label, const string &currentValue) {
    const string value = Storage::trim(readLine(label + " [" + currentValue + "]: "));
    return value.empty() ? currentValue : value;
}

int readInt(const string &prompt, int minValue, int maxValue) {
    while (true) {
        int value = 0;
        const string text = readLine(prompt);

        if (Storage::parseInt(text, value) && value >= minValue && value <= maxValue) {
            return value;
        }

        Console::error("Enter a valid number in the allowed range.");
    }
}

int readOptionalInt(const string &label, int currentValue, int minValue, int maxValue) {
    while (true) {
        int value = 0;
        const string text = Storage::trim(readLine(label + " [" + to_string(currentValue) + "]: "));

        if (text.empty()) {
            return currentValue;
        }

        if (Storage::parseInt(text, value) && value >= minValue && value <= maxValue) {
            return value;
        }

        Console::error("Enter a valid number in the allowed range.");
    }
}

double readDouble(const string &prompt, double minValue) {
    while (true) {
        double value = 0.0;
        const string text = readLine(prompt);

        if (Storage::parseDouble(text, value) && value >= minValue) {
            return value;
        }

        Console::error("Enter a valid amount.");
    }
}

bool readYesNo(const string &prompt) {
    while (true) {
        const string answer = Storage::toLower(Storage::trim(readLine(prompt + " (y/n): ")));

        if (answer == "y" || answer == "yes") {
            return true;
        }
        if (answer == "n" || answer == "no") {
            return false;
        }

        Console::error("Please enter y or n.");
    }
}

} // namespace Input

string clip(const string &value, size_t width) {
    if (value.length() <= width) {
        return value;
    }
    if (width <= 3) {
        return value.substr(0, width);
    }
    return value.substr(0, width - 3) + "...";
}

bool fileExists(const string &fileName) {
    ifstream input(fileName);
    return input.good();
}

// Writes to a temporary file first, then replaces the original file.
// This keeps patient, doctor, appointment, and billing files safer during updates.
bool writeLinesSafely(const string &fileName, const vector<string> &lines) {
    const string tempFile = fileName + ".tmp";
    const string backupFile = fileName + ".bak";

    {
        ofstream output(tempFile);
        if (!output) {
            Console::error("Unable to open " + tempFile + " for writing.");
            return false;
        }

        for (const string &line : lines) {
            output << line << '\n';
        }

        output.close();
        if (!output) {
            Console::error("Unable to finish writing " + tempFile + ".");
            remove(tempFile.c_str());
            return false;
        }
    }

    const bool hadOriginal = fileExists(fileName);
    if (hadOriginal) {
        remove(backupFile.c_str());
        if (rename(fileName.c_str(), backupFile.c_str()) != 0) {
            Console::error("Unable to create a backup for " + fileName + ".");
            remove(tempFile.c_str());
            return false;
        }
    }

    if (rename(tempFile.c_str(), fileName.c_str()) != 0) {
        Console::error("Unable to replace " + fileName + ".");
        remove(tempFile.c_str());
        if (hadOriginal) {
            rename(backupFile.c_str(), fileName.c_str());
        }
        return false;
    }

    if (hadOriginal) {
        remove(backupFile.c_str());
    }

    return true;
}

namespace Security {

uint64_t fnv1a64(const string &text) {
    uint64_t hash = 14695981039346656037ULL;
    for (unsigned char ch : text) {
        hash ^= ch;
        hash *= 1099511628211ULL;
    }
    return hash;
}

string toHex(uint64_t value) {
    ostringstream stream;
    stream << hex << setw(16) << setfill('0') << value;
    return stream.str();
}

string createSalt() {
    static const char hexDigits[] = "0123456789abcdef";
    random_device randomDevice;
    mt19937 generator(randomDevice());
    uniform_int_distribution<int> distribution(0, 15);

    string salt;
    for (int index = 0; index < 32; ++index) {
        salt += hexDigits[distribution(generator)];
    }
    return salt;
}

// Demonstration-friendly salted password digest. It avoids storing plain text.
// For production software, use a dedicated password hasher such as bcrypt or Argon2.
string passwordDigest(const string &password, const string &salt) {
    string digest = salt + ":" + password;
    for (int round = 0; round < 12000; ++round) {
        digest = toHex(fnv1a64(digest + ":" + salt + ":" + to_string(round)));
    }
    return digest;
}

} // namespace Security

class AdminAuth {
private:
    struct Credentials {
        string username;
        string salt;
        string digest;
    };

    const string credentialsFile = "admin.dat";

    bool loadCredentials(Credentials &credentials) const {
        ifstream input(credentialsFile);
        if (!input) {
            return false;
        }

        string line;
        getline(input, line);
        const vector<string> fields = Storage::splitRecord(line);

        if (fields.size() < 3 ||
            Storage::trim(fields[0]).empty() ||
            Storage::trim(fields[1]).empty() ||
            Storage::trim(fields[2]).empty()) {
            return false;
        }

        credentials.username = fields[0];
        credentials.salt = fields[1];
        credentials.digest = fields[2];
        return true;
    }

    bool saveCredentials(const Credentials &credentials) const {
        return writeLinesSafely(credentialsFile,
                                {Storage::joinFields({credentials.username,
                                                      credentials.salt,
                                                      credentials.digest})});
    }

    bool createFirstAdmin() const {
        Console::clearScreen();
        Console::header("Admin Account Setup");
        Console::warning("No admin account was found. Create one before using the system.");

        while (true) {
            Credentials credentials;
            credentials.username = Input::readRequiredLine("New admin username: ");

            const string password = Input::readPassword("New admin password: ");
            if (password.length() < 4) {
                Console::error("Password must contain at least 4 characters.");
                continue;
            }

            const string confirmPassword = Input::readPassword("Confirm password: ");
            if (password != confirmPassword) {
                Console::error("Passwords do not match.");
                continue;
            }

            credentials.salt = Security::createSalt();
            credentials.digest = Security::passwordDigest(password, credentials.salt);

            if (!saveCredentials(credentials)) {
                return false;
            }

            Console::success("Admin account created.");
            Console::pause();
            return true;
        }
    }

public:
    bool authenticate() const {
        Credentials credentials;

        if (!loadCredentials(credentials)) {
            if (!createFirstAdmin()) {
                return false;
            }
            if (!loadCredentials(credentials)) {
                Console::error("Unable to load admin credentials.");
                return false;
            }
        }

        for (int attempt = 1; attempt <= 3; ++attempt) {
            Console::clearScreen();
            Console::header("Admin Login");
            cout << "Attempt " << attempt << " of 3\n\n";

            const string username = Input::readRequiredLine("Username: ");
            const string password = Input::readPassword("Password: ");
            const string enteredDigest = Security::passwordDigest(password, credentials.salt);

            if (username == credentials.username && enteredDigest == credentials.digest) {
                Console::success("Login successful.");
                Console::pause();
                return true;
            }

            Console::error("Invalid username or password.");
            if (attempt < 3) {
                Console::pause();
            }
        }

        Console::error("Access denied.");
        return false;
    }
};

class HospitalManagementSystem {
private:
    // Data structures used by the project:
    // list  : Patient records, useful for insert/delete operations.
    // vector: Doctors, appointments, and bills, useful for indexing and table display.
    // queue : Scheduled appointment IDs, useful for viewing appointment order.
    list<Patient> patients;
    vector<Doctor> doctors;
    vector<Appointment> appointments;
    vector<Bill> bills;
    queue<string> appointmentQueue;
    vector<string> startupWarnings;

    const string patientFile = "patients.txt";
    const string doctorFile = "doctors.txt";
    const string appointmentFile = "appointments.txt";
    const string billFile = "bills.txt";

public:
    void run() {
        loadData();
        showStartupWarnings();

        while (true) {
            Console::clearScreen();
            Console::hospitalHeader();
            printDashboard();
            printMainMenu();

            const int choice = Input::readInt("Choose an option: ", 0, 4);
            switch (choice) {
            case 1:
                patientMenu();
                break;
            case 2:
                doctorMenu();
                break;
            case 3:
                appointmentMenu();
                break;
            case 4:
                billingMenu();
                break;
            case 0:
                saveAll();
                Console::success("All records saved. Goodbye!");
                return;
            }
        }
    }

private:
    void printDashboard() const {
        const int scheduledAppointments = count_if(appointments.begin(), appointments.end(),
                                                   [](const Appointment &item) { return item.isScheduled(); });
        const int unpaidBills = count_if(bills.begin(), bills.end(),
                                         [](const Bill &bill) { return !bill.isPaid(); });
        double unpaidAmount = 0.0;
        for (const Bill &bill : bills) {
            if (!bill.isPaid()) {
                unpaidAmount += bill.getTotalAmount();
            }
        }

        Console::setColor(Console::Color::Accent);
        cout << "\n+----------------------+----------------------+----------------------+----------------------+\n";
        cout << "| " << left << setw(20) << "Patients"
             << "| " << setw(20) << "Doctors"
             << "| " << setw(20) << "Scheduled Appts"
             << "| " << setw(20) << "Unpaid Bills" << "|\n";
        cout << "+----------------------+----------------------+----------------------+----------------------+\n";
        Console::resetColor();
        cout << "| " << left << setw(20) << patients.size()
             << "| " << setw(20) << doctors.size()
             << "| " << setw(20) << scheduledAppointments
             << "| " << setw(20) << unpaidBills << "|\n";
        cout << "+----------------------+----------------------+----------------------+----------------------+\n";
        cout << " Current Date/Time : " << Storage::currentDateTime() << '\n';
        cout << " Outstanding Total : " << fixed << setprecision(2) << unpaidAmount << "\n\n";
    }

    void printMainMenu() const {
        Console::line(92, '-');
        cout << "1. Patient Management\n";
        cout << "2. Doctor Management\n";
        cout << "3. Appointment Management\n";
        cout << "4. Billing Management\n";
        cout << "0. Save and Exit\n";
        Console::line(92, '-');
    }

    void showStartupWarnings() const {
        if (startupWarnings.empty()) {
            return;
        }

        Console::clearScreen();
        Console::header("File Loading Warnings");
        for (const string &warning : startupWarnings) {
            Console::warning(warning);
        }
        Console::pause();
    }

    void addWarning(const string &fileName, int lineNumber, const string &message) {
        startupWarnings.push_back(fileName + " line " + to_string(lineNumber) + ": " + message);
    }

    void loadData() {
        startupWarnings.clear();
        loadPatients();
        loadDoctors();
        loadAppointments();
        loadBills();
        rebuildAppointmentQueue();
    }

    void saveAll() const {
        savePatients();
        saveDoctors();
        saveAppointments();
        saveBills();
    }

    void loadPatients() {
        patients.clear();
        ifstream input(patientFile);
        if (!input) {
            return;
        }

        string line;
        int lineNumber = 0;
        while (getline(input, line)) {
            ++lineNumber;
            if (Storage::trim(line).empty()) {
                continue;
            }

            bool ok = false;
            Patient patient = Patient::fromRecord(line, ok);
            if (!ok) {
                addWarning(patientFile, lineNumber, "Invalid patient record skipped.");
                continue;
            }

            if (patientIdExists(patient.getPatientId())) {
                addWarning(patientFile, lineNumber, "Duplicate patient ID skipped.");
                continue;
            }

            patients.push_back(patient);
        }
    }

    void loadDoctors() {
        doctors.clear();
        ifstream input(doctorFile);
        if (!input) {
            return;
        }

        string line;
        int lineNumber = 0;
        while (getline(input, line)) {
            ++lineNumber;
            if (Storage::trim(line).empty()) {
                continue;
            }

            bool ok = false;
            Doctor doctor = Doctor::fromRecord(line, ok);
            if (!ok) {
                addWarning(doctorFile, lineNumber, "Invalid doctor record skipped.");
                continue;
            }

            if (doctorIdExists(doctor.getDoctorId())) {
                addWarning(doctorFile, lineNumber, "Duplicate doctor ID skipped.");
                continue;
            }

            doctors.push_back(doctor);
        }
    }

    void loadAppointments() {
        appointments.clear();
        ifstream input(appointmentFile);
        if (!input) {
            return;
        }

        string line;
        int lineNumber = 0;
        while (getline(input, line)) {
            ++lineNumber;
            if (Storage::trim(line).empty()) {
                continue;
            }

            bool ok = false;
            Appointment appointment = Appointment::fromRecord(line, ok);
            if (!ok) {
                addWarning(appointmentFile, lineNumber, "Invalid appointment record skipped.");
                continue;
            }

            if (appointmentIdExists(appointment.getAppointmentId())) {
                addWarning(appointmentFile, lineNumber, "Duplicate appointment ID skipped.");
                continue;
            }

            appointments.push_back(appointment);
        }
    }

    void loadBills() {
        bills.clear();
        ifstream input(billFile);
        if (!input) {
            return;
        }

        string line;
        int lineNumber = 0;
        while (getline(input, line)) {
            ++lineNumber;
            if (Storage::trim(line).empty()) {
                continue;
            }

            bool ok = false;
            Bill bill = Bill::fromRecord(line, ok);
            if (!ok) {
                addWarning(billFile, lineNumber, "Invalid bill record skipped.");
                continue;
            }

            if (billIdExists(bill.getBillId())) {
                addWarning(billFile, lineNumber, "Duplicate bill ID skipped.");
                continue;
            }

            bills.push_back(bill);
        }
    }

    bool savePatients() const {
        vector<string> lines;
        for (const Patient &patient : patients) {
            lines.push_back(patient.serialize());
        }
        return writeLinesSafely(patientFile, lines);
    }

    bool saveDoctors() const {
        vector<string> lines;
        for (const Doctor &doctor : doctors) {
            lines.push_back(doctor.serialize());
        }
        return writeLinesSafely(doctorFile, lines);
    }

    bool saveAppointments() const {
        vector<string> lines;
        for (const Appointment &appointment : appointments) {
            lines.push_back(appointment.serialize());
        }
        return writeLinesSafely(appointmentFile, lines);
    }

    bool saveBills() const {
        vector<string> lines;
        for (const Bill &bill : bills) {
            lines.push_back(bill.serialize());
        }
        return writeLinesSafely(billFile, lines);
    }

    bool patientIdExists(const string &id) const {
        return findPatient(id) != nullptr;
    }

    bool doctorIdExists(const string &id) const {
        return findDoctor(id) != nullptr;
    }

    bool appointmentIdExists(const string &id) const {
        return findAppointment(id) != nullptr;
    }

    bool billIdExists(const string &id) const {
        return findBill(id) != nullptr;
    }

    Patient *findPatient(const string &id) {
        const string normalizedId = Storage::normalizePatientId(id);
        for (Patient &patient : patients) {
            if (patient.getPatientId() == normalizedId) {
                return &patient;
            }
        }
        return nullptr;
    }

    const Patient *findPatient(const string &id) const {
        const string normalizedId = Storage::normalizePatientId(id);
        for (const Patient &patient : patients) {
            if (patient.getPatientId() == normalizedId) {
                return &patient;
            }
        }
        return nullptr;
    }

    Doctor *findDoctor(const string &id) {
        const string normalizedId = Storage::normalizeDoctorId(id);
        for (Doctor &doctor : doctors) {
            if (doctor.getDoctorId() == normalizedId) {
                return &doctor;
            }
        }
        return nullptr;
    }

    const Doctor *findDoctor(const string &id) const {
        const string normalizedId = Storage::normalizeDoctorId(id);
        for (const Doctor &doctor : doctors) {
            if (doctor.getDoctorId() == normalizedId) {
                return &doctor;
            }
        }
        return nullptr;
    }

    Appointment *findAppointment(const string &id) {
        const string normalizedId = Storage::normalizeAppointmentId(id);
        for (Appointment &appointment : appointments) {
            if (appointment.getAppointmentId() == normalizedId) {
                return &appointment;
            }
        }
        return nullptr;
    }

    const Appointment *findAppointment(const string &id) const {
        const string normalizedId = Storage::normalizeAppointmentId(id);
        for (const Appointment &appointment : appointments) {
            if (appointment.getAppointmentId() == normalizedId) {
                return &appointment;
            }
        }
        return nullptr;
    }

    Bill *findBill(const string &id) {
        const string normalizedId = Storage::normalizeBillId(id);
        for (Bill &bill : bills) {
            if (bill.getBillId() == normalizedId) {
                return &bill;
            }
        }
        return nullptr;
    }

    const Bill *findBill(const string &id) const {
        const string normalizedId = Storage::normalizeBillId(id);
        for (const Bill &bill : bills) {
            if (bill.getBillId() == normalizedId) {
                return &bill;
            }
        }
        return nullptr;
    }

    string nextPatientId() const {
        int maxId = 1000;
        for (const Patient &patient : patients) {
            maxId = max(maxId, Storage::patientIdNumber(patient.getPatientId()));
        }
        return Storage::formatPatientId(maxId + 1);
    }

    string nextDoctorId() const {
        int maxId = 0;
        for (const Doctor &doctor : doctors) {
            maxId = max(maxId, Storage::doctorIdNumber(doctor.getDoctorId()));
        }
        return Storage::formatDoctorId(maxId + 1);
    }

    string nextAppointmentId() const {
        int maxId = 0;
        for (const Appointment &appointment : appointments) {
            maxId = max(maxId, Storage::appointmentIdNumber(appointment.getAppointmentId()));
        }
        return Storage::formatAppointmentId(maxId + 1);
    }

    string nextBillId() const {
        int maxId = 0;
        for (const Bill &bill : bills) {
            maxId = max(maxId, Storage::billIdNumber(bill.getBillId()));
        }
        return Storage::formatBillId(maxId + 1);
    }

    string patientName(const string &id) const {
        const Patient *patient = findPatient(id);
        return patient ? patient->getName() : "Unknown Patient";
    }

    string readPatientId(const string &prompt) const {
        while (true) {
            const string normalizedId = Storage::normalizePatientId(Input::readRequiredLine(prompt));
            if (!normalizedId.empty()) {
                return normalizedId;
            }

            Console::error("Enter a valid patient ID such as HSP2026001.");
        }
    }

    string doctorName(const string &id) const {
        const Doctor *doctor = findDoctor(id);
        return doctor ? doctor->getName() : "Unknown Doctor";
    }

    string readDoctorId(const string &prompt) const {
        while (true) {
            const string normalizedId = Storage::normalizeDoctorId(Input::readRequiredLine(prompt));
            if (!normalizedId.empty()) {
                return normalizedId;
            }

            Console::error("Enter a valid doctor ID such as DOC2026001.");
        }
    }

    string readAppointmentId(const string &prompt) const {
        while (true) {
            const string normalizedId = Storage::normalizeAppointmentId(Input::readRequiredLine(prompt));
            if (!normalizedId.empty()) {
                return normalizedId;
            }

            Console::error("Enter a valid appointment ID such as APT2026001.");
        }
    }

    string readBillId(const string &prompt) const {
        while (true) {
            const string normalizedId = Storage::normalizeBillId(Input::readRequiredLine(prompt));
            if (!normalizedId.empty()) {
                return normalizedId;
            }

            Console::error("Enter a valid bill ID such as BIL2026001.");
        }
    }

    void rebuildAppointmentQueue() {
        queue<string> emptyQueue;
        swap(appointmentQueue, emptyQueue);

        for (const Appointment &appointment : appointments) {
            if (appointment.isScheduled()) {
                appointmentQueue.push(appointment.getAppointmentId());
            }
        }
    }

    void patientMenu() {
        while (true) {
            Console::clearScreen();
            Console::header("Patient Management");
            cout << "1. Add Patient\n";
            cout << "2. View All Patients\n";
            cout << "3. Search Patient By ID\n";
            cout << "4. Search Patient By Name\n";
            cout << "5. Update Patient Details\n";
            cout << "6. Delete Patient Record\n";
            cout << "0. Back\n";
            Console::line(92, '-');

            const int choice = Input::readInt("Choose an option: ", 0, 6);
            if (choice == 0) {
                return;
            }

            Console::clearScreen();
            switch (choice) {
            case 1:
                addPatient();
                break;
            case 2:
                listPatients();
                break;
            case 3:
                searchPatientById();
                break;
            case 4:
                searchPatientByName();
                break;
            case 5:
                updatePatient();
                break;
            case 6:
                deletePatient();
                break;
            }
            Console::pause();
        }
    }

    void printPatientHeader() const {
        Console::line(104, '-');
        cout << left << setw(12) << "ID"
             << setw(24) << "Name"
             << setw(6) << "Age"
             << setw(12) << "Gender"
             << setw(16) << "Phone"
             << setw(10) << "Blood"
             << setw(24) << "Disease" << '\n';
        Console::line(104, '-');
    }

    void addPatient() {
        Console::header("Add Patient");
        string id = nextPatientId();
        while (patientIdExists(id)) {
            id = Storage::formatPatientId(Storage::patientIdNumber(id) + 1);
        }

        Patient patient(id,
                        Input::readRequiredLine("Name: "),
                        Input::readInt("Age: ", 0, 130),
                        Input::readRequiredLine("Gender: "),
                        Input::readRequiredLine("Phone: "),
                        Input::readRequiredLine("Blood Group: "),
                        Input::readRequiredLine("Disease / Diagnosis: "),
                        Input::readRequiredLine("Address: "));

        patients.push_back(patient);
        if (savePatients()) {
            Console::success("Patient added with ID " + patient.getPatientId() + ".");
        }
    }

    void listPatients() const {
        Console::header("Patient List");
        if (patients.empty()) {
            Console::warning("No patient records found.");
            return;
        }

        printPatientHeader();
        for (const Patient &patient : patients) {
            patient.printRow();
        }
    }

    void searchPatientById() const {
        Console::header("Search Patient By ID");
        const string id = readPatientId("Enter patient ID (for example HSP2026001): ");
        const Patient *patient = findPatient(id);

        if (!patient) {
            Console::error("Patient not found.");
            return;
        }

        printPatientHeader();
        patient->printRow();
        patient->showDetails();
    }

    void searchPatientByName() const {
        Console::header("Search Patient By Name");
        const string name = Input::readRequiredLine("Enter full or partial patient name: ");
        vector<const Patient *> matches;

        for (const Patient &patient : patients) {
            if (patient.matchesName(name)) {
                matches.push_back(&patient);
            }
        }

        if (matches.empty()) {
            Console::error("No matching patients found.");
            return;
        }

        printPatientHeader();
        for (const Patient *patient : matches) {
            patient->printRow();
        }
    }

    void updatePatient() {
        Console::header("Update Patient Details");
        const string id = readPatientId("Enter patient ID to update: ");
        Patient *patient = findPatient(id);

        if (!patient) {
            Console::error("Patient not found.");
            return;
        }

        patient->showDetails();
        Console::info("\nPress Enter to keep an existing value.");

        patient->setName(Input::readOptionalLine("Name", patient->getName()));
        patient->setAge(Input::readOptionalInt("Age", patient->getAge(), 0, 130));
        patient->setGender(Input::readOptionalLine("Gender", patient->getGender()));
        patient->setPhone(Input::readOptionalLine("Phone", patient->getPhone()));
        patient->setBloodGroup(Input::readOptionalLine("Blood Group", patient->getBloodGroup()));
        patient->setDisease(Input::readOptionalLine("Disease / Diagnosis", patient->getDisease()));
        patient->setAddress(Input::readOptionalLine("Address", patient->getAddress()));

        if (savePatients()) {
            Console::success("Patient record updated.");
        }
    }

    void deletePatient() {
        Console::header("Delete Patient Record");
        const string id = readPatientId("Enter patient ID to delete: ");
        Patient *patient = findPatient(id);

        if (!patient) {
            Console::error("Patient not found.");
            return;
        }

        patient->showDetails();
        if (!Input::readYesNo("Delete this patient and linked appointments/bills?")) {
            Console::warning("Delete cancelled.");
            return;
        }

        patients.remove_if([id](const Patient &record) { return record.getPatientId() == id; });
        appointments.erase(remove_if(appointments.begin(), appointments.end(),
                                     [id](const Appointment &record) {
                                         return record.getPatientId() == id;
                                     }),
                           appointments.end());
        bills.erase(remove_if(bills.begin(), bills.end(),
                              [id](const Bill &record) {
                                  return record.getPatientId() == id;
                              }),
                    bills.end());
        rebuildAppointmentQueue();

        savePatients();
        saveAppointments();
        saveBills();
        Console::success("Patient record deleted.");
    }

    void doctorMenu() {
        while (true) {
            Console::clearScreen();
            Console::header("Doctor Management");
            cout << "1. Add Doctor\n";
            cout << "2. View All Doctors\n";
            cout << "3. Search Doctor\n";
            cout << "4. Update Doctor\n";
            cout << "5. Delete Doctor\n";
            cout << "0. Back\n";
            Console::line(92, '-');

            const int choice = Input::readInt("Choose an option: ", 0, 5);
            if (choice == 0) {
                return;
            }

            Console::clearScreen();
            switch (choice) {
            case 1:
                addDoctor();
                break;
            case 2:
                listDoctors();
                break;
            case 3:
                searchDoctor();
                break;
            case 4:
                updateDoctor();
                break;
            case 5:
                deleteDoctor();
                break;
            }
            Console::pause();
        }
    }

    void printDoctorHeader() const {
        Console::line(106, '-');
        cout << left << setw(12) << "ID"
             << setw(24) << "Name"
             << setw(24) << "Specialization"
             << setw(12) << "Room"
             << setw(18) << "Availability"
             << setw(16) << "Phone" << '\n';
        Console::line(106, '-');
    }

    void addDoctor() {
        Console::header("Add Doctor");
        string id = nextDoctorId();
        while (doctorIdExists(id)) {
            id = Storage::formatDoctorId(Storage::doctorIdNumber(id) + 1);
        }

        Doctor doctor(id,
                      Input::readRequiredLine("Name: "),
                      Input::readInt("Age: ", 20, 100),
                      Input::readRequiredLine("Gender: "),
                      Input::readRequiredLine("Phone: "),
                      Input::readRequiredLine("Specialization: "),
                      Input::readRequiredLine("Room Number: "),
                      Input::readRequiredLine("Availability: "));

        doctors.push_back(doctor);
        if (saveDoctors()) {
            Console::success("Doctor added with ID " + doctor.getDoctorId() + ".");
        }
    }

    void listDoctors() const {
        Console::header("Doctor List");
        if (doctors.empty()) {
            Console::warning("No doctor records found.");
            return;
        }

        printDoctorHeader();
        for (const Doctor &doctor : doctors) {
            doctor.printRow();
        }
    }

    void searchDoctor() const {
        Console::header("Search Doctor");
        const string id = readDoctorId("Enter doctor ID: ");
        const Doctor *doctor = findDoctor(id);

        if (!doctor) {
            Console::error("Doctor not found.");
            return;
        }

        printDoctorHeader();
        doctor->printRow();
        doctor->showDetails();
    }

    void updateDoctor() {
        Console::header("Update Doctor");
        const string id = readDoctorId("Enter doctor ID to update: ");
        Doctor *doctor = findDoctor(id);

        if (!doctor) {
            Console::error("Doctor not found.");
            return;
        }

        doctor->showDetails();
        Console::info("\nPress Enter to keep an existing value.");

        doctor->setName(Input::readOptionalLine("Name", doctor->getName()));
        doctor->setAge(Input::readOptionalInt("Age", doctor->getAge(), 20, 100));
        doctor->setGender(Input::readOptionalLine("Gender", doctor->getGender()));
        doctor->setPhone(Input::readOptionalLine("Phone", doctor->getPhone()));
        doctor->setSpecialization(Input::readOptionalLine("Specialization", doctor->getSpecialization()));
        doctor->setRoomNumber(Input::readOptionalLine("Room Number", doctor->getRoomNumber()));
        doctor->setAvailability(Input::readOptionalLine("Availability", doctor->getAvailability()));

        if (saveDoctors()) {
            Console::success("Doctor record updated.");
        }
    }

    void deleteDoctor() {
        Console::header("Delete Doctor");
        const string id = readDoctorId("Enter doctor ID to delete: ");
        Doctor *doctor = findDoctor(id);

        if (!doctor) {
            Console::error("Doctor not found.");
            return;
        }

        doctor->showDetails();
        if (!Input::readYesNo("Delete this doctor and linked appointments?")) {
            Console::warning("Delete cancelled.");
            return;
        }

        doctors.erase(remove_if(doctors.begin(), doctors.end(),
                                [id](const Doctor &record) {
                                    return record.getDoctorId() == id;
                                }),
                      doctors.end());
        appointments.erase(remove_if(appointments.begin(), appointments.end(),
                                     [id](const Appointment &record) {
                                         return record.getDoctorId() == id;
                                     }),
                           appointments.end());
        rebuildAppointmentQueue();

        saveDoctors();
        saveAppointments();
        Console::success("Doctor record deleted.");
    }

    void appointmentMenu() {
        while (true) {
            Console::clearScreen();
            Console::header("Appointment Management");
            cout << "1. Book Appointment\n";
            cout << "2. View All Appointments\n";
            cout << "3. View Appointments By Patient\n";
            cout << "4. Cancel Appointment\n";
            cout << "5. View Appointment Queue\n";
            cout << "0. Back\n";
            Console::line(92, '-');

            const int choice = Input::readInt("Choose an option: ", 0, 5);
            if (choice == 0) {
                return;
            }

            Console::clearScreen();
            switch (choice) {
            case 1:
                bookAppointment();
                break;
            case 2:
                listAppointments();
                break;
            case 3:
                listAppointmentsByPatient();
                break;
            case 4:
                cancelAppointment();
                break;
            case 5:
                viewAppointmentQueue();
                break;
            }
            Console::pause();
        }
    }

    void printAppointmentHeader() const {
        Console::line(124, '-');
        cout << left << setw(12) << "ID"
             << setw(24) << "Patient"
             << setw(24) << "Doctor"
             << setw(14) << "Date"
             << setw(10) << "Time"
             << setw(14) << "Status"
             << setw(24) << "Reason" << '\n';
        Console::line(124, '-');
    }

    void printAppointmentRow(const Appointment &appointment) const {
        cout << left << setw(12) << appointment.getAppointmentId()
             << setw(24) << clip(patientName(appointment.getPatientId()), 23)
             << setw(24) << clip(doctorName(appointment.getDoctorId()), 23)
             << setw(14) << clip(appointment.getDate(), 13)
             << setw(10) << clip(appointment.getTime(), 9)
             << setw(14) << clip(appointment.getStatus(), 13)
             << setw(24) << clip(appointment.getReason(), 23) << '\n';
    }

    void bookAppointment() {
        Console::header("Book Appointment");
        if (patients.empty() || doctors.empty()) {
            Console::error("Add at least one patient and one doctor before booking an appointment.");
            return;
        }

        const string patientId = readPatientId("Patient ID: ");
        if (!findPatient(patientId)) {
            Console::error("Patient not found.");
            return;
        }

        const string doctorId = readDoctorId("Doctor ID: ");
        if (!findDoctor(doctorId)) {
            Console::error("Doctor not found.");
            return;
        }

        string id = nextAppointmentId();
        while (appointmentIdExists(id)) {
            id = Storage::formatAppointmentId(Storage::appointmentIdNumber(id) + 1);
        }

        const string appointmentDate = Storage::currentDate();
        const string appointmentTime = Storage::currentTime();
        Appointment appointment(id,
                                patientId,
                                doctorId,
                                appointmentDate,
                                appointmentTime,
                                Input::readRequiredLine("Reason: "),
                                "Scheduled");

        appointments.push_back(appointment);
        appointmentQueue.push(appointment.getAppointmentId());

        if (saveAppointments()) {
            Console::success("Appointment booked with ID " + appointment.getAppointmentId() + ".");
            Console::info("Date/time generated automatically: " + appointmentDate + " " + appointmentTime);
        }
    }

    void listAppointments() const {
        Console::header("Appointment List");
        if (appointments.empty()) {
            Console::warning("No appointment records found.");
            return;
        }

        printAppointmentHeader();
        for (const Appointment &appointment : appointments) {
            printAppointmentRow(appointment);
        }
    }

    void listAppointmentsByPatient() const {
        Console::header("Appointments By Patient");
        const string patientId = readPatientId("Patient ID: ");
        if (!findPatient(patientId)) {
            Console::error("Patient not found.");
            return;
        }

        bool found = false;
        printAppointmentHeader();
        for (const Appointment &appointment : appointments) {
            if (appointment.getPatientId() == patientId) {
                printAppointmentRow(appointment);
                found = true;
            }
        }

        if (!found) {
            Console::warning("No appointments found for this patient.");
        }
    }

    void cancelAppointment() {
        Console::header("Cancel Appointment");
        const string id = readAppointmentId("Appointment ID: ");
        Appointment *appointment = findAppointment(id);

        if (!appointment) {
            Console::error("Appointment not found.");
            return;
        }

        printAppointmentHeader();
        printAppointmentRow(*appointment);

        if (!appointment->isScheduled()) {
            Console::warning("Only scheduled appointments can be cancelled.");
            return;
        }

        if (!Input::readYesNo("Cancel this appointment?")) {
            Console::warning("Cancellation skipped.");
            return;
        }

        appointment->cancel();
        rebuildAppointmentQueue();
        if (saveAppointments()) {
            Console::success("Appointment cancelled.");
        }
    }

    void viewAppointmentQueue() const {
        Console::header("Appointment Queue");
        queue<string> queueCopy = appointmentQueue;

        if (queueCopy.empty()) {
            Console::warning("No scheduled appointments in the queue.");
            return;
        }

        printAppointmentHeader();
        while (!queueCopy.empty()) {
            const Appointment *appointment = findAppointment(queueCopy.front());
            queueCopy.pop();
            if (appointment) {
                printAppointmentRow(*appointment);
            }
        }
    }

    void billingMenu() {
        while (true) {
            Console::clearScreen();
            Console::header("Billing Management");
            cout << "1. Generate Bill\n";
            cout << "2. View All Bills\n";
            cout << "3. View Bills By Patient\n";
            cout << "4. Print Final Bill\n";
            cout << "5. Mark Bill As Paid\n";
            cout << "6. Export Bill To Text File\n";
            cout << "0. Back\n";
            Console::line(92, '-');

            const int choice = Input::readInt("Choose an option: ", 0, 6);
            if (choice == 0) {
                return;
            }

            Console::clearScreen();
            switch (choice) {
            case 1:
                generateBill();
                break;
            case 2:
                listBills();
                break;
            case 3:
                listBillsByPatient();
                break;
            case 4:
                printFinalBill();
                break;
            case 5:
                markBillPaid();
                break;
            case 6:
                exportBill();
                break;
            }
            Console::pause();
        }
    }

    void printBillHeader() const {
        Console::line(114, '-');
        cout << left << setw(12) << "ID"
             << setw(24) << "Patient"
             << setw(19) << "Date/Time"
             << setw(28) << "Description"
             << right << setw(12) << "Total"
             << "   " << left << setw(10) << "Status" << '\n';
        Console::line(114, '-');
    }

    void printBillRow(const Bill &bill) const {
        cout << left << setw(12) << bill.getBillId()
             << setw(24) << clip(patientName(bill.getPatientId()), 23)
             << setw(19) << clip(bill.getDate(), 18)
             << setw(28) << clip(bill.getDescription(), 27)
             << right << setw(12) << fixed << setprecision(2) << bill.getTotalAmount()
             << "   " << left << setw(10) << (bill.isPaid() ? "Paid" : "Unpaid") << '\n';
    }

    void generateBill() {
        Console::header("Generate Bill");
        if (patients.empty()) {
            Console::error("Add a patient before creating a bill.");
            return;
        }

        const string patientId = readPatientId("Patient ID: ");
        if (!findPatient(patientId)) {
            Console::error("Patient not found.");
            return;
        }

        string id = nextBillId();
        while (billIdExists(id)) {
            id = Storage::formatBillId(Storage::billIdNumber(id) + 1);
        }

        Bill bill(id,
                  patientId,
                  Storage::currentDateTime(),
                  Input::readRequiredLine("Description: "),
                  Input::readDouble("Medicine charges: ", 0.0),
                  Input::readDouble("Room charges: ", 0.0),
                  Input::readDouble("Doctor fees: ", 0.0),
                  Input::readDouble("Other charges: ", 0.0),
                  Input::readYesNo("Is the bill already paid?"));

        bills.push_back(bill);
        if (saveBills()) {
            Console::success("Bill generated with ID " + bill.getBillId() + ".");
            bill.printReceipt(patientName(patientId));
            if (bill.exportReceipt(patientName(patientId), "bill_exports")) {
                Console::success("Invoice exported to bill_exports/" + bill.getBillId() + ".txt.");
            } else {
                Console::warning("Bill saved, but invoice export failed.");
            }
        }
    }

    void listBills() const {
        Console::header("Bill List");
        if (bills.empty()) {
            Console::warning("No bill records found.");
            return;
        }

        printBillHeader();
        for (const Bill &bill : bills) {
            printBillRow(bill);
        }
    }

    void listBillsByPatient() const {
        Console::header("Bills By Patient");
        const string patientId = readPatientId("Patient ID: ");
        if (!findPatient(patientId)) {
            Console::error("Patient not found.");
            return;
        }

        bool found = false;
        printBillHeader();
        for (const Bill &bill : bills) {
            if (bill.getPatientId() == patientId) {
                printBillRow(bill);
                found = true;
            }
        }

        if (!found) {
            Console::warning("No bills found for this patient.");
        }
    }

    void printFinalBill() const {
        Console::header("Print Final Bill");
        const string billId = readBillId("Bill ID: ");
        const Bill *bill = findBill(billId);

        if (!bill) {
            Console::error("Bill not found.");
            return;
        }

        bill->printReceipt(patientName(bill->getPatientId()));
    }

    void markBillPaid() {
        Console::header("Mark Bill As Paid");
        const string billId = readBillId("Bill ID: ");
        Bill *bill = findBill(billId);

        if (!bill) {
            Console::error("Bill not found.");
            return;
        }

        if (bill->isPaid()) {
            Console::warning("This bill is already marked as paid.");
            return;
        }

        bill->markPaid();
        if (saveBills()) {
            Console::success("Bill marked as paid.");
            if (bill->exportReceipt(patientName(bill->getPatientId()), "bill_exports")) {
                Console::success("Updated invoice exported to bill_exports/" + bill->getBillId() + ".txt.");
            }
        }
    }

    void exportBill() const {
        Console::header("Export Bill");
        const string billId = readBillId("Bill ID: ");
        const Bill *bill = findBill(billId);

        if (!bill) {
            Console::error("Bill not found.");
            return;
        }

        if (bill->exportReceipt(patientName(bill->getPatientId()), "bill_exports")) {
            Console::success("Invoice exported to bill_exports/" + bill->getBillId() + ".txt.");
        } else {
            Console::error("Unable to export this bill.");
        }
    }
};

int main() {
    AdminAuth adminAuth;
    if (!adminAuth.authenticate()) {
        return 0;
    }

    HospitalManagementSystem hospital;
    hospital.run();
    return 0;
}
