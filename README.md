# Hospital Management System in C++

A modular, console-based Hospital Management System built with C++ object-oriented programming and text-file persistence. The project manages patients, doctors, appointments, billing, and admin-only access through a simple terminal interface.

## Project Overview

This system is designed as a beginner-to-intermediate C++ project that demonstrates:

- Object-oriented programming with classes, inheritance, and encapsulation
- File handling using text files
- Modular source organization with header and implementation files
- Input validation and friendly console error messages
- Basic admin authentication with hashed credentials
- Practical C++ data structures
- Professional auto-generated IDs such as `HSP2026001`, `DOC2026001`, `APT2026001`, and `BIL2026001`

## Features

- Admin login before accessing the system
- Masked password input on Windows
- Patient management
  - Add patient with auto-generated ID
  - View all patients
  - Search patient by ID
  - Search patient by name
  - Update patient details
  - Delete patient records
- Doctor management
  - Add, view, search, update, and delete doctors with professional IDs
- Appointment management
  - Book appointment
  - Generate appointment ID, date, and time automatically
  - Cancel appointment
  - View all appointments
  - View appointments by patient
  - View scheduled appointment queue
- Billing management
  - Generate bill
  - Generate bill ID, date, and time automatically
  - Add medicine charges
  - Add room charges
  - Add doctor fees
  - Add other charges
  - Print final bill
  - Export bill invoice to a text file
  - Mark bill as paid
- Improved console UI
  - ASCII hospital header
  - Professional dashboard summary
  - Borders and separators
  - Clear screen support
  - Windows console colors
  - Better table formatting
- Safer file handling
  - Temporary file writes
  - Backup replacement during saves
  - Empty-file handling
  - Duplicate ID checks while loading records

## Patient ID Format

Professional IDs are generated automatically and saved through the file handling system.

```text
HSP2026001
HSP2026002
HSP2026003
DOC2026001
APT2026001
BIL2026001
```

Appointments and bills store linked patient and doctor IDs so records stay consistent.

## Technologies Used

- C++17
- Object-oriented programming
- Standard Template Library
  - `std::vector`
  - `std::list`
  - `std::queue`
- File handling with `ifstream` and `ofstream`
- Windows console APIs for color and masked password input

## Data Structures Used

- `std::list<Patient>` stores patient records because patients are frequently inserted and deleted.
- `std::vector<Doctor>`, `std::vector<Appointment>`, and `std::vector<Bill>` store records that are commonly displayed, searched, and rewritten to files.
- `std::queue<std::string>` stores scheduled appointment IDs so appointments can be viewed in booking order.

## Project Structure

```text
.
|-- main.cpp
|-- person.h
|-- patient.h
|-- patient.cpp
|-- doctor.h
|-- doctor.cpp
|-- appointment.h
|-- appointment.cpp
|-- billing.h
|-- billing.cpp
|-- storage_utils.h
`-- README.md
```

## Files Created at Runtime

```text
admin.dat
patients.txt
doctors.txt
appointments.txt
bills.txt
bill_exports/
```

The first run asks you to create an admin username and password. Passwords are saved as salted digests instead of plain text.

## Compile Instructions

### Using g++

```powershell
g++ -std=c++17 -Wall -Wextra main.cpp patient.cpp doctor.cpp appointment.cpp billing.cpp -o hospital
```

### Run

```powershell
.\hospital.exe
```

## Screenshots

Add screenshots of your console output here:

```text
screenshots/
|-- admin-login.png
|-- main-menu.png
|-- patient-search.png
`-- final-bill.png
```

## Future Improvements

- Add role-based access for receptionist, doctor, and admin users
- Export bills as PDF files
- Add date/time validation for appointments
- Add password reset support
- Store records in SQLite or MySQL
- Add unit tests for file parsing and billing calculations

## Notes

This project is intended for learning and academic submission. For real hospital software, use a secure database, audited authentication, encryption, access control, and proper healthcare compliance standards.
